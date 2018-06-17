// © Creative Services and Development
// Site Oficial: www.creativesd.com.br
// Termos de Contrato e Autoria em: http://creativesd.com.br/?p=termos

#ifndef _QUEUE_HPP_
#define _QUEUE_HPP_

#include "map.hpp" // EVENT_NAME_LENGTH

#ifdef QUEUE_SYSTEM_ENABLE

#ifndef QUEUE_COLOR
	#define	QUEUE_COLOR	0x89cff0
#endif


struct queue_players {
	int position;
	struct map_session_data *sd;
	struct queue_players *next;
};

struct queue_data {
	unsigned int queue_id;
	unsigned char count;
	int min_level, max_level;
	struct queue_players *first, *last;
	char name[QUEUE_NAME_LENGTH];
	char join_event[EVENT_NAME_LENGTH];
};

/* [CreativeSD]: Queue System */
struct queue_data *queue_search(int queue_id);
int queue_create(int queue_id, const char *name, int min_level, int max_level, const char *event);
int queue_delete(int queue_id);
int queue_join(struct map_session_data *sd, int queue_id, int flag);
int queue_leave(struct map_session_data *sd, int flag);
int queue_clean(struct queue_data *queue, int delay, int flag);
int queue_atcommand_list(struct map_session_data *sd);
void queue_join_notify(int queue_id, struct map_session_data *sd);
void queue_leave_notify(int queue_id, struct map_session_data *sd, int flag);
const char* queue_delay(int delay);

#define queue_check_town(m) (map[m].flag.town)

void do_init_queue(void);
void do_final_queue(void);
#endif

#endif /* _QUEUE_HPP_ */
