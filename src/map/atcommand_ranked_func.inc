// © Creative Services and Development
// Site Oficial: www.creativesd.com.br
// Termos de Contrato e Autoria em: http://creativesd.com.br/?p=termos

/*==========================================
 * CreativeSD: Ranked System
 *------------------------------------------*/
ACMD_FUNC(reloadrankdb)
{
	nullpo_retr(-1, sd);
	do_final_ranked();
	do_reload_ranked();
	clif_displaymessage(fd, msg_txt(sd, (RANKED_MSGTXT+6)));
	return 0;
}

ACMD_FUNC(rankinfo)
{
	struct ranked_data *rank;
	char output[200];
	int rank_id;
	float points;

	nullpo_retr(-1,sd);

	points = ranked_config.show_mode ? sd->ranked.account_points : sd->status.ranked_points;
	rank_id = ranked_get_rank(points);

	if( rank_id <= 0 || (rank = ranked_search(rank_id)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(sd,(RANKED_MSGTXT+7)));
		sprintf(output, msg_txt(sd,(RANKED_MSGTXT+8)), points);
		clif_displaymessage(fd, output);
		return 0;
	}

	sprintf(output, msg_txt(sd,(RANKED_MSGTXT+9)), rank->g->name);
	clif_displaymessage(fd, output);
	sprintf(output, msg_txt(sd,(RANKED_MSGTXT+8)), points);
	clif_displaymessage(fd, output);
	return 0;
}

ACMD_FUNC(rankinfo2)
{
	struct ranked_data *rank, *nrank;
	char output[200];
	int rank_id, next_rank_id;
	float points;

	nullpo_retr(-1,sd);

	points = ranked_config.show_mode ? sd->ranked.account_points : sd->status.ranked_points;
	rank_id = ranked_get_rank(points);

	if( rank_id <= 0 || (rank = ranked_search(rank_id)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(sd,(RANKED_MSGTXT+7)));
		sprintf(output, msg_txt(sd,(RANKED_MSGTXT+8)), points);
		clif_displaymessage(fd, output);

		// More Infos
		next_rank_id = ranked_get_next(points);
		if( next_rank_id && (nrank = ranked_search(next_rank_id)) != NULL )
		{
			sprintf(output, msg_txt(sd,(RANKED_MSGTXT+10)), (nrank->points-points), nrank->g->name);
			clif_displaymessage(fd, output);
		}
		return 0;
	}

	sprintf(output, msg_txt(sd,(RANKED_MSGTXT+9)), rank->g->name);
	clif_displaymessage(fd, output);
	sprintf(output, msg_txt(sd,(RANKED_MSGTXT+8)), points);
	clif_displaymessage(fd, output);

	// More Infos
	next_rank_id = ranked_get_next(points);
	if( next_rank_id && (nrank = ranked_search(next_rank_id)) != NULL )
	{
		sprintf(output, msg_txt(sd,(RANKED_MSGTXT+10)), (nrank->points-points), nrank->g->name);
		clif_displaymessage(fd, output);
	}
	return 0;
}

ACMD_FUNC(rankinfo3)
{
	struct ranked_data *rank, *nrank;
	struct map_session_data *pl_sd;
	char output[200];
	int rank_id, next_rank_id;
	float points;

	nullpo_retr(-1,sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, msg_txt(sd,(RANKED_MSGTXT+11)));
		return -1;
	}

	if ( (pl_sd = map_nick2sd(atcmd_player_name,true)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(sd,3)); // Character not found.
		return -1;
	}

	points = ranked_config.show_mode ? pl_sd->ranked.account_points : pl_sd->status.ranked_points;
	rank_id = ranked_get_rank(points);

	if( rank_id <= 0 || (rank = ranked_search(rank_id)) == NULL )
	{
		sprintf(output, msg_txt(sd,(RANKED_MSGTXT+12)), pl_sd->status.name);
		clif_displaymessage(fd, output);
		sprintf(output, msg_txt(sd,(RANKED_MSGTXT+8)), points);
		clif_displaymessage(fd, output);

		// More Infos
		next_rank_id = ranked_get_next(points);
		if( next_rank_id && (nrank = ranked_search(next_rank_id)) != NULL )
		{
			sprintf(output, msg_txt(sd,(RANKED_MSGTXT+10)), (nrank->points-points), nrank->g->name);
			clif_displaymessage(fd, output);
		}
		return 0;
	}

	sprintf(output, msg_txt(sd,(RANKED_MSGTXT+13)), pl_sd->status.name);
	clif_displaymessage(fd, output);
	sprintf(output, msg_txt(sd,(RANKED_MSGTXT+9)), rank->g->name);
	clif_displaymessage(fd, output);
	sprintf(output, msg_txt(sd,(RANKED_MSGTXT+8)), points);
	clif_displaymessage(fd, output);

	// More Infos
	next_rank_id = ranked_get_next(points);
	if( next_rank_id && (nrank = ranked_search(next_rank_id)) != NULL )
	{
		sprintf(output, msg_txt(sd,(RANKED_MSGTXT+10)), (nrank->points-points), nrank->g->name);
		clif_displaymessage(fd, output);
	}
	return 0;
}


ACMD_FUNC(rankshow)
{
	int16 m;
	nullpo_retr(-1,sd);

	m = map_id2index(sd->bl.m); // Get Mapindex
	if( !map[sd->bl.m].flag.town ) {
		clif_displaymessage(sd->fd, msg_txt(sd,(RANKED_MSGTXT+14)));
	}
	else if( sd->ranked.disable && ranked_show_map(sd->bl.m) ) {
		int rk_chg_timer = sd->status.guild_id ? ranked_config.show_timer : 1;
		sd->ranked.disable = 0;
		sd->ranked.display_timer = add_timer(gettick()+rk_chg_timer,ranked_change_emblem,sd->bl.id,0);
		clif_displaymessage(sd->fd, msg_txt(sd,(RANKED_MSGTXT+15)));
	}
	else {
		// Free Timers
		delete_timer(sd->ranked.display_timer,ranked_change_emblem);
		sd->ranked.disable = 1;
		sd->ranked.display_timer = INVALID_TIMER;
		sd->ranked.display_pos = RANK_NONE;
		clif_displaymessage(sd->fd, msg_txt(sd,(RANKED_MSGTXT+16)));
		clif_remove_belonginfo(sd);
		if( sd->status.guild_id )
		{
			struct guild *g = guild_search(sd->status.guild_id);
			if( g != NULL )
			{
				clif_guild_emblem(sd, g);
				clif_guild_belonginfo(sd);
				clif_guild_emblem_area(&sd->bl);
			}
		}
		pc_setpos(sd, m, sd->bl.x, sd->bl.y, CLR_TELEPORT);
	}
	return 0;
}

ACMD_FUNC(ranklist)
{	
	nullpo_retr(-1,sd);
	ranked_show_list(sd);
	return 0;
}

ACMD_FUNC(setrank)
{
	struct ranked_data *rank;
	int rank_id, type;

	nullpo_retr(-1,sd);

	if (!message || !*message || sscanf(message, "%d %d", &rank_id, &type) < 2 || (type < RANK_CHAR || type > RANK_BOTH) ) {
		clif_displaymessage(sd->fd, msg_txt(sd,(RANKED_MSGTXT+17)));
		clif_displaymessage(sd->fd, msg_txt(sd,(RANKED_MSGTXT+18)));
		clif_displaymessage(sd->fd, msg_txt(sd,(RANKED_MSGTXT+19)));
		clif_displaymessage(sd->fd, msg_txt(sd,(RANKED_MSGTXT+20)));
		clif_displaymessage(sd->fd, msg_txt(sd,(RANKED_MSGTXT+21)));
		ranked_show_list(sd);
		return -1;
	}
	
	if( (rank = ranked_search(rank_id)) != NULL )
	{
		type++;
		if( type&1 )
			sd->ranked.account_points = rank->points;
		if( type&2 )
			sd->status.ranked_points = rank->points;
			
		ranked_update_rank(sd,0,(enum ranked_update)(type-1),RANK_P_OTHERS);
		clif_displaymessage(sd->fd, msg_txt(sd,(RANKED_MSGTXT+22)));
		pc_setpos(sd, map_id2index(sd->bl.m), sd->bl.x, sd->bl.y, CLR_TELEPORT);
		return 0;
	}
	
	clif_displaymessage(sd->fd, msg_txt(sd,(RANKED_MSGTXT+23)));
	return -1;
}

ACMD_FUNC(setrankpoints)
{
	int type;
	float points;

	nullpo_retr(-1,sd);

	if (!message || !*message || sscanf(message, "%d %f", &type, &points) < 2 || (type < RANK_CHAR || type > RANK_BOTH) ) {
		clif_displaymessage(sd->fd, msg_txt(sd,(RANKED_MSGTXT+24)));
		clif_displaymessage(sd->fd, msg_txt(sd,(RANKED_MSGTXT+18)));
		clif_displaymessage(sd->fd, msg_txt(sd,(RANKED_MSGTXT+19)));
		clif_displaymessage(sd->fd, msg_txt(sd,(RANKED_MSGTXT+20)));
		clif_displaymessage(sd->fd, msg_txt(sd,(RANKED_MSGTXT+21)));
		return -1;
	}

	ranked_update_rank(sd,points,(enum ranked_update)type,RANK_P_OTHERS);
	pc_setpos(sd, map_id2index(sd->bl.m), sd->bl.x, sd->bl.y, CLR_TELEPORT);
	return 0;
}