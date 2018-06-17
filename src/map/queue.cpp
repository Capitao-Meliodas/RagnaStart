// © Creative Services and Development
// Site Oficial: www.creativesd.com.br
// Termos de Contrato e Autoria em: http://creativesd.com.br/?p=termos
#include "queue.hpp"
#include "map.hpp"

#ifdef QUEUE_SYSTEM_ENABLE
#include "../common/cbasetypes.h"
#include "../common/timer.h"
#include "../common/malloc.h"
#include "../common/nullpo.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"

#include "battle.hpp"
#include "clif.hpp"
#include "npc.hpp"
#include "pc.hpp"

#include <string.h>
#include <stdio.h>

static DBMap* queue_db;
static unsigned int queue_counter = 0;

struct queue_data* queue_search(int queue_id)
{
	if( !queue_id ) return NULL;
	return (struct queue_data *)idb_get(queue_db, queue_id);
}

int queue_create(int queue_id, const char *name, int min_level, int max_level, const char *event)
{
	struct queue_data *queue;
	queue_counter++;
	
	CREATE(queue, struct queue_data, 1);
	queue->queue_id = queue_id;
	queue->count = 0;
	queue->min_level = min_level;
	queue->max_level = max_level;
	safestrncpy(queue->name, name, sizeof(queue->name));
	safestrncpy(queue->join_event, event, sizeof(queue->join_event));
	queue->first = queue->last = NULL;
	idb_put(queue_db, queue_id, queue);
	ShowStatus("queue_create: The queue (%d: %s) was successfully created.\n", queue->queue_id, queue->name);
	return true;
}

int queue_delete(int queue_id)
{
	struct queue_data *queue = queue_search(queue_id);
	char name[QUEUE_NAME_LENGTH];

	if( queue == NULL ) return 0;
	queue_counter--;
	queue_clean(queue,0,3);
	safestrncpy(name, queue->name, sizeof(name));
	idb_remove(queue_db, queue_id);
	ShowStatus("queue_delete: The row (%d: %s) was successfully removed.\n", queue->queue_id, name);
	return true;
}

int queue_join(struct map_session_data *sd, int queue_id, int flag) {
	struct queue_data *queue = queue_search(queue_id);
	struct queue_players *members = NULL;
	char output[200];

	if (queue == NULL || sd == NULL)
		return false;

	// Remove from the last queue and insert the new.
	if( flag < 2 && sd->queue_id )
		queue_leave(sd,0);

	if( flag < 2 && queue->min_level && sd->status.base_level < queue->min_level )
	{
		sprintf(output, msg_txt(sd,QUEUE_MSG_TXT), queue->name);
		clif_displaymessage(sd->fd, output);
		sprintf(output, msg_txt(sd,(QUEUE_MSG_TXT)+1), queue->min_level);
		clif_displaymessage(sd->fd, output);
		return 0;
	}
	
	if( flag < 2 && queue->max_level && sd->status.base_level > queue->max_level )
	{
		sprintf(output, msg_txt(sd, QUEUE_MSG_TXT), queue->name);
		clif_displaymessage(sd->fd, output);
		sprintf(output, msg_txt(sd, (QUEUE_MSG_TXT)+2), queue->max_level);
		clif_displaymessage(sd->fd, output);
		return 0;
	}

	CREATE(members, struct queue_players, 1);
	queue->count++;
	members->sd = sd;
	members->position = queue->count;
	members->next = NULL;
	sd->queue_id = queue_id;

	if( queue->last == NULL )
		queue->first = queue->last = members;
	else {
		queue->last->next = members;
		queue->last = members;
	}
	sd->status.queue_delay = (int)time(NULL)+battle_config.queue_join_delay;
	ShowInfo("Player '%s' joined in queue (%d: %s).\n", sd->status.name, queue_id, queue->name);

	if( battle_config.queue_notify == 1 || battle_config.queue_notify == 3 )
		queue_join_notify(queue_id, sd);

	if( queue->join_event[0] && flag )
	{
		if( !npc_event_do(queue->join_event) )
			ShowError("queue_join: '%s' not found in (queue_id: %d)!\n", queue->join_event, queue_id);
	}
	return true;
}

int queue_leave(struct map_session_data *sd, int flag)
{
	struct queue_data *queue = NULL;
	struct queue_players *head, *previous;
	int queue_id;
	
	if( sd == NULL)
		return false;

	queue_id = sd->queue_id;
	queue = queue_search(queue_id);

	if( queue == NULL )
		return false;

	head = queue->first;
	previous = NULL;

	while( head != NULL )
	{
		if( head->sd && head->sd == sd )
		{
			struct queue_players *next = head->next;
		
			sd->queue_id = 0;
			queue->count--;

			if( previous )
				previous->next = head->next;
			else
				queue->first = head->next;

			if( head->next == NULL )
				queue->last = previous;

			while( next != NULL )
			{
				next->position--;
				next = next->next;
			}

			ShowInfo("Player '%s' removed from queue (%d: %s).\n", sd->status.name, queue_id, queue->name);
			if( flag && battle_config.queue_notify >= 2 )
				queue_leave_notify(queue_id, sd, flag);

			aFree(head);
			return true;
		}

		previous = head;
		head = head->next;
	}
	return false;
}

int queue_clean(struct queue_data *queue, int delay, int flag)
{
	struct queue_players *head, *next;
	struct map_session_data *sd;

	nullpo_ret(queue);
	
	if( queue == NULL )
		return false;

	head = queue->first;
	while( head != NULL )
	{
		if( (sd = head->sd) != NULL )
		{
			if( flag && battle_config.queue_notify >= 2 )
				queue_leave_notify(queue->queue_id, sd, flag);

			if( delay >= 0 )
				sd->status.queue_delay = (int)time(NULL)+delay;

			sd->queue_id = 0;
			ShowInfo("Player '%s' removed from queue (%d: %s).\n", sd->status.name, queue->queue_id, queue->name);
		}
		next = head->next;
		aFree(head);
		head = next;
	}

	queue->first = queue->last = NULL;
	queue->count = 0;
	return true;
}

int queue_atcommand_list(struct map_session_data *sd)
{
	DBIterator* iter = db_iterator(queue_db);
	struct queue_data* queue = NULL;
	char output[128];
	int count = 0;

	for( queue = (struct queue_data*)dbi_first(iter); dbi_exists(iter); queue = (struct queue_data*)dbi_next(iter) )
	{
		sprintf(output, "    %d - %s", queue->queue_id, queue->name);
		clif_displaymessage(sd->fd, output);
		count++;
	}
	dbi_destroy(iter);
	return count;
}

void queue_join_notify(int queue_id, struct map_session_data *sd)
{
	struct queue_data *queue = NULL;
	struct queue_players *next;
	char output[256];

	queue = queue_search(queue_id);
	if( queue == NULL )
		return;

	next = queue->first;
	while( next != NULL )
	{
		if( next->sd && next->sd != sd )
		{
			sprintf(output, msg_txt(next->sd,(QUEUE_MSG_TXT+3)), sd->status.name, queue->name);
			clif_messagecolor(&next->sd->bl, QUEUE_COLOR, output, true, SELF);
		}
		next = next->next;
	}

	sprintf(output, msg_txt(sd,(QUEUE_MSG_TXT+4)), queue->name);
	clif_messagecolor(&sd->bl, QUEUE_COLOR, output, true, SELF);
}

void queue_leave_notify(int queue_id, struct map_session_data *sd, int flag)
{
	struct queue_data *queue = NULL;
	struct queue_players *next;
	char output[256];

	if( flag <= 0 || flag >= 9 )
		return;

	queue = queue_search(queue_id);
	if( queue == NULL )
		return;

	next = queue->first;
	while( next != NULL )
	{
		if( next->sd && (sd == NULL || next->sd != sd) )
		{
			switch(flag)
			{
				case 1:
					sprintf(output, msg_txt(next->sd,(QUEUE_MSG_TXT+5)), sd->status.name, queue->name);
					clif_messagecolor(&next->sd->bl, QUEUE_COLOR, output, true, SELF);
					break;
				case 2:
					sprintf(output, msg_txt(next->sd,(QUEUE_MSG_TXT+6)), sd->status.name, queue->name);
					clif_messagecolor(&next->sd->bl, QUEUE_COLOR, output, true, SELF);
					break;
				case 3:
					sprintf(output, msg_txt(next->sd,(QUEUE_MSG_TXT+7)), sd->status.name, queue->name);
					clif_messagecolor(&next->sd->bl, QUEUE_COLOR, output, true, SELF);
					break;
				case 4:
					sprintf(output, msg_txt(next->sd,(QUEUE_MSG_TXT+8)), sd->status.name, queue->name);
					clif_messagecolor(&next->sd->bl, QUEUE_COLOR, output, true, SELF);
					break;
				default:
					break;
			}
		}
		next = next->next;
	}

	if( sd != NULL )
	{
		switch(flag)
		{
			case 1:
				sprintf(output, msg_txt(sd,(QUEUE_MSG_TXT+9)), queue->name);
				clif_messagecolor(&sd->bl, QUEUE_COLOR, output, true, SELF);
				break;
			case 2:
				sprintf(output, msg_txt(sd,(QUEUE_MSG_TXT+10)), queue->name);
				clif_messagecolor(&sd->bl, QUEUE_COLOR, output, true, SELF);
				break;
			case 3:
				sprintf(output, msg_txt(sd,(QUEUE_MSG_TXT+11)), queue->name);
				clif_messagecolor(&sd->bl, QUEUE_COLOR, output, true, SELF);
				break;
			case 5:
				sprintf(output, msg_txt(sd,(QUEUE_MSG_TXT+12)), queue->name);
				clif_messagecolor(&sd->bl, QUEUE_COLOR, output, true, SELF);
				break;
			case 6:
				sprintf(output, msg_txt(sd,(QUEUE_MSG_TXT+13)), queue->name, queue_delay(sd->status.queue_delay));
				clif_messagecolor(&sd->bl, QUEUE_COLOR, output, true, SELF);
				break;
			case 7:
				sprintf(output, msg_txt(sd,(QUEUE_MSG_TXT+14)), queue->name);
				clif_messagecolor(&sd->bl, QUEUE_COLOR, output, true, SELF);
				break;
			case 8:
				sprintf(output, msg_txt(sd,(QUEUE_MSG_TXT+15)), queue->name);
				clif_messagecolor(&sd->bl, QUEUE_COLOR, output, true, SELF);
				break;
			default:
				return; // Nenhuma mensagem para o jogador.
		}
	}
}

const char* queue_delay(int delay) {
	int timer = delay-(int)time(NULL);
	int sec = 0, min = 0, hour = 0, days = 0;
	static char output[128];

	if( timer < 60 ) {
		sprintf(output, msg_txt(NULL,(QUEUE_MSG_TXT+16)), timer);
		return ((const char*)output);
	}
	
	min = timer/60;
	sec = timer - (min*60);

	if( min < 60 ) {
		sprintf(output, msg_txt(NULL,(QUEUE_MSG_TXT+17)), min, sec);
		return ((const char*)output);
	}

	hour = min/60;
	min = min - (hour*60);

	if( hour < 24 ) {
		sprintf(output, msg_txt(NULL,(QUEUE_MSG_TXT+18)), hour, min, sec);
		return ((const char*)output);
	}

	days = hour/24;
	hour = hour - (days*24);

	sprintf(output, msg_txt(NULL,(QUEUE_MSG_TXT+19)), days, hour, min, sec);
	return ((const char*)output);
}

void do_init_queue(void)
{
	queue_db = idb_alloc(DB_OPT_RELEASE_DATA);
	ShowMessage(CL_WHITE"[Queue System]:" CL_RESET " System Queue successfully initialized.\n");
	ShowMessage(CL_WHITE"[Queue System]:" CL_RESET " by (c) CreativeSD, suport in " CL_GREEN "www.creativesd.com.br" CL_RESET "\n");
}

static int queue_db_final(DBKey key, DBData *data, va_list ap)
{
	struct queue_data *queue = (struct queue_data *)data;
	if( queue )
		queue_clean(queue,0,7);
	return 0;
}

void do_final_queue(void)
{
	queue_db->destroy(queue_db, queue_db_final);
}
#endif