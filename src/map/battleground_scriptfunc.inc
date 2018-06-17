// © Creative Services and Development
// Site Oficial: www.creativesd.com.br
// Termos de Contrato e Autoria em: http://creativesd.com.br/?p=termos

/*==========================================
 * CreativeSD: Battleground Warfare
 *------------------------------------------*/
 BUILDIN_FUNC(bg_create_team)
 {
	struct battleground_data *bg;
	const char *map_name, *ev, *dev, *wev;
	int bg_id, x, y, rx, ry, army, map_index = 0;

	bg_id = script_getnum(st,2);

	if( bg_id <= 0 )
	{
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	map_name = script_getstr(st, 3);
	if (strcmp(map_name, "-") != 0) {
		map_index = mapindex_name2id(map_name);
		if (map_index == 0) { // Invalid Map
			script_pushint(st, 0);
			return SCRIPT_CMD_SUCCESS;
		}
	}

	x = script_getnum(st, 4);
	y = script_getnum(st, 5);
	rx = script_getnum(st, 6);
	ry = script_getnum(st, 7);
	army = script_getnum(st, 8);
	ev = script_getstr(st, 9);
	dev = script_getstr(st, 10);
	wev = script_getstr(st, 11);

	if( (bg = bg_team_search(bg_id)) != NULL )
	{
		bg->x = x;
		bg->y = y;
		bg->respawn_x = rx;
		bg->respawn_y = ry;
		safestrncpy(bg->logout_event, ev, sizeof(bg->logout_event));
		safestrncpy(bg->die_event, dev, sizeof(bg->die_event));
		safestrncpy(bg->without_event, wev, sizeof(bg->without_event));
		
		if( bg->army != army ) {
			bg->army = army;
			bg->g = &bg_guild[army];
			bg_guild_refresh_all(bg);
		}
	
		script_pushint(st, bg_id);
		return SCRIPT_CMD_SUCCESS;
	}

	if ((bg_id = bg_create(bg_id, map_index, x, y, rx, ry, army, ev, dev, wev)) == 0) { // Creation failed
		script_pushint(st, -1);
	}
	else
		script_pushint(st, bg_id);

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_join_team)
{
	TBL_PC* sd = NULL;
	int team_id = script_getnum(st, 2);
	int flag = 0;

	if (script_hasdata(st, 3))
		flag = script_getnum(st, 3);

	if (script_hasdata(st, 4))
		sd = map_id2sd(script_getnum(st, 4));
	else
		script_rid2sd(sd);

	if (!sd)
		script_pushint(st, 0);
	else
		script_pushint(st, bg_team_join(team_id, sd, flag) ? 0 : 1);

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_kick_team)
{
	int i, bg_id = script_getnum(st, 2);
	struct map_session_data *sd = NULL;
	struct battleground_data *bg = bg_team_search(bg_id);
	int delay = -1;

	if( script_hasdata(st,3) )
		delay = script_getnum(st,3);

	if( bg == NULL )
		script_pushint(st, 0); // Equipe não foi encontrada ou a id é inválida.
	else {
		for( i = 0; i < MAX_BG_MEMBERS; i++ )
		{
			if( (sd = bg->members[i].sd) == NULL )
				continue;
			else {
				bg_team_leave(sd, BGTL_LEFT);
				if( delay >= 0 ) sd->status.queue_delay = (int)time(NULL)+delay;
			}
		}
	}

	script_pushint(st, 1);
	return SCRIPT_CMD_SUCCESS;
}

 BUILDIN_FUNC(bg_team_search)
 {
	struct battleground_data *bg;
	int bg_id = script_getnum(st,0);

	if( bg_id <= 0 || (bg = bg_team_search(bg_id)) )
	{
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	script_pushint(st, bg->bg_id);
	return SCRIPT_CMD_SUCCESS;
 }

BUILDIN_FUNC(bg_team_size)
{
	struct battleground_data *bg;
	int bg_id = script_getnum(st, 2);
	int i, j = 0;

	if ((bg = bg_team_search(bg_id)) == NULL)
	{
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	for (i = 0; i < MAX_BG_MEMBERS; i++) {
		struct map_session_data *sd = bg->members[i].sd;
		if (sd == NULL)
			continue;

		j++;
	}
	script_pushint(st, j);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_get_team)
{
	struct battleground_data *bg;
	int bg_id = script_getnum(st, 2);
	int i, j = 0;

	if ((bg = bg_team_search(bg_id)) == NULL)
	{
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	for (i = 0; i < MAX_BG_MEMBERS; i++) {
		struct map_session_data *sd = bg->members[i].sd;
		if (sd == NULL)
			continue;

		set_reg(st, NULL, reference_uid(add_str(".@bg_members_cid"), j), ".@bg_members_cid", (void*)__64BPRTSIZE(sd->status.char_id), NULL);
		set_reg(st, NULL, reference_uid(add_str(".@bg_members_aid"), j), ".@bg_members_aid", (void*)__64BPRTSIZE(sd->status.account_id), NULL);
		j++;
	}

	set_reg(st, NULL, add_str(".@bg_members_count"), ".@bg_members_count", (void*)__64BPRTSIZE(j), NULL);
	script_pushint(st, j);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_set_respawn)
{
	struct battleground_data *bg = bg_team_search(script_getnum(st,2));

	if( bg == NULL )
	{
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	bg->respawn_x = script_getnum(st,3);
	bg->respawn_y = script_getnum(st,4);
	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_attach_respawn)
{
	TBL_PC* sd = NULL;
	struct battleground_data *bg = NULL;
	int timer = 1;

	if( script_hasdata(st,2) )
		timer = script_getnum(st,2);

	if (script_hasdata(st,3))
		sd = map_charid2sd(script_getnum(st,3));
	else
		script_rid2sd(sd);

	if( sd == NULL || !sd->bg_id || (bg = bg_team_search(sd->bg_id)) == NULL )
	{
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	sd->bg_respawn_timer_count = 10;
	sd->bg_respawn_timer = add_timer(gettick()+timer, bg_respawn_timer, sd->bl.id, 0);
	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_attach_respawn_all)
{
	struct map_session_data *sd = NULL;
	struct battleground_data *bg = bg_team_search(script_getnum(st,2));
	int timer = 1, i;

	if( bg == NULL )
	{
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	if( script_hasdata(st,3) )
		timer = script_getnum(st,3);

	for (i = 0; i < MAX_BG_MEMBERS; i++) {
		sd = bg->members[i].sd;
		if (sd == NULL)
			continue;

		sd->bg_respawn_timer_count = 10;
		sd->bg_respawn_timer = add_timer(gettick()+timer, bg_respawn_timer, sd->bl.id, 0);
	}
	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_viewpoint)
{
	struct battleground_data *bg;
	int bg_id, type, x, y, id, color, i;
	
	bg_id = script_getnum(st,2);
	type = script_getnum(st,3);
	x = script_getnum(st,4);
	y = script_getnum(st,5);
	id = script_getnum(st,6);
	color = script_getnum(st,7);

	bg = bg_team_search(bg_id);

	if (bg == NULL)
	{
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}
	
	for (i = 0; i < MAX_BG_MEMBERS; i++) {
		struct map_session_data *sd = bg->members[i].sd;
		if (sd == NULL)
			continue;

		clif_viewpoint(sd, st->oid, type, x, y, id, color);
	}

	script_pushint(st, 1);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_areapercentheal)
{
	const char *str;
	int16 m, x0, y0, x1, y1;
	int bg_id, hp, sp;
	int i = 0;
	struct battleground_data *bg = NULL;

	bg_id = script_getnum(st,2);
	str = script_getstr(st,3);

	if( (bg = bg_team_search(bg_id)) == NULL || (m = map_mapname2mapid(str)) < 0 ) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	x0 = script_getnum(st,4);
	y0 = script_getnum(st,5);
	x1 = script_getnum(st,6);
	y1 = script_getnum(st,7);
	hp = script_getnum(st,8);
	sp = script_getnum(st,9);

	for (i = 0; i < MAX_BG_MEMBERS; i++) {
		struct map_session_data *sd = bg->members[i].sd;
		if (sd == NULL)
			continue;
		if (sd->bl.m != m || sd->bl.x < x0 || sd->bl.y < y0 || sd->bl.x > x1 || sd->bl.y > y1)
			continue;

		pc_percentheal(sd,hp,sp);
	}
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_team_getxy)
{
	struct battleground_data *bg = bg_team_search(script_getnum(st,2));

	if( bg == NULL )
		return SCRIPT_CMD_SUCCESS;

	if( !script_getnum(st,3) )
		script_pushint(st,bg->x);
	else
		script_pushint(st,bg->y);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_respawn_getxy)
{
	struct battleground_data *bg = bg_team_search(script_getnum(st,2));

	if( bg == NULL )
		return SCRIPT_CMD_SUCCESS;

	if( !script_getnum(st,3) )
		script_pushint(st,bg->respawn_x);
	else
		script_pushint(st,bg->respawn_y);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_refresh_patent)
{
	struct battleground_data *bg;
	int bg_id = script_getnum(st, 2);

	if (!battle_config.bg_name_position)
		return SCRIPT_CMD_SUCCESS;

	bg = bg_team_search(bg_id);

	if (bg == NULL)
		return SCRIPT_CMD_SUCCESS;

	bg_refresh_patent(bg_id);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_message)
{
	const char *msg;
	int i;
	struct map_session_data *sd;
	struct battleground_data *bg = bg_team_search(script_getnum(st, 2));

	if (bg == NULL)
		return SCRIPT_CMD_SUCCESS;

	msg = script_getstr(st, 3);
	for (i = 0; i < MAX_BG_MEMBERS; i++)
	{
		if ((sd = bg->members[i].sd) == NULL)
			continue;
	
		clif_messagecolor(&sd->bl, BG_COLOR, msg, true, SELF);
	}
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_digit_timer)
{
	struct battleground_data *bg = bg_team_search(script_getnum(st, 2));
	int value = 0;

	if (bg == NULL)
		return SCRIPT_CMD_SUCCESS;

	if( script_hasdata(st,3) )
		value = script_getnum(st,3);

	if( value > 0 ) {
		bg->timerdigit_count = value+1;
		bg->timerdigit = add_timer(gettick() + 1, bg_digit_timer, bg->bg_id, 0);
	}
	else {
		bg->timerdigit_count = 0;
		bg->timerdigit = add_timer(gettick(), bg_digit_timer, bg->bg_id, 0);
	}
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_reward)
{
	TBL_PC* sd = NULL;
	struct battleground_rewards *bgr;
	int arena_id = script_getnum(st,2), type = script_getnum(st,3), count;

	if (script_hasdata(st,4))
		sd = map_charid2sd(script_getnum(st,4));
	else
		script_rid2sd(sd);

	if( sd == NULL )
	{
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	if ( (bgr = bg_search_reward(arena_id)) == NULL)
	{
		ShowError("script:bg_reward: Battleground (ID: %d) not found!\n", arena_id);
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	if( type < BGRT_WIN || type > BGRT_WO )
	{
		ShowError("script:bg_reward: Flag type (%d) incompatible.\n", type);
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	count = bg_reward(sd, arena_id, (enum bg_reward_type)type);
	script_pushint(st,count);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_get_score)
{
	TBL_PC* sd = NULL;
	int type = script_getnum(st,2), value = 0;

	if( script_hasdata(st,3) )
		sd = map_charid2sd(script_getnum(st,3));
	else
		script_rid2sd(sd);

	if( sd == NULL )
	{
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	switch(type)
	{
		case 0:  value = sd->bg_score.skills_support_success; break;
		case 1:  value = sd->bg_score.skills_support_fail; break;
		case 2:  value = sd->bg_score.heal_hp; break;
		case 3:  value = sd->bg_score.heal_sp; break;
		case 4:  value = sd->bg_score.item_heal_hp; break;
		case 5:  value = sd->bg_score.item_heal_sp; break;
		case 6:  value = sd->bg_score.item_heal; break;
		case 7:  value = sd->bg_score.ammos; break;
		case 8:  value = sd->bg_score.poison_bottle; break;
		case 9:  value = sd->bg_score.fire_bottle; break;
		case 10: value = sd->bg_score.acid_bottle; break;
		case 11: value = sd->bg_score.oridecon; break;
		case 12: value = sd->bg_score.elunium; break;
		case 13: value = sd->bg_score.steel; break;
		case 14: value = sd->bg_score.emveretarcon; break;
		case 15: value = sd->bg_score.wooden_block; break;
		case 16: value = sd->bg_score.stone; break;
		case 17: value = sd->bg_score.yellow_gemstone; break;
		case 18: value = sd->bg_score.red_gemstone; break;
		case 19: value = sd->bg_score.blue_gemstone; break;
		case 20: value = sd->bg_score.player_kills; break;
		case 21: value = sd->bg_score.player_deaths; break;
		case 22: value = sd->bg_score.player_damage_given; break;
		case 23: value = sd->bg_score.player_damage_taken; break;
		case 24: value = sd->bg_score.runestone_kills; break;
		case 25: value = sd->bg_score.runestone_damage; break;
		case 26: value = sd->bg_score.runestone_repair; break;
		case 27: value = sd->bg_score.emperium_kills; break;
		case 28: value = sd->bg_score.emperium_damage; break;
		case 29: value = sd->bg_score.barrier_kills; break;
		case 30: value = sd->bg_score.barrier_damage; break;
		case 31: value = sd->bg_score.barrier_repair; break;
		case 32: value = sd->bg_score.objective_kills; break;
		case 33: value = sd->bg_score.objective_damage; break;
		case 34: value = sd->bg_score.flag_kills; break;
		case 35: value = sd->bg_score.flag_damage; break;
		case 36: value = sd->bg_score.flag_capture; break;
		case 37: value = sd->bg_score.flag_recapture; break;
		case 38: value = sd->bg_score.cristal_kills; break;
		case 39: value = sd->bg_score.cristal_damage; break;
		case 40: value = sd->bg_score.guardian_kills; break;
		case 41: value = sd->bg_score.guardian_deaths; break;
		case 42: value = sd->bg_score.guardian_damage_given; break;
		case 43: value = sd->bg_score.guardian_damage_taken; break;
		default:
			script_pushint(st,0);
			ShowError("script:bg_get_score: Unknow type (%d)\n", type);
			return SCRIPT_CMD_SUCCESS;
	}

	script_pushint(st,value);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_set_score)
{
	TBL_PC* sd = NULL;
	int type = script_getnum(st,2), value = script_getnum(st,3);

	if( script_hasdata(st,4) )
		sd = map_charid2sd(script_getnum(st,4));
	else
		script_rid2sd(sd);

	if( sd == NULL )
	{
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	if( value > USHRT_MAX )
		value = USHRT_MAX;

	switch(type)
	{
		case 0:  sd->bg_score.skills_support_success = value; break;
		case 1:  sd->bg_score.skills_support_fail = value; break;
		case 2:  sd->bg_score.heal_hp = value; break;
		case 3:  sd->bg_score.heal_sp = value; break;
		case 4:  sd->bg_score.item_heal_hp = value; break;
		case 5:  sd->bg_score.item_heal_sp = value; break;
		case 6:  sd->bg_score.item_heal = value; break;
		case 7:  sd->bg_score.ammos = value; break;
		case 8:  sd->bg_score.poison_bottle = value; break;
		case 9:  sd->bg_score.fire_bottle = value; break;
		case 10:  sd->bg_score.acid_bottle = value; break;
		case 11: sd->bg_score.oridecon = value; break;
		case 12: sd->bg_score.elunium = value; break;
		case 13: sd->bg_score.steel = value; break;
		case 14: sd->bg_score.emveretarcon = value; break;
		case 15: sd->bg_score.wooden_block = value; break;
		case 16: sd->bg_score.stone = value; break;
		case 17: sd->bg_score.yellow_gemstone = value; break;
		case 18: sd->bg_score.red_gemstone = value; break;
		case 19: sd->bg_score.blue_gemstone = value; break;
		case 20: sd->bg_score.player_kills = value; break;
		case 21: sd->bg_score.player_deaths = value; break;
		case 22: sd->bg_score.player_damage_given = value; break;
		case 23: sd->bg_score.player_damage_taken = value; break;
		case 24: sd->bg_score.runestone_kills = value; break;
		case 25: sd->bg_score.runestone_damage = value; break;
		case 26: sd->bg_score.runestone_repair = value; break;
		case 27: sd->bg_score.emperium_kills = value; break;
		case 28: sd->bg_score.emperium_damage = value; break;
		case 29: sd->bg_score.barrier_kills = value; break;
		case 30: sd->bg_score.barrier_damage = value; break;
		case 31: sd->bg_score.barrier_repair = value; break;
		case 32: sd->bg_score.objective_kills = value; break;
		case 33: sd->bg_score.objective_damage = value; break;
		case 34: sd->bg_score.flag_kills = value; break;
		case 35: sd->bg_score.flag_damage = value; break;
		case 36: sd->bg_score.flag_capture = value; break;
		case 37: sd->bg_score.flag_recapture = value; break;
		case 38: sd->bg_score.cristal_kills = value; break;
		case 39: sd->bg_score.cristal_damage = value; break;
		case 40: sd->bg_score.guardian_kills = value; break;
		case 41: sd->bg_score.guardian_deaths = value; break;
		case 42: sd->bg_score.guardian_damage_given = value; break;
		case 43: sd->bg_score.guardian_damage_taken = value; break;
		default:
			script_pushint(st,0);
			ShowError("script:bg_set_score: Unknow type (%d)\n", type);
			return SCRIPT_CMD_SUCCESS;
	}

	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_set_npc)
{
	TBL_NPC* nd;
	int bg_id = script_getnum(st,2), x = 0, y = 0;
	const char *ev = "", *rev = "";

	if( bg_id < 0 )
		return SCRIPT_CMD_SUCCESS;

	if( script_hasdata(st,3) )
		x = script_getnum(st,3);
	if( script_hasdata(st,4) )
		y = script_getnum(st,4);
	if( script_hasdata(st,5) )
		ev = script_getstr(st,5);
	if( script_hasdata(st,6) )
		rev = script_getstr(st,6);

	check_event(st,ev);
	check_event(st,rev);

	nd = (TBL_NPC*)map_id2nd(st->oid);
	if( nd == NULL )
		ShowError("script:bg_set_npc: npc %d not found\n", st->oid);
	else if( nd->subtype != NPCTYPE_SCRIPT )
		ShowError("script:bg_set_npc: unexpected subtype %d for npc %d '%s'\n", nd->subtype, st->oid, nd->exname);
	else if( bg_set_npc(bg_id, nd, st->oid, x, y, ev, rev) ) {
		clif_guild_emblem_area(&nd->bl);
		script_pushint(st,1);
		return SCRIPT_CMD_SUCCESS;
	}

	script_pushint(st,0);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_npc_getid)
{
	TBL_NPC* nd = map_id2nd(st->oid);

	if( !nd ) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	script_pushint(st,nd->battleground.bg_id);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_flag_click)
{
	TBL_PC* sd;
	TBL_NPC *nd = (TBL_NPC*)map_id2nd(st->oid);
	
	if( nd == NULL )
		ShowError("script:bg_flag_click: npc %d not found\n", st->oid);
	else if( !script_rid2sd(sd) )
		ShowError("script:bg_flag_click: Player not attached!\n");
	else if( nd->battleground.bg_id && (nd->battleground.xs > 0 || nd->battleground.ys > 0) && sd->bg_flags.nd == NULL )
	{
		bg_flag_capture_area(sd, sd->bl.m, sd->bl.x, sd->bl.y);
		script_pushint(st,1);
		return SCRIPT_CMD_SUCCESS;
	}
	script_pushint(st,0);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_flag_respawn)
{
	const char *map_name;
	int16 m;
	int bg1_id = 0, bg2_id = 0;
	
	map_name = script_getstr(st,2);
	if( script_hasdata(st,3) )
		bg1_id = script_getnum(st,3);
	if( script_hasdata(st,4) )
		bg2_id = script_getnum(st,4);

	if( (m = map_mapname2mapid(map_name)) < 0 )
	{ // Invalid Map
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	bg_flag_respawn(m, bg1_id, bg2_id);
	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_console)
{
	const char *str;
	str=script_getstr(st,2);
	ShowMessage(CL_WHITE "[BattleGround]:" CL_RESET " %s\n",str);
	return SCRIPT_CMD_SUCCESS;
}
