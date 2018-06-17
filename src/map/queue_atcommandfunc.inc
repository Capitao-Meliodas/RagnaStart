// © Creative Services and Development
// Site Oficial: www.creativesd.com.br
// Termos de Contrato e Autoria em: http://creativesd.com.br/?p=termos

/*==========================================
 * CreativeSD: Queue System
 *------------------------------------------*/
ACMD_FUNC(queue)
{
	int count = 0;
	char output[128];
	nullpo_retr(-1, sd);

	if( sd == NULL )
		return -1;

	clif_displaymessage(fd, msg_txt(sd,(QUEUE_MSG_TXT+20)));
	count = queue_atcommand_list(sd);
	
	if( !count )
		clif_displaymessage(fd, msg_txt(sd,(QUEUE_MSG_TXT+21)));
	else {
		sprintf(output, msg_txt(sd,(QUEUE_MSG_TXT+22)), count);
		clif_displaymessage(fd, output);
	}
	return 0;
}

ACMD_FUNC(queuelist)
{
	struct queue_data *queue = NULL;
	struct queue_players *next;
	int queue_id;
	char output[128];

	nullpo_retr(-1, sd);

	if( sd == NULL )
		return -1;

	if (!message || !*message || sscanf(message, "%d", &queue_id) < 1 || queue_id == 0 || (queue = queue_search(queue_id)) == NULL) {
		clif_displaymessage(fd, msg_txt(sd,(QUEUE_MSG_TXT+23)));
		queue_atcommand_list(sd);
		return -1;
	}

	if( queue->count <= 0 )
	{
		sprintf(output, msg_txt(sd,(QUEUE_MSG_TXT+24)), queue->name);
		clif_displaymessage(fd,output);
		return 0;
	}

	next = queue->first;
	sprintf(output, msg_txt(sd,(QUEUE_MSG_TXT+25)), queue->name);
	clif_displaymessage(fd,output);
	while( next != NULL )
	{
		if( next->sd ) {
			sprintf(output, msg_txt(sd,(QUEUE_MSG_TXT+26)), next->sd->status.char_id, next->sd->status.name, job_name(next->sd->status.class_), next->sd->status.base_level, next->sd->status.job_level);
			clif_displaymessage(fd,output);
		}
		next = next->next;
	}
	return 0;
}

ACMD_FUNC(queuekick)
{
	struct queue_data *queue = NULL;
	struct map_session_data *pl_sd = NULL;
	char output[128];

	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, msg_txt(sd,(QUEUE_MSG_TXT+27)));
		return -1;
	}

	if((pl_sd=map_nick2sd(atcmd_player_name,true)) == NULL && (pl_sd=map_charid2sd(atoi(atcmd_player_name))) == NULL)
	{
		clif_displaymessage(fd, msg_txt(sd,(QUEUE_MSG_TXT+28)));
		return -1;
	}

	if( !pl_sd->queue_id || (queue = queue_search(pl_sd->queue_id)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(sd,(QUEUE_MSG_TXT+29)));
		return -1;
	}

	if( queue_leave(pl_sd,3) ) {
		sprintf(output, msg_txt(sd,(QUEUE_MSG_TXT+30)), pl_sd->status.name, queue->name);
		clif_displaymessage(fd, output);
		return 0;
	}
	
	sprintf(output, msg_txt(sd,(QUEUE_MSG_TXT+31)), pl_sd->status.name, queue->name);
	clif_displaymessage(fd, output);
	return -1;
}

ACMD_FUNC(queuesetdelay)
{
	int value = 0, delay = 0;
	char output[128];

	nullpo_retr(-1, sd);

	if( sd == NULL )
		return -1;

	if (!message || !*message || sscanf(message, "%d", &value) < 1) {
		clif_displaymessage(fd, msg_txt(sd,(QUEUE_MSG_TXT+32)));
		return -1;
	}

	if( value > battle_config.queue_join_delay )
		value = battle_config.queue_join_delay;

	sd->status.queue_delay = (int)time(NULL)+value;
	delay = sd->status.queue_delay-(int)time(NULL);
	clif_displaymessage(fd, msg_txt(sd,(QUEUE_MSG_TXT+33)));
	if( delay > 0 ) {
		sprintf(output, msg_txt(sd,(QUEUE_MSG_TXT+34)), queue_delay(sd->status.queue_delay));
		clif_displaymessage(fd, output);
	}
	return 0;
}

ACMD_FUNC(queuedelay)
{
	int delay = 0;
	char output[128];

	nullpo_retr(-1, sd);

	if( sd == NULL )
		return -1;

	// Update Delay.
	delay = sd->status.queue_delay-(int)time(NULL);
	if( delay > battle_config.queue_join_delay ) {
		sd->status.queue_delay = (int)time(NULL)+battle_config.queue_join_delay;
		delay = sd->status.queue_delay-(int)time(NULL);
	}

	if( delay <= 0 )
		clif_displaymessage(fd, msg_txt(sd,(QUEUE_MSG_TXT+35)));
	else {
		sprintf(output, msg_txt(sd,(QUEUE_MSG_TXT+34)), queue_delay(sd->status.queue_delay));
		clif_displaymessage(fd, output);
	}
	return 0;
}

ACMD_FUNC(queuenomove)
{
	nullpo_retr(-1, sd);

	if( sd == NULL )
		return -1;

	sd->queue_nomove = sd->queue_nomove ? 0 : 1;
	if( sd->queue_nomove )
		clif_displaymessage(fd, msg_txt(sd,(QUEUE_MSG_TXT+36)));
	else
		clif_displaymessage(fd, msg_txt(sd,(QUEUE_MSG_TXT+37)));
	return 0;
}