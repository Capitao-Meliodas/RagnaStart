// © Creative Services and Development
// Site Oficial: www.creativesd.com.br
// Termos de Contrato e Autoria em: http://creativesd.com.br/?p=termos

/*==========================================
 * CreativeSD: Battleground Warfare
 *------------------------------------------*/
struct guild* bg_get_guild(int idx)
{
	if( idx > 1 )
		return NULL;
	return &bg_guild[idx];
}

// Atualiza Patentes
int bg_refresh_patent(int bg_id)
{
	struct battleground_data *bg = bg_team_search(bg_id);
	struct map_session_data *sd;
	char output[256];
	int i, p, j = 0;
	int army;

	if (bg == NULL)
	{
		ShowError("bg_refresh_patent: Team not found.\n");
		return false;
	}

	army = bg->army;
	if (army < 0 || army > 1)
	{
		ShowError("bg_refresh_patent: Unknown Army.\n");
		return false;
	}

	for (i = 0; i < MAX_BG_MEMBERS; i++)
	{
		if ((sd = bg->members[i].sd) == NULL)
			continue;

		p = j>=MAX_BG_POSITION?(MAX_BG_POSITION - 1):j;
		if( p != sd->bg_position )
		{
			sd->bg_position = p;

			sprintf(output, msg_txt(sd,(BG_MSG_TXT+4)), bg->g->position[j].name, bg->g->name);
			clif_messagecolor(&sd->bl, BG_COLOR, output, true, SELF);
			if (j == 0 ) {
				bg->master_id = sd->status.char_id;
				bg_block_skill_change(sd);
			}
		}

		bg_guild_requestinfo(sd);
		j++;
	}
	return true;
}

// Limpa as habilidades de clã das batalhas campais.
void bg_clean_skill(struct battleground_data *bg)
{
	memset(bg->skill_block_timer, 0, sizeof(bg->skill_block_timer));
}

// Bloqueio de Habilidades
void bg_block_skill(struct map_session_data *sd, int time)
{
	struct battleground_data *bg = NULL;
	uint16 skill_id[] = { GD_BATTLEORDER, GD_REGENERATION, GD_RESTORE, GD_EMERGENCYCALL };
	int idx, i;

	if( !sd->bg_id || (bg = bg_team_search(sd->bg_id)) == NULL )
		return;

	for (i = 0; i < 4; i++) {
		idx = skill_id[i] - GD_SKILLBASE;
		skill_blockpc_start(sd, skill_id[i], time);
		bg->skill_block_timer[idx] = gettick()+time;
	}
}

// Altera o delay de habilidades de Clã das Batalhas Campais para outro jogador.
void bg_block_skill_change(struct map_session_data *sd)
{
	struct battleground_data *bg;
	uint16 skill_id[] = { GD_BATTLEORDER, GD_REGENERATION, GD_RESTORE, GD_EMERGENCYCALL };
	int i, idx, timer;

	if( !sd->bg_id || (bg = bg_team_search(sd->bg_id)) == NULL )
		return;

	for (i=0; i < 4; i++) {
		idx = skill_id[i] - GD_SKILLBASE;
		timer = bg->skill_block_timer[idx];

		if( timer >= gettick() ) {
			bg->skill_block_timer[idx] = 0;
			continue;
		}

		timer -= gettick();
		skill_blockpc_start(sd, skill_id[i], timer);
	}
}

// Remove os delays das habilidades de Clã das Batalhas Campais.
void bg_block_skill_end(struct map_session_data *sd)
{
	uint16 skill_id[] = { GD_BATTLEORDER, GD_REGENERATION, GD_RESTORE, GD_EMERGENCYCALL };
	int i;

	for (i=0; i < 4; i++) {
		//idx = skill_id[i] - GD_SKILLBASE;
		//sd->blockskill[skill_id[i]] = 0;
		
		if( battle_config.display_status_timers )
			clif_skill_cooldown(sd, skill_id[i], 0);
	}
}

// Reportagem de AFK
int bg_report_afk(int tid, unsigned int tick, int id, intptr_t data)
{
	struct map_session_data *sd;

	sd = map_id2sd(id);
	if (sd)
	{
		if (sd->bg_afk_timer != INVALID_TIMER && sd->bg_id && !sd->npc_id && DIFF_TICK(last_tick, sd->idletime))
		{
			bg_team_leave(sd, BGTL_AFK);
			if (battle_config.bg_afk_warp_save_point)
				pc_setpos(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, CLR_TELEPORT);
			else {
				unsigned short map_index = mapindex_name2id("bat_room");
				if (map_index)
					pc_setpos(sd, map_index, 154, 150, CLR_TELEPORT);
			}
		}
		sd->bg_afk_timer = INVALID_TIMER;
		return 1;
	}
	return 0;
}

// Expulsão de Jogadores
int bg_kick_player(struct map_session_data* sd, struct map_session_data *pl_sd, const char* mes)
{
	struct battleground_data *bg;
	int i, c = 0;
	char output[200];

	if( (bg = bg_team_search(sd->bg_id)) == NULL )
		return false;

	ARR_FIND(0, MAX_BG_MEMBERS, i, pl_sd->bg_kick.char_id[i] == sd->status.char_id);
	if( i < MAX_BG_MEMBERS ) {
		if( i == 0 ) {
			sprintf(output, msg_txt(sd,(BG_MSG_TXT+5)), pl_sd->status.name);
			clif_messagecolor(&sd->bl, color_table[COLOR_RED], output, false, SELF);
		}
		else {
			sprintf(output, msg_txt(sd,(BG_MSG_TXT+6)), pl_sd->status.name);
			clif_messagecolor(&sd->bl, color_table[COLOR_RED], output, false, SELF);
		}
		sprintf(output, msg_txt(sd,(BG_MSG_TXT+7)), pl_sd->bg_kick.mes);
		clif_messagecolor(&sd->bl, color_table[COLOR_RED], output, false, SELF);
		return false;
	}

	ARR_FIND(0, MAX_BG_MEMBERS, i, pl_sd->bg_kick.char_id[i] == 0 );
	if( i == 0 && strlen(mes) == 0 )
	{
		sprintf(output, msg_txt(sd,(BG_MSG_TXT+8)), pl_sd->status.name);
		clif_messagecolor(&sd->bl, color_table[COLOR_RED], output, false, SELF);
		return false;
	}

	// Ceil is not working in this formula, make the manual rounding...
	c = (bg->count/2)+1;
	while( c >= bg->count ) {
		c--;
	}
	
	pl_sd->bg_kick.char_id[i] = sd->status.char_id;
	pl_sd->bg_kick.count++;

	// Notify Members
	if( i == 0 )
	{
		safestrncpy(pl_sd->bg_kick.mes, mes, sizeof(pl_sd->bg_kick.mes));
		sprintf(output, msg_txt(sd,(BG_MSG_TXT+9)), sd->status.name, pl_sd->status.name);
		clif_bg_message(bg, 0, "Server", output, strlen(output)+1);
	}

	sprintf(output, msg_txt(sd,(BG_MSG_TXT+10)), pl_sd->status.name);
	clif_bg_message(bg, 0, "Server", output, strlen(output)+1);
	sprintf(output, msg_txt(sd,(BG_MSG_TXT+11)), mes);
	clif_bg_message(bg, 0, "Server", output, strlen(output)+1);
	sprintf(output, msg_txt(sd,(BG_MSG_TXT+12)), pl_sd->bg_kick.count, c);
	clif_bg_message(bg, 0, "Server", output, strlen(output)+1);

	if( pl_sd->bg_kick.count >= c ) {
		bg_team_leave(pl_sd, BGTL_KICK);
		pc_setpos(pl_sd,pl_sd->status.save_point.map,pl_sd->status.save_point.x,pl_sd->status.save_point.y,CLR_TELEPORT);
	}
	return true;
}

// Altera o Capitão da Batalha.
int bg_change_master(int bg_id, int char_id, struct map_session_data *sd)
{
	struct battleground_data *bg = bg_team_search(bg_id);
	struct map_session_data *tsd;
	int i, x1, y1, x2, y2, m_afk, n_afk, j = 0;
	if (bg == NULL)
		return 0;

	for (i = 0; i < MAX_BG_MEMBERS; i++)
	{
		if ((tsd = bg->members[i].sd) == NULL)
			continue;

		if (tsd->status.char_id == char_id)
		{
			m_afk = bg->members[0].afk;
			n_afk = bg->members[i].afk;
			x1 = bg->members[0].x;
			y1 = bg->members[0].y;
			x2 = bg->members[i].x;
			y2 = bg->members[i].y;

			bg->members[0].sd = tsd;
			bg->members[0].x = x2;
			bg->members[0].y = y2;
			bg->members[0].afk = m_afk;

			bg->members[i].sd = sd;
			bg->members[i].x = x1;
			bg->members[i].y = y1;
			bg->members[i].afk = n_afk;
			bg_refresh_patent(bg_id);
			return 1;
		}
		j++;
	}
	return 0;
}

// Respawn automático.
int bg_respawn_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	struct map_session_data *sd;
	struct battleground_data *bg;
	char cutin[NAME_LENGTH];
	char sound[NAME_LENGTH];
	sd = map_id2sd(id);

	if (sd)
	{
		if (sd->bg_id && (bg = bg_team_search(sd->bg_id)) != NULL && sd->bg_respawn_timer != INVALID_TIMER )
		{
			if( sd->bg_respawn_timer_count < 0 )
			{
				clif_cutin(sd,"",255);
				sd->bg_respawn_timer = INVALID_TIMER;
				sd->bg_respawn_timer_count = 0;
				pc_percentheal(sd,100,100);
				pc_setpos(sd, bg->mapindex, bg->respawn_x, bg->respawn_y, CLR_OUTSIGHT);
				bg_guild_requestinfo(sd);
			}
			else {
				sd->bg_respawn_timer_count--;
				sprintf(cutin,"respawn_%d",sd->bg_respawn_timer_count+1);
				sprintf(sound,"respawn_%s.wav",sd->bg_respawn_timer_count<0?"go":"beep");
				clif_cutin(sd,cutin,2);
				clif_soundeffect(sd,&sd->bl,sound,0);
				sd->bg_respawn_timer = add_timer(gettick() + 1000, bg_respawn_timer, sd->bl.id, 0);
			}
		}
		else {
			sd->bg_respawn_timer = INVALID_TIMER;
			sd->bg_respawn_timer_count = 0;
		}
		return 1;
	}
	return 0;
}

void bg_respawn_timer_delete(struct map_session_data *sd)
{
	sd->bg_respawn_timer = INVALID_TIMER;
	sd->bg_respawn_timer_count = 0;
	clif_cutin(sd,"",255);
}

int bg_digit_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	struct map_session_data *sd = NULL;
	struct battleground_data *bg = bg_team_search(id);
	int i;
	
	if( bg == NULL && bg->timerdigit != INVALID_TIMER )
		return 0;

	for (i = 0; i < MAX_BG_MEMBERS; i++)
	{
		if ((sd = bg->members[i].sd) == NULL)
			continue;

		if( bg->timerdigit_count > 0 )
			clif_showdigit(sd, (unsigned char)2, -(bg->timerdigit_count));
		else
			clif_showdigit(sd, (unsigned char)3, 0);
	}

	if( bg->timerdigit_count <= 0 )
		bg->timerdigit = INVALID_TIMER;
	else {
		bg->timerdigit_count--;
		bg->timerdigit = add_timer(gettick() + 1000, bg_digit_timer, bg->bg_id, 0);
	}

	return 1;
}

// Cria o Clã dentro da estrutura das Batalhas.
void bg_create_guild()
{
	int j;
	memset(&bg_guild, 0, sizeof(bg_guild));
	for( j=0; j <= 1; j++ )
	{
		FILE* fp = NULL;
		char path[256];
		int i, skill;

		bg_guild[j].emblem_id = 1;
		bg_guild[j].guild_id = SHRT_MAX-j;
		bg_guild[j].guild_lv = 50;
		bg_guild[j].max_member = MAX_BG_MEMBERS;

		for( i=0; i < MAX_GUILDSKILL; i++ )
		{
			skill = i + GD_SKILLBASE;
			bg_guild[j].skill[i].id = skill;
			switch( skill )
			{
				case GD_GLORYGUILD:
					bg_guild[j].skill[i].lv = 0;
					break;
				case GD_APPROVAL:
				case GD_KAFRACONTRACT:
				case GD_GUARDRESEARCH:
				case GD_BATTLEORDER:
				case GD_RESTORE:
				case GD_EMERGENCYCALL:
				case GD_DEVELOPMENT:
					bg_guild[j].skill[i].lv = 1;
					break;
				case GD_GUARDUP:
				case GD_REGENERATION:
					bg_guild[j].skill[i].lv = 3;
					break;
				case GD_LEADERSHIP:
				case GD_GLORYWOUNDS:
				case GD_SOULCOLD:
				case GD_HAWKEYES:
					bg_guild[j].skill[i].lv = 5;
					break;
				case GD_EXTENSION:
					bg_guild[j].skill[i].lv = 10;
					break;
			}
		}

		if( j == 1 )
		{
			// Guild Data - Croix
			strncpy(bg_guild[j].name, msg_txt(NULL,(BG_MSG_TXT+58)), NAME_LENGTH);
			strncpy(bg_guild[j].master, msg_txt(NULL,(BG_MSG_TXT+59)), NAME_LENGTH);
			memcpy(bg_guild[j].mes1, msg_txt(NULL,(BG_MSG_TXT+60)),MAX_GUILDMES1);
			memcpy(bg_guild[j].mes2, msg_txt(NULL,(BG_MSG_TXT+61)),MAX_GUILDMES2);
		}
		else {
			// Exército de Guillaume
			strncpy(bg_guild[j].name, msg_txt(NULL,(BG_MSG_TXT+54)), NAME_LENGTH);
			strncpy(bg_guild[j].master, msg_txt(NULL,(BG_MSG_TXT+55)), NAME_LENGTH);
			memcpy(bg_guild[j].mes1, msg_txt(NULL,(BG_MSG_TXT+56)),MAX_GUILDMES1);
			memcpy(bg_guild[j].mes2, msg_txt(NULL,(BG_MSG_TXT+57)),MAX_GUILDMES2);
		}

		strncpy(bg_guild[j].position[0].name, msg_txt(NULL,(BG_MSG_TXT+62)), NAME_LENGTH);
		strncpy(bg_guild[j].position[1].name, msg_txt(NULL,(BG_MSG_TXT+63)), NAME_LENGTH);
		strncpy(bg_guild[j].position[2].name, msg_txt(NULL,(BG_MSG_TXT+64)), NAME_LENGTH);
		strncpy(bg_guild[j].position[3].name, msg_txt(NULL,(BG_MSG_TXT+65)), NAME_LENGTH);

		sprintf(path, "%s/emblems/bg_%d.ebm", db_path, (j+1));
		if( (fp = fopen(path, "rb")) != NULL )
		{
			fseek(fp, 0, SEEK_END);
			bg_guild[j].emblem_len = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			fread(bg_guild[j].emblem_data, 1, bg_guild[j].emblem_len, fp);
			fclose(fp);
			ShowStatus("Done reading '" CL_WHITE "%s" CL_RESET "' battleground emblem data file.\n", path);
		}
	}
	ShowStatus("Create '" CL_WHITE "%d" CL_RESET "' guild for Battleground.\n", j);
}

// Faz requisição de informações do Clã da Batalha.
void bg_guild_requestinfo(struct map_session_data *sd)
{
	nullpo_retv(sd);
	guild_send_dot_remove(sd);
	clif_name_area(&sd->bl);
	clif_name_self(&sd->bl);
	clif_bg_belonginfo(sd);
	clif_bg_basicinfo(sd);
	clif_guild_skillinfo(sd);
	clif_guild_memberlist(sd);
	clif_guild_emblem_area(&sd->bl);
}

// Computa o dano nas Batalhas Campais.
void bg_score_damage(struct block_list *src, struct block_list *dst, int damage)
{
	struct block_list *s_bl;
	struct map_session_data *sd;

	if( !src || !dst || src == dst || damage <= 0 || !map[src->m].flag.battleground )
		return;

	if( (s_bl = battle_get_master(src)) == NULL )
		s_bl = src;

	sd = BL_CAST(BL_PC, s_bl);
	switch( dst->type )
	{
		case BL_PC:
			{
				struct map_session_data *tsd = BL_CAST(BL_PC, dst);

				if( tsd == NULL )
					break;

				switch(src->type)
				{
					case BL_PC:
						if( sd != NULL && tsd->bg_id && sd->bg_id && tsd->bg_id != sd->bg_id ) {
							increment_limit(tsd->bg_score.player_damage_taken, damage, USHRT_MAX);
							increment_limit(sd->bg_score.player_damage_given, damage, USHRT_MAX);
						}
						break;
					case BL_MOB:
						{
							struct mob_data *md = BL_CAST(BL_MOB, dst);
							if( sd == NULL || md == NULL || !sd->bg_id )
								break;

							switch(md->mob_id)
							{
								case 1285:
								case 1286:
								case 1287:
								case 1949:
								case 1950:
									increment_limit(sd->bg_score.guardian_damage_taken, damage, USHRT_MAX);
									break;
								default:
									break;
							}
						}
						break;
					default:
						break;
				}
			}
			break;
		case BL_MOB:
			{
				struct mob_data *md = BL_CAST(BL_MOB, dst);
				if( sd == NULL || md == NULL || !sd->bg_id )
					break;
				
				switch(md->mob_id)
				{
					case MOBID_EMPERIUM:
						increment_limit(sd->bg_score.emperium_damage, damage, USHRT_MAX);
						break;
					case MOBID_GUARDIAN_STONE1:
					case MOBID_GUARDIAN_STONE2:
						increment_limit(sd->bg_score.runestone_damage, damage, USHRT_MAX);
						break;
					case 1285:
					case 1286:
					case 1287:
					case 1949:
					case 1950:
						increment_limit(sd->bg_score.guardian_damage_given, damage, USHRT_MAX);
						break;
					case 1906:
						increment_limit(sd->bg_score.barrier_damage, damage, USHRT_MAX);
						break;
					case 1909:
					case 1910:
						increment_limit(sd->bg_score.objective_damage, damage, USHRT_MAX);
						break;
					case 1911:
					case 1912:
					case 1913:
						increment_limit(sd->bg_score.flag_damage, damage, USHRT_MAX);
						break;
					case 1914:
					case 1915:
						increment_limit(sd->bg_score.cristal_damage, damage, USHRT_MAX);
						break;
					default:
						break;
				}
			}
			break;
	}
}

// Computa eleminações nas Batalhas Campais.
void bg_score_kills(struct block_list *src, struct block_list *dst)
{
	struct block_list *s_bl;
	struct map_session_data *sd;

	if( !src || !dst || src == dst || !map[src->m].flag.battleground )
		return;

	if( (s_bl = battle_get_master(src)) == NULL )
		s_bl = src;

	sd = BL_CAST(BL_PC, s_bl);
	switch( dst->type )
	{
		case BL_PC:
			{
				struct map_session_data *tsd = BL_CAST(BL_PC, dst);
				if( s_bl->type == BL_MOB )
				{
					if( tsd != NULL && tsd->bg_id )
						increment_limit(tsd->bg_score.guardian_deaths, 1, USHRT_MAX);
				}
				else {
					if( tsd != NULL && sd != NULL && tsd->bg_id && sd->bg_id && tsd->bg_id != sd->bg_id ) {
						increment_limit(tsd->bg_score.player_deaths, 1, USHRT_MAX);
						increment_limit(sd->bg_score.player_kills, 1, USHRT_MAX);
					}
				}
			}
			break;
		case BL_MOB:
			{
				struct mob_data *md = BL_CAST(BL_MOB, dst);

				if( md == NULL || sd == NULL || !sd->bg_id )
					break;
				
				switch(md->mob_id)
				{
					case MOBID_EMPERIUM:
						increment_limit(sd->bg_score.emperium_kills, 1, USHRT_MAX);
						break;
					case MOBID_GUARDIAN_STONE1:
					case MOBID_GUARDIAN_STONE2:
						increment_limit(sd->bg_score.runestone_kills, 1, USHRT_MAX);
						break;
					case 1285:
					case 1286:
					case 1287:
					case 1949:
					case 1950:
						increment_limit(sd->bg_score.guardian_kills, 1, USHRT_MAX);
						break;
					case 1906:
						increment_limit(sd->bg_score.barrier_kills, 1, USHRT_MAX);
						break;
					case 1909:
					case 1910:
						increment_limit(sd->bg_score.objective_kills, 1, USHRT_MAX);
						break;
					case 1911:
					case 1912:
					case 1913:
						increment_limit(sd->bg_score.flag_kills, 1, USHRT_MAX);
						break;
					case 1914:
					case 1915:
						increment_limit(sd->bg_score.cristal_kills, 1, USHRT_MAX);
						break;
					default:
						break;
				}
			}
			break;
		default:
			break;
	}
}

// Computa as curas nas Batalhas Campais.
void bg_score_heal(struct block_list *bl, int hp, int sp)
{
	struct block_list *s_bl = battle_get_master(bl);
	struct map_session_data *sd;

	if( s_bl == NULL )
		s_bl = bl;

	sd = BL_CAST(BL_PC, s_bl);

	if( sd == NULL || !sd->bg_id || !map[sd->bl.m].flag.battleground )
		return;

	if( hp > 0 )
		increment_limit(sd->bg_score.heal_hp, hp, USHRT_MAX);
	if( sp > 0 )
		increment_limit(sd->bg_score.heal_sp, sp, USHRT_MAX);
	return;
}

// Computa utilização de itens de cura nas Batalhas Campais.
void bg_score_item_heal(struct map_session_data *sd, int amount, int hp, int sp)
{
	if( sd == NULL || !sd->bg_id || !map[sd->bl.m].flag.battleground )
		return;

	if( amount > 0 )
		increment_limit(sd->bg_score.item_heal, amount, USHRT_MAX);
	if( hp > 0 )
		increment_limit(sd->bg_score.item_heal_hp, hp, USHRT_MAX);
	if( sp > 0 )
		increment_limit(sd->bg_score.item_heal_sp, sp, USHRT_MAX);
	return;
}

// Computa utilização de itens de requerimentos nas Batalhas Campais.
// Itens utilizados em NPC's e Habilidades.
void bg_score_del_item(struct map_session_data *sd, int item_id, int amount)
{
	if( item_id <= 0 || amount <= 0 )
		return;

	switch(item_id)
	{
		// Poision Bottle (Garrafa de Veneno)
		case 678:
			increment_limit(sd->bg_score.poison_bottle, amount, USHRT_MAX);
			break;
		// Yellow Gemstone (Gema Amarela)
		case 715:
			increment_limit(sd->bg_score.yellow_gemstone, amount, USHRT_MAX);
			break;
		// Red Gemstone (Gema Vermelha)
		case 716:
			increment_limit(sd->bg_score.red_gemstone, amount, USHRT_MAX);
			break;
		// Blue Gemstone (Gema Azul)
		case 717:
			increment_limit(sd->bg_score.blue_gemstone, amount, USHRT_MAX);
			break;
		// Oridecon (Oridecon)
		case 984:
			increment_limit(sd->bg_score.oridecon, amount, USHRT_MAX);
			break;
		// Elunium (Elunium)
		case 985:
			increment_limit(sd->bg_score.elunium, amount, USHRT_MAX);
			break;
		// Steel (Aço)
		case 999:
			increment_limit(sd->bg_score.steel, amount, USHRT_MAX);
			break;
		// Emveretarcon (Emveretarcon)
		case 1011:
			increment_limit(sd->bg_score.emveretarcon, amount, USHRT_MAX);
			break;
		// Wooden Block (Tronco)
		case 1019:
			increment_limit(sd->bg_score.wooden_block, amount, USHRT_MAX);
			break;
		// Stone (Pedra)
		case 7049:
			increment_limit(sd->bg_score.stone, amount, USHRT_MAX);
			break;
		// Fire Bottle (Frasco de Fogo Grego)
		case 7135:
			increment_limit(sd->bg_score.fire_bottle, amount, USHRT_MAX);
			break;
		// Acid Bottle (Frasco de Ácido)
		case 7136:
			increment_limit(sd->bg_score.acid_bottle, amount, USHRT_MAX);
			break;
		default:
			break;
	}
	return;
}

static int bg_rate_adjust(int baserate, int rate_adjust, unsigned short rate_min, unsigned short rate_max)
{
	double rate = baserate;

	if (battle_config.logarithmic_drops && rate_adjust > 0 && rate_adjust != 100 && baserate > 0) //Logarithmic drops equation by Ishizu-Chan
		//Equation: Droprate(x,y) = x * (5 - log(x)) ^ (ln(y) / ln(5))
		//x is the normal Droprate, y is the Modificator.
		rate = rate * pow((5.0 - log10(rate)), (log(rate_adjust/100.) / log(5.0))) + 0.5;
	else
		//Classical linear rate adjustment.
		rate = rate * rate_adjust/100;

	return (unsigned int)cap_value(rate,rate_min,rate_max);
}

struct battleground_rewards* bg_search_reward(int arena_id)
{ // Search Rewards by Arena ID
	if( !arena_id ) return NULL;
	return (struct battleground_rewards *)idb_get(bg_reward_db, arena_id);
}

struct battleground_rewards* bg_create_reward(int arena_id)
{
	struct battleground_rewards *bgr;
	CREATE(bgr, struct battleground_rewards, 1);
	bgr->arena_id = arena_id;
	bgr->count = 0;
	memset(&bgr->items, 0, sizeof(bgr->items));
	idb_put(bg_reward_db, arena_id, bgr);
	return bgr;
}

void bg_reload_rewards()
{
	bg_reward_db->clear(bg_reward_db, NULL);
	bg_load_rewards_from_sql();
}

int bg_load_rewards_from_sql()
{
	SqlStmt* stmt = SqlStmt_Malloc(mmysql_handle);
	StringBuf buff;
	struct battleground_rewards *bgr;
	struct battleground_rewards_objects tmp_reward;
	int i, arena_id, count = 0;

	StringBuf_Init(&buff);
	StringBuf_AppendStr(&buff, "SELECT `bg_id`, `object`, `value`, `type`, `for`, `rate`, `desc` FROM `bg_rewards` WHERE `status`='1'");

	if( SQL_ERROR == SqlStmt_PrepareStr(stmt, StringBuf_Value(&buff))
		|| SQL_ERROR == SqlStmt_Execute(stmt)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_INT, &arena_id, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 1, SQLDT_STRING, &tmp_reward.object, sizeof(tmp_reward.object), NULL, NULL) 
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 2, SQLDT_INT, &tmp_reward.value, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 3, SQLDT_INT, &tmp_reward.object_type, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 4, SQLDT_INT, &tmp_reward.flag_type, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 5, SQLDT_INT, &tmp_reward.rate, 0, NULL, NULL)
		|| SQL_ERROR == SqlStmt_BindColumn(stmt, 6, SQLDT_STRING, &tmp_reward.desc, sizeof(tmp_reward.desc), NULL, NULL)
	) {
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		return 0;
	}

	if( SqlStmt_NumRows(stmt) <= 0 )
	{
		SqlStmt_FreeResult(stmt);
		return 0;
	}

	while( SQL_SUCCESS == SqlStmt_NextRow(stmt) ) {
		if( (bgr = bg_search_reward(arena_id)) == NULL )
			bgr = bg_create_reward(arena_id);

		i = bgr->count;
		if( i >= MAX_BG_REWARDS )
			continue;

		memcpy(&bgr->items[i], &tmp_reward, sizeof(tmp_reward));
		bgr->count++;
		count++;
	}

	StringBuf_Destroy(&buff);
	SqlStmt_FreeResult(stmt);
	ShowStatus("Load '" CL_WHITE "%d" CL_RESET "' rewards for Battleground.\n", count);
	return count;
}

int bg_reward(struct map_session_data *sd, int arena_id, enum bg_reward_type flag) {
	struct battleground_rewards *bgr;
	int i, rate, rate_adjust, rate_min, rate_max, count = 0;
	char output[200];
	
	if( sd == NULL || (bgr = bg_search_reward(arena_id)) == NULL || bgr->count <= 0 )
		return 0;
	
	rate_adjust = battle_config.bg_reward_rate;
	rate_min = battle_config.bg_reward_rate_min;
	rate_max = battle_config.bg_reward_rate_max;

	for( i = 0; i < MAX_BG_REWARDS; i++ )
	{
		if( strlen(bgr->items[i].object) <= 0 )
			continue;

		if( bgr->items[i].flag_type != flag )
			continue;
			
		if( battle_config.bg_reward_max_given && count >= battle_config.bg_reward_max_given )
			break;

		rate = bg_rate_adjust(bgr->items[i].rate, rate_adjust, rate_min, rate_max);
		if( rate <= rand()%10000+1 )
			continue;

		// Selecte Reward type
		switch(bgr->items[i].object_type)
		{
			// Item Reward
			case BGRI_ITEM:
				{
					struct item it;
					struct item_data *i_data = NULL;
					int nameid = atoi(bgr->items[i].object), amount = bgr->items[i].value, n, get_count, weight = 0, i_flag = 0, a_flag;

					if( !nameid || !amount )
						break;

					if( !(i_data = itemdb_exists(nameid)) )
						break;

					memset(&it,0,sizeof(it));
					it.nameid = nameid;
					it.identify = 1;
					it.bound = BOUND_NONE;

					get_count = itemdb_isstackable2(i_data)?amount:1;
					weight += itemdb_weight(nameid)*amount;

					if( (weight+sd->weight) < sd->max_weight )
					{
						for (n = 0; n < amount; n += get_count) {
							if (!pet_create_egg(sd, nameid)) {
								if( (a_flag = pc_additem(sd, &it, get_count, LOG_TYPE_OTHER)) )
									clif_additem(sd, 0, 0, flag);
								else
									i_flag = 1;
							}
						}
					}
					else if( sd->storage.max_amount < MAX_STORAGE && itemdb_canstore(&it,pc_get_group_level(sd)) ) {
						for (n = 0 ; i < amount; n += get_count)
						{
							if( storage_additem(sd, &sd->storage, &it, get_count) == 0 )
								i_flag = 2;
						}

						storage_storageclose(sd);
					}

					switch(i_flag)
					{
						case 1:
							count++;
							sprintf(output, msg_txt(sd,(BG_MSG_TXT+13)), amount, i_data->jname);
							clif_messagecolor(&sd->bl, BG_COLOR, output, true, SELF);
							break;
						case 2:
							count++;
							sprintf(output, msg_txt(sd,(BG_MSG_TXT+14)), amount, i_data->jname);
							clif_messagecolor(&sd->bl, BG_COLOR, output, true, SELF);
							break;
						default:
							break;
					}
				}
				break;
			// Zeny Reward
			case BGRI_ZENY:
				{
					int amount = bgr->items[i].value;

					if( amount > MAX_ZENY-sd->status.zeny )
						amount = MAX_ZENY - sd->status.zeny;

					if( amount > 0 ) {
						count++;
						pc_getzeny(sd, amount, LOG_TYPE_OTHER, NULL);
						sprintf(output, msg_txt(sd,(BG_MSG_TXT+15)), amount, bgr->items[i].desc);
						clif_messagecolor(&sd->bl, BG_COLOR, output, true, SELF);
					}
				}
				break;
			// Cash Point
			case BGRI_CASH:
				{
					int amount = bgr->items[i].value;
				
					if( amount > MAX_ZENY-sd->cashPoints )
						amount = MAX_ZENY - sd->cashPoints; 
					
					if( amount > 0 ) {
						count++;
						pc_setaccountreg(sd, add_str("#CASHPOINTS"), sd->cashPoints+amount);
						sprintf(output, msg_txt(sd,(BG_MSG_TXT+15)), amount, bgr->items[i].desc);
						clif_messagecolor(&sd->bl, BG_COLOR, output, true, SELF);
					}
				}
				break;
			// Kafra Point
			case BGRI_KAFRA:
				{
					int amount = bgr->items[i].value;
			
					if( amount > MAX_ZENY-sd->kafraPoints )
						amount = MAX_ZENY - sd->kafraPoints; 

					if( amount > 0 ) {
						count++;
						pc_setaccountreg(sd, add_str("#KAFRAPOINTS"), sd->kafraPoints+amount);
						sprintf(output, msg_txt(sd,(BG_MSG_TXT+15)), amount, bgr->items[i].desc);
						clif_messagecolor(&sd->bl, BG_COLOR, output, true, SELF);
					}
				}
				break;
			// Base EXP Reward
			case BGRI_BEXP:
				{
					int amount = bgr->items[i].value;
					
					count++;
					pc_gainexp(sd,NULL,amount,0,true);
					sprintf(output, msg_txt(sd,(BG_MSG_TXT+15)), amount, bgr->items[i].desc);
					clif_messagecolor(&sd->bl, BG_COLOR, output, true, SELF);
				}
				break;
			// Job EXP Reward
			case BGRI_JEXP:
				{
					int amount = bgr->items[i].value;
			
					count++;
					pc_gainexp(sd,NULL,0,amount,true);
					sprintf(output, msg_txt(sd,(BG_MSG_TXT+15)), amount, bgr->items[i].desc);
					clif_messagecolor(&sd->bl, BG_COLOR, output, true, SELF);
				}
				break;
			// Base Level Reward
			case BGRI_BLVL:
				{
					int status_point = 0, level = bgr->items[i].value, n;

					if( sd->status.base_level >= pc_maxbaselv(sd) )
						break;

					if( (unsigned int)level > pc_maxbaselv(sd) || (unsigned int)level > (pc_maxbaselv(sd)-sd->status.base_level) )
						level = pc_maxbaselv(sd) - sd->status.base_level;

					for( n = 0; n < level; n++ )
						status_point += pc_gets_status_point(sd->status.base_level + n);
				
					sd->status.status_point += status_point;
					sd->status.base_level += (unsigned int)level;
					status_percent_heal(&sd->bl, 100, 100);
					clif_misceffect(&sd->bl, 0);

					clif_updatestatus(sd, SP_STATUSPOINT);
					clif_updatestatus(sd, SP_BASELEVEL);
					clif_updatestatus(sd, SP_BASEEXP);
					clif_updatestatus(sd, SP_NEXTBASEEXP);
					pc_baselevelchanged(sd);
					if(sd->status.party_id)
						party_send_levelup(sd);

					count++;
					sprintf(output, msg_txt(sd,(BG_MSG_TXT+16)), level, bgr->items[i].desc);
					clif_messagecolor(&sd->bl, BG_COLOR, output, true, SELF);
				}
				break;
			// Job Level Reward
			case BGRI_JLVL:
				{
					int level = bgr->items[i].value;

					if( sd->status.base_level >= pc_maxbaselv(sd) )
						break;

					if( (unsigned int)level > pc_maxjoblv(sd) || (unsigned int)level > (pc_maxjoblv(sd)-sd->status.job_level) )
						level = pc_maxjoblv(sd) - sd->status.job_level;

					sd->status.skill_point += (unsigned int)level;
					sd->status.job_level += (unsigned int)level;
					clif_misceffect(&sd->bl, 1);

					clif_updatestatus(sd, SP_JOBLEVEL);
					clif_updatestatus(sd, SP_JOBEXP);
					clif_updatestatus(sd, SP_NEXTJOBEXP);
					clif_updatestatus(sd, SP_SKILLPOINT);
					status_calc_pc(sd, SCO_FORCE);

					count++;
					sprintf(output, msg_txt(sd,(BG_MSG_TXT+16)), level, bgr->items[i].desc);
					clif_messagecolor(&sd->bl, BG_COLOR, output, true, SELF);
				}
				break;
			// Status
			case BGRI_BONUS_STATUS:
				{
					unsigned short *status[6], max_status[6];
					int s = atoi(bgr->items[i].object), amount = bgr->items[i].value, new_value;

					if( i < 0 || i > MAX_STATUS_TYPE )
						break;

					status[0] = &sd->status.str;
					status[1] = &sd->status.agi;
					status[2] = &sd->status.vit;
					status[3] = &sd->status.int_;
					status[4] = &sd->status.dex;
					status[5] = &sd->status.luk;

					if( pc_has_permission(sd, PC_PERM_BYPASS_MAX_STAT) )
						max_status[0] = max_status[1] = max_status[2] = max_status[3] = max_status[4] = max_status[5] = SHRT_MAX;

					if( amount > 0 && *status[s] + amount >= max_status[s] )
						new_value = max_status[s];
					else if( amount < 0 && *status[s] <= -amount )
						new_value = 1;
					else
						new_value = *status[s] + amount;

					if( new_value != *status[s] )
					{
						*status[s] = new_value;
						clif_updatestatus(sd, SP_STR+s);
						clif_updatestatus(sd, SP_USTR+s);
						status_calc_pc(sd, SCO_FORCE);
						count++;
						sprintf(output, msg_txt(sd,(BG_MSG_TXT+17)), new_value, bgr->items[i].desc);
						clif_messagecolor(&sd->bl, BG_COLOR, output, true, SELF);
					}
				}
				break;
			// Character Variable
			case BGRI_CHAR_VAR:
				{
					int value = pc_readglobalreg(sd, add_str(bgr->items[i].object));

					count++;
					pc_setglobalreg(sd, add_str(bgr->items[i].object), (value+bgr->items[i].value));
					sprintf(output, msg_txt(sd,(BG_MSG_TXT+16)), bgr->items[i].value, bgr->items[i].object);
					clif_messagecolor(&sd->bl, BG_COLOR, output, true, SELF);
				}
				break;
			// Character Variable
			case BGRI_ACC_VAR:
				{
					int value = pc_readaccountreg(sd, add_str(bgr->items[i].object));

					count++;
					pc_setaccountreg(sd, add_str(bgr->items[i].object), (value+bgr->items[i].value));
					sprintf(output, msg_txt(sd,(BG_MSG_TXT+16)), bgr->items[i].value, bgr->items[i].object);
					clif_messagecolor(&sd->bl, BG_COLOR, output, true, SELF);
				}
				break;
			// Unknown type!
			default:
				ShowDebug("bg_reward: Type %d is unknown in Object %s from (arena_id: %d). (CID/AID: %d, %d)\n", bgr->items[i].flag_type, bgr->items[i].object, arena_id, sd->status.char_id, sd->status.account_id);
				return 0;
		}
	}
	
	if( count <= 0 ) {
		sprintf(output, msg_txt(sd,(BG_MSG_TXT+18)));
		clif_messagecolor(&sd->bl, BG_COLOR, output, true, SELF);
	}
	
	return count;
}

int bg_flag_alarm_npc(int tid, unsigned int tick, int id, intptr_t data)
{
	struct npc_data* nd=(struct npc_data *)map_id2bl(id);
	struct battleground_data *bg;

	if( nd && nd->battleground.bg_id && (bg = bg_team_search(nd->battleground.bg_id)) != NULL )
	{
		struct map_session_data *sd;
		struct s_mapiterator* iter = mapit_getallusers();
		int flag = 2;

		if( nd->bl.x != nd->battleground.xs || nd->bl.y != nd->battleground.ys )
			flag = 1;

		for( sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC*)mapit_next(iter) )
		{
			if( sd->bl.m != nd->bl.m )
				continue;

			if( sd->bg_flags.nd == nd )
				continue;

			clif_viewpoint(sd,nd->battleground.npc_id,flag,nd->bl.x,nd->bl.y,nd->battleground.n,bg->army?0xFF0000:0x0000FF);
		}
		nd->battleground.target_timer = add_timer(gettick()+5000, bg_flag_alarm_npc, nd->bl.id, 0);
		return 1;
	}
	return 0;
}

int bg_flag_alarm_player(int tid, unsigned int tick, int id, intptr_t data)
{
	struct map_session_data *sd = map_id2sd(id);
	struct npc_data *nd = NULL;
	int flag = 0;

	if( sd && sd->bg_flags.nd && sd->bg_flags.target_timer != INVALID_TIMER && (nd = sd->bg_flags.nd) != NULL )
	{
		if( nd->battleground.bg_id == sd->bg_id )
			flag = 1;

		sd->bg_flags.nd->bl.x = sd->bl.x;
		sd->bg_flags.nd->bl.y = sd->bl.y;
		clif_specialeffect(&sd->bl, 60, AREA);
		clif_viewpoint(sd,nd->battleground.npc_id,1,flag?nd->battleground.x:nd->battleground.xs,flag?nd->battleground.y:nd->battleground.ys,nd->battleground.n,0xFFFF00);
		sd->bg_flags.target_timer = add_timer(gettick()+5000, bg_flag_alarm_player, sd->bl.id, 0);
		return 1;
	}
	return 0;
}

int bg_set_npc(int bg_id, struct npc_data *nd, int npc_id, short x, short y, const char *ev, const char *rev)
{
	struct battleground_data *bg = bg_team_search(bg_id);
	int flag = 0;

	if( bg == NULL || nd == NULL )
		return 0;

	if( nd->battleground.bg_id > 0 )
		flag = 1;

	nd->battleground.npc_id = npc_id;
	nd->battleground.bg_id = bg_id;
	nd->battleground.x = nd->bl.x;
	nd->battleground.y = nd->bl.y;
	nd->battleground.xs = x;
	nd->battleground.ys = y;
	nd->battleground.dir = nd->ud.dir;
	nd->battleground.status = 0;
	safestrncpy(nd->battleground.capture_event, ev, sizeof(nd->battleground.capture_event));
	safestrncpy(nd->battleground.recapture_event, rev, sizeof(nd->battleground.recapture_event));

	// Set Touch events.
	if( x || y )
	{
		if( flag <= 0 ) {
			map[nd->bl.m].bg_flags_touch++;
			nd->battleground.n = map[nd->bl.m].bg_flags_touch;
		}
		nd->battleground.target_timer = add_timer(gettick() + 1, bg_flag_alarm_npc, nd->bl.id, 0);
	}
	return nd->bl.id;
}

int bg_flag_drop(struct map_session_data *sd)
{
	struct npc_data *nd;
	nullpo_retr(1, sd);

	if( sd->bg_flags.nd == NULL )
		return 0;

	// Move Npc
	nd = sd->bg_flags.nd;
	nd->bl.x = sd->bl.x;
	nd->bl.y = sd->bl.y;
	nd->ud.dir = sd->ud.dir;
	nd->battleground.status = 0;
	npc_enable(nd->exname, 1);
	npc_movenpc(nd,nd->bl.x,nd->bl.y);

	// Clear Player Events
	sd->bg_flags.nd = NULL;
	sd->bg_flags.target_timer = INVALID_TIMER;
	clif_viewpoint(sd,nd->battleground.npc_id,2,nd->battleground.xs,nd->battleground.ys,nd->battleground.n,0xFFFF00);
	return 1;
}

int bg_flag_capture_area(struct map_session_data* sd, int m, int x, int y)
{
	struct battleground_data *bg;
	int i;
	int f = 1;
	int xs = battle_config.bg_capture_flag_path,ys = battle_config.bg_capture_flag_path;
	char output[200];
	
	nullpo_retr(1, sd);

	if( sd->bg_flags.nd != NULL || pc_isdead(sd) || sd->state.warping )
		return 0;

	for( i=0; i<map[m].npc_num; i++ )
	{
		if( map[m].npc[i]->sc.option&OPTION_INVISIBLE ||
			!map[m].npc[i]->battleground.bg_id ||
			!map[m].npc[i]->battleground.xs ||
			!map[m].npc[i]->battleground.ys ||
			map[m].npc[i]->battleground.status ||
			map[m].npc[i]->subtype != NPCTYPE_SCRIPT
		) {
			f=0; // a npc was found, but it is disabled function, don't print warning
			continue;
		}

		if( x >= map[m].npc[i]->bl.x-xs && x <= map[m].npc[i]->bl.x+xs
		&&  y >= map[m].npc[i]->bl.y-ys && y <= map[m].npc[i]->bl.y+ys )
			break;
	}

	if( i == map[m].npc_num )
	{
		if( f == 1 ) // no npc found
			ShowError("bg_check_flag_areanpc : stray NPC cell on coordinates '%s',%d,%d\n", map[m].name, x, y);
		return 0;
	}
	
	if( map[m].npc[i]->subtype == NPCTYPE_SCRIPT )
	{
		// members do not capture the flag at the base.
		if( map[m].npc[i]->battleground.bg_id == sd->bg_id
			&& map[m].npc[i]->bl.x == map[m].npc[i]->battleground.x
			&& map[m].npc[i]->bl.y == map[m].npc[i]->battleground.y )
			return 0;

		// no battleground found
		if( (bg = bg_team_search(map[m].npc[i]->battleground.bg_id)) == NULL )
			return 0;

		// no battleground map
		if( sd->bl.m != map[m].npc[i]->bl.m )
			return 0;

		sd->bg_flags.nd = map[m].npc[i];
		map[m].npc[i]->battleground.status = 1;
		npc_enable(map[m].npc[i]->exname, 0);
		sd->bg_flags.target_timer = add_timer(gettick() + 1, bg_flag_alarm_player, sd->bl.id, 0);
		clif_viewpoint(sd,map[m].npc[i]->battleground.npc_id,1,map[m].npc[i]->battleground.xs,map[m].npc[i]->battleground.ys,map[m].npc[i]->battleground.n,0xFFFF00);

		// output messages
		sprintf(output, msg_txt(sd,(BG_MSG_TXT+19)), map[m].npc[i]->name);
		clif_messagecolor(&sd->bl, 0x00BFFF, output, true, SELF);
		//sprintf(output, msg_txt(sd,(BG_MSG_TXT+20)), sd->status.name, map[m].npc[i]->name);
		return 1;
	}
	return 0;
}

int bg_flag_catch_area(struct map_session_data* sd, int m, int x, int y)
{
	struct npc_data *nd;
	struct battleground_data *bg;
	char output[200];
	int xs = battle_config.bg_capture_flag_path,ys = battle_config.bg_capture_flag_path;

	nullpo_retr(1, sd);

	if( sd->bg_id <= 0 || pc_isdead(sd) || sd->bg_flags.nd == NULL || sd->state.warping  )
		return 0;
	
	if((nd = sd->bg_flags.nd) == NULL || nd->battleground.bg_id <= 0 || (bg = bg_team_search(nd->battleground.bg_id)) == NULL )
		return 0;

	if( nd->battleground.xs <= 0 && nd->battleground.ys <= 0 )
		return 0;

	// no battleground map
	if( sd->bl.m != nd->bl.m )
		return 0;

	if( sd->bg_id != nd->battleground.bg_id )
	{
		if( x >= nd->battleground.xs-xs && x <= nd->battleground.xs+xs
			&& y >= nd->battleground.ys-ys && y <= nd->battleground.ys+ys )
		{
			nd->bl.x = nd->battleground.xs;
			nd->bl.y = nd->battleground.ys;
			sd->bg_flags.nd = NULL;
			sd->bg_flags.target_timer = INVALID_TIMER;
			increment_limit(sd->bg_score.flag_capture, 1, USHRT_MAX);
			
			// output messages
			sprintf(output, msg_txt(sd,(BG_MSG_TXT+21)), nd->name);
			clif_messagecolor(&sd->bl, 0x00BFFF, output, true, SELF);

			if( nd->battleground.capture_event[0] ) {
				set_var(sd, (char *)"@flag_name$", nd->exname);
				npc_event(sd, nd->battleground.capture_event, 0);
			}
			return 1;
		}
	}
	else {
		if( x >= nd->battleground.x-xs && x <= nd->battleground.x+ys
			&& y >= nd->battleground.y-ys && y <= nd->battleground.y+ys)
		{
			// Move Npc
			nd->bl.x = nd->battleground.x;
			nd->bl.y = nd->battleground.y;
			nd->ud.dir = nd->battleground.dir;
			nd->battleground.status = 0;
			npc_enable(nd->exname, 1);
			npc_movenpc(nd,nd->bl.x,nd->bl.y);

			// Clear Player Event
			sd->bg_flags.nd = NULL;
			sd->bg_flags.target_timer = INVALID_TIMER;
			increment_limit(sd->bg_score.flag_recapture, 1, USHRT_MAX);
			clif_viewpoint(sd,nd->battleground.npc_id,2,nd->battleground.x,nd->battleground.y,nd->battleground.n,0xFFFF00);

			// output messages
			sprintf(output, msg_txt(sd,(BG_MSG_TXT+22)), nd->name);
			clif_messagecolor(&sd->bl, 0x00BFFF, output, true, SELF);

			if( nd->battleground.recapture_event[0] ) {
				set_var(sd, (char *)"@flag_name$", nd->exname);
				npc_event(sd, nd->battleground.recapture_event, 0);
			}
			return 1;
		}
	}
	return 0;
}

int bg_flag_respawn(int m, int bg1_id, int bg2_id)
{
	struct battleground_data *bg;
	struct npc_data *nd;
	int i;

	// Remove Liked Players
	if( bg1_id && (bg = bg_team_search(bg1_id)) != NULL )
	{
		struct map_session_data *sd = NULL;
		for( i=0; i < MAX_BG_MEMBERS; i++ )
		{
			if( (sd = bg->members[i].sd) == NULL )
				continue;

			if( sd->bg_flags.nd != NULL )
			{
				// Clear Player Events
				nd = sd->bg_flags.nd;
				sd->bg_flags.nd = NULL;
				sd->bg_flags.target_timer = INVALID_TIMER;
				clif_viewpoint(sd,nd->battleground.npc_id,2,nd->battleground.xs,nd->battleground.ys,nd->battleground.n,0xFFFF00);
			}
		}
	}

	// Remove Linked Players
	if( bg2_id && (bg = bg_team_search(bg2_id)) != NULL )
	{
		struct map_session_data *sd = NULL;
		for( i=0; i < MAX_BG_MEMBERS; i++ )
		{
			if( (sd = bg->members[i].sd) == NULL )
				continue;

			if( sd->bg_flags.nd != NULL )
			{
				// Clear Player Events
				nd = sd->bg_flags.nd;
				sd->bg_flags.nd = NULL;
				sd->bg_flags.target_timer = INVALID_TIMER;
				clif_viewpoint(sd,nd->battleground.npc_id,2,nd->battleground.xs,nd->battleground.ys,nd->battleground.n,0xFFFF00);
			}
		}
	}

	for( i=0; i<map[m].npc_num; i++ )
	{
		nd = map[m].npc[i];
		if( !nd->battleground.bg_id || !nd->battleground.xs || !nd->battleground.ys )
			continue;

		// Disable to move...
		if( nd->sc.option&OPTION_INVISIBLE )
			npc_enable(nd->exname, 0);

		nd->battleground.status = 1; // prev capture
		nd->bl.x = nd->battleground.x;
		nd->bl.y = nd->battleground.y;
		nd->ud.dir = nd->battleground.dir;
		npc_movenpc(nd,nd->bl.x,nd->bl.y);
		npc_enable(nd->exname, 1);
		nd->battleground.status = 0; // status dropped
		
	}
	return 1;	
}

int bg_guild_refresh_all(struct battleground_data *bg)
{
	int i;

	if( bg == NULL )
		return 0;

	for( i=0; i < MAX_BG_MEMBERS; i++ )
	{
		struct map_session_data* sd = bg->members[i].sd;
		if( sd == NULL )
			continue;
		
		clif_bg_basicinfo(sd);
		clif_guild_emblem(sd,bg->g);
		clif_bg_memberlist(sd);
	}
	bg_refresh_patent(bg->bg_id);
	return 0;
}

void do_bg_reward_reload(void)
{
	bg_reward_db->clear(bg_reward_db, NULL);
	bg_load_rewards_from_sql();
}
