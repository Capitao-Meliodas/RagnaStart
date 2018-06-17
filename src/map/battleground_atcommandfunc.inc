// © Creative Services and Development
// Site Oficial: www.creativesd.com.br
// Termos de Contrato e Autoria em: http://creativesd.com.br/?p=termos

/*==========================================
 * CreativeSD: Battleground Warfare
 *------------------------------------------*/
ACMD_FUNC(reloadbg)
{
	struct npc_data *nd = npc_name2id("BattleGround#queue_main");
	if (nd == NULL)
	{
		clif_displaymessage(sd->fd, msg_txt(sd,(BG_MSG_TXT+23)));
		return -1;
	}

	npc_event(sd, "BattleGround#queue_main::OnCmdReload", 0);
	clif_messagecolor(&sd->bl, BG_COLOR, msg_txt(sd,(BG_MSG_TXT+24)), true, SELF);
	return 0;
}

ACMD_FUNC(bgregister)
{
	struct npc_data *nd = npc_name2id("BattleGround#cmd_register");

	if (nd == NULL)
	{
		clif_displaymessage(sd->fd, msg_txt(sd,(BG_MSG_TXT+25)));
		return -1;
	}

	run_script(nd->u.scr.script, 0, sd->bl.id, nd->bl.id);
	return 0;
}

ACMD_FUNC(bgjoin)
{
	struct npc_data *nd = npc_name2id("BattleGround#cmd_join");

	if (nd == NULL)
	{
		clif_displaymessage(sd->fd, msg_txt(sd,(BG_MSG_TXT+26)));
		return -1;
	}

	run_script(nd->u.scr.script, 0, sd->bl.id, nd->bl.id);
	return 0;
}

ACMD_FUNC(bgleave)
{
	int16 m = sd->bl.m;
	if (sd->bg_id)
	{
		if (map[m].flag.battleground)
		{
			bg_team_leave(sd, BGTL_QUIT);
			if( !sd->bg_id )
			{
				if (battle_config.bg_afk_warp_save_point)
					pc_setpos(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, CLR_TELEPORT);
				else {
					unsigned short map_index = mapindex_name2id("bat_room");
					if (map_index)
						pc_setpos(sd, map_index, 154, 150, CLR_TELEPORT);
				}
				return 0;
			}
			else {
				clif_displaymessage(sd->fd, msg_txt(sd,(BG_MSG_TXT+27)));
				return 0;
			}
		}
		else {
			// Não ativar labeis de eventos por estar fora da arena.
			bg_team_leave(sd, BGTL_LEFT);
			if (sd->bg_id)
			{
				clif_displaymessage(sd->fd, msg_txt(sd,(BG_MSG_TXT+28)));
				return 1;
			}
			else {
				clif_displaymessage(sd->fd, msg_txt(sd,(BG_MSG_TXT+27)));
				return -1;
			}
		}
	}
	else {
		clif_displaymessage(sd->fd, msg_txt(sd,(BG_MSG_TXT+29)));
		return -1;
	}
}

ACMD_FUNC(bgunregister)
{
	struct npc_data *nd = npc_name2id("BattleGround#cmd_unregister");

	if (nd == NULL)
	{
		clif_displaymessage(sd->fd, msg_txt(sd,(BG_MSG_TXT+25)));
		return -1;
	}

	run_script(nd->u.scr.script, 0, sd->bl.id, nd->bl.id);
	return 0;
}

ACMD_FUNC(bgreportafk)
{
	struct map_session_data *pl_sd = NULL;
	char output[128];
	
	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));
	if (!*message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1)
	{
		clif_displaymessage(fd, msg_txt(sd,(BG_MSG_TXT+30)));
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_player_name,true)) == NULL && (pl_sd = map_charid2sd(atoi(atcmd_player_name))) == NULL)
	{
		sprintf(atcmd_output, msg_txt(sd,(BG_MSG_TXT+31)), atcmd_player_name);
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	if (!pl_sd->bg_id)
	{
		sprintf(atcmd_output, msg_txt(sd,(BG_MSG_TXT+32)), message);
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	if (battle_config.bg_afk_team_report && sd->bg_id != pl_sd->bg_id)
	{
		clif_displaymessage(fd, msg_txt(sd,(BG_MSG_TXT+33)));
		return -1;
	}

	if (pl_sd->npc_id)
	{
		clif_displaymessage(fd, msg_txt(sd,(BG_MSG_TXT+34)));
		return -1;
	}

	if (!DIFF_TICK(last_tick, pl_sd->idletime))
	{
		clif_displaymessage(fd, msg_txt(sd,(BG_MSG_TXT+35)));
		return -1;
	}

	if (pl_sd->bg_afk_timer && pl_sd->bg_afk_timer != INVALID_TIMER)
	{
		clif_displaymessage(fd, msg_txt(sd,(BG_MSG_TXT+36)));
		return -1;
	}

	clif_displaymessage(fd, msg_txt(sd,(BG_MSG_TXT+37)));
	sprintf(output, msg_txt(sd,(BG_MSG_TXT+38)), battle_config.bg_afk_timer);
	clif_messagecolor(&pl_sd->bl, BG_COLOR, output, true, SELF);

	pl_sd->bg_afk_timer = add_timer(gettick() + (battle_config.bg_afk_timer * 1000), bg_report_afk, pl_sd->bl.id, 0);
	return 0;
}

ACMD_FUNC(bgwaiting)
{
	struct npc_data *nd;
	struct script_state *st;
	int arg1 = 0, arg2 = 0, count = 0, nargs = 0 ;

	if (strcmpi("bgwaiting",command))
	{
		nd = npc_name2id("BattleGround#cmd_waiting");
		if (nd == NULL)
		{
			clif_displaymessage(sd->fd, msg_txt(sd,(BG_MSG_TXT+39)));
			return -1;
		}
	}
	else
	{
		nd = npc_name2id("BattleGround#cmd_playing");
		if (nd == NULL)
		{
			clif_displaymessage(sd->fd, msg_txt(sd,(BG_MSG_TXT+40)));
			return -1;
		}
	}

	if (*message)
		count = sscanf(message, "%2d %1d", &arg1, &arg2);

	st = script_alloc_state(nd->u.scr.script, 0, sd->bl.id, nd->bl.id);

	if (count > 0)
		setd_sub(st, NULL, ".@atcmd_parameters", nargs++, (void *)&arg1, NULL);

	if (count > 1)
		setd_sub(st, NULL, ".@atcmd_parameters", nargs++, (void *)&arg2, NULL);

	setd_sub(st, NULL, ".@atcmd_numparameters", 0, (void *)&nargs, NULL);
	run_script_main(st);
	return 0;
}

ACMD_FUNC(bgranking)
{
	struct npc_data *nd;
	struct script_state *st;
	int arg1 = 0, arg2 = 0, arg3 = 0, count = 0, nargs = 0;

	nd = npc_name2id("BattleGround#cmd_ranking");
	if (nd == NULL)
	{
		clif_displaymessage(sd->fd, msg_txt(sd,(BG_MSG_TXT+41)));
		return -1;
	}

	if (*message)
		count = sscanf(message, "%2d %1d %2d", &arg1, &arg2, &arg3);

	st = script_alloc_state(nd->u.scr.script, 0, sd->bl.id, nd->bl.id);

	if (count > 0)
		setd_sub(st, NULL, ".@atcmd_parameters", nargs++, (void *)&arg1, NULL);

	if (count > 1)
		setd_sub(st, NULL, ".@atcmd_parameters", nargs++, (void *)&arg2, NULL);

	if (count > 2)
		setd_sub(st, NULL, ".@atcmd_parameters", nargs++, (void *)&arg3, NULL);

	setd_sub(st, NULL, ".@atcmd_numparameters", 0, (void *)&nargs, NULL);
	run_script_main(st);
	return 0;
}

ACMD_FUNC(bgorder)
{
	struct battleground_data *bg = NULL;
	nullpo_retr(-1,sd);

	if( !message || !*message )
	{
		clif_displaymessage(fd, msg_txt(sd,(BG_MSG_TXT+42)));
		return -1;
	}

	if( map[sd->bl.m].flag.battleground && sd->bg_id )
	{
		if( (bg = bg_team_search(sd->bg_id)) == NULL && bg->master_id != sd->status.char_id )
		{
			clif_displaymessage(fd, msg_txt(sd,(BG_MSG_TXT+43)));
			return -1;
		}
		clif_broadcast2(&sd->bl, message, (int)strlen(message)+1, BG_COLOR, 0x190, 12, 0, 0, BG);
	}
	else {
		clif_displaymessage(fd, msg_txt(sd,(BG_MSG_TXT+44)));
		return -1;
	}

	return 0;
}

ACMD_FUNC(bgchangeleader)
{
	struct battleground_data *bg = NULL;
	struct map_session_data *pl_sd = NULL;
	struct map_session_data *tsd = NULL;
	char output[128];
	int i, showlist = 0;

	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!sd->bg_id)
	{
		clif_displaymessage(fd, msg_txt(sd,(BG_MSG_TXT+45)));
		return -1;
	}

	bg = bg_team_search(sd->bg_id);
	if (bg == NULL || bg->master_id != sd->status.char_id )
	{
		clif_displaymessage(fd, msg_txt(sd,(BG_MSG_TXT+46)));
		return -1;
	}
	else if (bg->count <= 1)
	{
		clif_displaymessage(fd, msg_txt(sd,(BG_MSG_TXT+47)));
		return -1;
	}
	else if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1)
		showlist = 1;
	else if ((pl_sd = map_nick2sd(atcmd_player_name,true)) == NULL || pl_sd->bg_id != sd->bg_id) {
		clif_displaymessage(fd, msg_txt(sd,(BG_MSG_TXT+48)));
		showlist = 1;
	}
	else {
		if (bg_change_master(sd->bg_id, pl_sd->status.char_id, sd))
		{
			sprintf(output, msg_txt(sd,(BG_MSG_TXT+49)), pl_sd->status.name);
			clif_displaymessage(fd, output);
			return 0;
		}
		else {
			sprintf(output, msg_txt(sd,(BG_MSG_TXT+50)), pl_sd->status.name);
			clif_displaymessage(fd, output);
			showlist = 1;
		}
	}

	if (showlist)
	{
		clif_displaymessage(fd, msg_txt(sd,(BG_MSG_TXT+51)));
		for (i = 0; i < bg->count; i++)
		{
			if (bg->members[i].sd == sd)
				continue;

			tsd = bg->members[i].sd;
			if (tsd == NULL)
				continue;

			sprintf(output, msg_txt(sd,(BG_MSG_TXT+52)), tsd->status.name, tsd->status.base_level, tsd->status.job_level, job_name(tsd->status.class_));
			clif_displaymessage(fd, output);
		}
	}

	return -1;
}

ACMD_FUNC(bgreloadrewards)
{
	do_bg_reward_reload();
	clif_displaymessage(fd, msg_txt(sd,(BG_MSG_TXT+53)));
	return 0;
}
