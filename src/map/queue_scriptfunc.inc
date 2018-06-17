// © Creative Services and Development
// Site Oficial: www.creativesd.com.br
// Termos de Contrato e Autoria em: http://creativesd.com.br/?p=termos

#ifndef SCRIPT_CMD_SUCCESS
	#define	SCRIPT_CMD_SUCCESS	0
#endif

#ifndef SCRIPT_CMD_FAILURE
	#define	SCRIPT_CMD_FAILURE	1
#endif

/*==========================================
 * CreativeSD: Queue System
 *------------------------------------------*/
BUILDIN_FUNC(queue_create)
{
	struct queue_data *queue = NULL;
	const char *event = "", *name = "";
	int queue_id = script_getnum(st,2), min_level = 0, max_level = 0;

	if( queue_id <= 0 )
	{
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	name = script_getstr(st,3);
	if( script_hasdata(st,4) ) min_level = script_getnum(st,4);
	if( script_hasdata(st,5) ) max_level = script_getnum(st,5);
	if( script_hasdata(st,6) )
	{
		event = script_getstr(st,6);
		check_event(st, event);
	}

	if( (queue = queue_search(queue_id)) != NULL )
	{
		safestrncpy(queue->name, name, sizeof(queue->name));
		safestrncpy(queue->join_event, event, sizeof(queue->join_event));
		queue->min_level = min_level;
		queue->max_level = max_level;
		script_pushint(st,1);
		return SCRIPT_CMD_SUCCESS;
	}

	script_pushint(st,queue_create(queue_id, name, min_level, max_level, event));
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(queue_delete)
{
	struct queue_data *queue = NULL;
	int queue_id = script_getnum(st,2);
	
	if( (queue = queue_search(queue_id)) == NULL )
	{
		script_pushint(st,0);
		ShowError("script:queue_delete: queue (%d) does not exist\n", queue_id);
		return SCRIPT_CMD_SUCCESS;
	}

	script_pushint(st,queue_delete(queue_id));
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(queue_check)
{
	struct queue_data *queue = queue_search(script_getnum(st,2));
	script_pushint(st,queue==NULL?0:1);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(queue_size)
{
	struct queue_data *queue = queue_search(script_getnum(st,2));
	script_pushint(st,(queue==NULL?0:queue->count));
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(queue_join)
{
	TBL_PC* sd;
	struct queue_data *queue = NULL;
	int queue_id = script_getnum(st,2);
	int flag = 1, check_delay = 1, delay = 0;

	if( (queue = queue_search(queue_id)) == NULL )
	{
		script_pushint(st,0);
		ShowError("script:queue_join: queue (%d) does not exist\n", queue_id);
		return SCRIPT_CMD_SUCCESS;
	}

	if( script_hasdata(st,3) )
		flag = script_getnum(st,3);

	// Delay
	if( script_hasdata(st,4) )
		check_delay = script_getnum(st,4);

	if (script_hasdata(st,5))
		sd = map_id2sd(script_getnum(st,5));
	else
		script_rid2sd(sd);

	if (!sd)
	{
		script_pushint(st,1);
		return SCRIPT_CMD_SUCCESS;
	}
	
	if (sd->queue_id && !queue_leave(sd,2) ) {
		script_pushint(st,-1);
		return SCRIPT_CMD_SUCCESS;
	}
	
	if( battle_config.queue_only_towns && !queue_check_town(sd->bl.m) ) {
		script_pushint(st,-2);
		return SCRIPT_CMD_SUCCESS;
	}

	if( check_delay )
	{
		delay = sd->status.queue_delay-(int)time(NULL);
		if( delay > battle_config.queue_join_delay ) {
			sd->status.queue_delay = (int)time(NULL)+battle_config.queue_join_delay;
			delay = sd->status.queue_delay-(int)time(NULL);
		}

		if( delay > 0 ) {
			script_pushint(st,-3);
			return SCRIPT_CMD_SUCCESS;
		}
	}
	
	script_pushint(st, queue_join(sd,queue_id,flag));
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(queue_leave)
{
	TBL_PC* sd = NULL;
	struct queue_data *queue = NULL;
	int queue_id = 0, flag = 2;

	if( script_hasdata(st,2) )
		flag = script_getnum(st,2);

	if( script_hasdata(st,3) )
		sd = map_id2sd(script_getnum(st,3));
	else
		script_rid2sd(sd);
	
	if( !sd || !sd->queue_id )
	{
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}
	
	queue_id = sd->queue_id;
	
	if( (queue = queue_search(queue_id)) == NULL )
	{
		if( sd->queue_id == queue_id ) sd->queue_id = 0;
		script_pushint(st,1);
		ShowError("script:queue_leave: queue (%d) does not exist\n", queue_id);
		return SCRIPT_CMD_SUCCESS;
	}
	
	script_pushint(st, queue_leave(sd,flag));
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(queue_clean)
{
	struct queue_data *queue = queue_search(script_getnum(st,2));
	int flag = 0, delay = -1;

	if( script_hasdata(st,3) )
		flag = script_getnum(st,3);
	
	if( script_hasdata(st,4) )
		delay = script_getnum(st,4);

	if( queue == NULL ) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	script_pushint(st, queue_clean(queue,delay,flag));
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(queue_leave_notify)
{
	TBL_PC* sd = NULL;
	struct queue_data *queue;
	int queue_id, flag;

	queue_id = script_getnum(st,2);
	if( script_hasdata(st,3) )
		flag = script_getnum(st,3);

	if( script_hasdata(st,4) )
		sd = map_id2sd(script_getnum(st,4));
	
	if( (queue = queue_search(queue_id)) == NULL )
	{
		script_pushint(st,1);
		ShowError("script:queue_leave_notify: queue (%d) does not exist\n", queue_id);
		return SCRIPT_CMD_SUCCESS;
	}
	
	queue_leave_notify(queue_id,sd,flag);
	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(queue_get_players)
{
	struct queue_data *queue = queue_search(script_getnum(st,2));
	struct queue_players *head;
	struct map_session_data *sd;
	int i = 0;

	if ( queue == NULL)
	{
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	head = queue->first;
	while( head )
	{
		if( (sd = head->sd) != NULL )
		{
			set_reg(st, NULL, reference_uid(add_str(".@queue_cid"), i), ".@queue_cid", (void*)__64BPRTSIZE(sd->status.char_id), NULL);
			set_reg(st, NULL, reference_uid(add_str(".@queue_aid"), i), ".@queue_aid", (void*)__64BPRTSIZE(sd->status.account_id), NULL);
			set_reg(st, NULL, reference_uid(add_str(".@queue_player$"), i), ".@queue_player$", (void*)__64BPRTSIZE(sd->status.name), NULL);
			i++;
		}
		head = head->next;
	}

	set_reg(st, NULL, add_str(".@queue_count"), ".@queue_count", (void*)__64BPRTSIZE(i), NULL);
	script_pushint(st, i);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(queue_get_data)
{
	struct queue_data *queue = NULL;
	int queue_id = script_getnum(st,2), type = script_getnum(st,3);

	if( (queue = queue_search(queue_id)) == NULL )
	{
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	switch( type )
	{
		case 0: script_pushstrcopy(st, queue->name); break;
		case 1: script_pushstrcopy(st, queue->join_event); break;
		case 2: script_pushint(st, queue->min_level); break;
		case 3: script_pushint(st, queue->max_level); break;
		case 4: script_pushint(st, queue->count); break;
		default:
			ShowError("script:queue_get_data: invalid parameter (%d).\n", type);
			script_pushint(st,0);
			break;
	}

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(queue_set_data)
{
	struct queue_data *queue = NULL;
	int queue_id = script_getnum(st,2), type = script_getnum(st,3);
	
	if( (queue = queue_search(queue_id)) == NULL )
	{
		script_pushint(st,0);
		ShowError("script:queue_change_event: queue (%d) does not exist\n", queue_id);
		return SCRIPT_CMD_SUCCESS;
	}

	switch(type)
	{
		case 0:
			{
				const char* change = script_getstr(st,4);
				safestrncpy(queue->name, change, sizeof(queue->name));
			}
			break;
		case 1:
			{
				const char* change = script_getstr(st,4);
				check_event(st, change);
				safestrncpy(queue->join_event, change, sizeof(queue->name));
			}
			break;
		case 2:
			{
				int value = script_getnum(st,4);
				queue->min_level = value;
			}
			break;
		case 3:
			{
				int value = script_getnum(st,4);
				queue->max_level = value;
			}
			break;
		default:
			script_pushint(st,0);
			ShowError("script:queue_change: invalid type (%d).\n", type);
			return SCRIPT_CMD_SUCCESS;
	}

	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(queue_char_info)
{
	TBL_PC* sd = NULL;
	struct queue_data *queue = NULL;
	int type = script_getnum(st,2), delay = 0;

	if (script_hasdata(st,3))
		sd = map_id2sd(script_getnum(st,3));
	else
		script_rid2sd(sd);

	if( sd == NULL ) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}
	
	if( sd->queue_id && (queue = queue_search(sd->queue_id)) == NULL )
		sd->queue_id = 0;

	// Update Delay.
	delay = sd->status.queue_delay-(int)time(NULL);
	if( delay > battle_config.queue_join_delay )
		sd->status.queue_delay = (int)time(NULL)+battle_config.queue_join_delay;

	switch( type )
	{
		case 0: script_pushint(st, sd->queue_id); break;
		case 1: script_pushint(st, sd->status.queue_delay); break;
		case 2:
			delay = sd->status.queue_delay-(int)time(NULL);
			script_pushint(st, (delay>0?delay:0));
			break;
		case 3: script_pushstrcopy(st, queue_delay(sd->status.queue_delay)); break;
		case 4: script_pushint(st, sd->status.base_level); break;
		case 5: script_pushint(st, sd->queue_nomove); break;
		default:
			ShowError("script:queue_getcharinfo: invalid parameter (%d).\n", type);
			script_pushint(st,0);
			break;
	}

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(queue_set_delay)
{
	TBL_PC* sd = NULL;
	int delay = script_getnum(st,2);

	if (script_hasdata(st,3))
		sd = map_id2sd(script_getnum(st,3));
	else
		script_rid2sd(sd);

	if( sd == NULL ) {
		script_pushint(st,-1);
		return SCRIPT_CMD_SUCCESS;
	}

	sd->status.queue_delay = (int)time(NULL)+delay;

	// Update Delay.
	delay = sd->status.queue_delay-(int)time(NULL);
	if( delay > battle_config.queue_join_delay )
		sd->status.queue_delay = (int)time(NULL)+battle_config.queue_join_delay;

	script_pushint(st,sd->status.queue_delay);
	return SCRIPT_CMD_SUCCESS;
}
