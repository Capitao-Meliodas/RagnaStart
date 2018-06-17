// © Creative Services and Development
// Site Oficial: www.creativesd.com.br
// Termos de Contrato e Autoria em: http://creativesd.com.br/?p=termos

#include "ranked.hpp"

#ifdef RANKED_SYSTEM_ENABLE

#include "../common/cbasetypes.h"
#include "../common/timer.h"
#include "../common/malloc.h"
#include "../common/nullpo.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/utils.h"

#include "atcommand.hpp" // msg_txt
#include "clif.hpp"
#include "map.hpp"
#include "npc.hpp"
#include "pc.hpp"
#include "homunculus.hpp"
#include "mercenary.hpp"
#include "pet.hpp"
#include "guild.hpp"
#include "battleground.hpp"
#include "elemental.hpp"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static DBMap* ranked_db = NULL;
static unsigned int ranked_counter = 0;
static float ranked_last_points = 0;
struct Ranked_Config ranked_config;

struct guild ranked_guild[MAX_RANKED_DB];

struct ranked_data* ranked_search(int rank_id)
{
	if( !rank_id ) return NULL;
	return (struct ranked_data *)idb_get(ranked_db, rank_id);
}

float ranked_get_last_points()
{
	return ranked_last_points;
}

int ranked_getbl_rank(struct block_list *bl)
{
	nullpo_ret(bl);
	switch( bl->type )
	{
		case BL_PC:
			return ((TBL_PC*)bl)->ranked.current_rank;
		case BL_PET:
			if( ((TBL_PET*)bl)->master )
				return ((TBL_PET*)bl)->master->ranked.current_rank;
			break;
		case BL_MOB:
		{
			struct map_session_data *msd;
			struct mob_data *md = (TBL_MOB*)bl;
			if( md->special_state.ai && (msd = map_id2sd(md->master_id)) != NULL )
				return msd->ranked.current_rank;
			break;
		}
		case BL_HOM:
			if( ((TBL_HOM*)bl)->master )
				return ((TBL_HOM*)bl)->master->ranked.current_rank;
			break;
		case BL_MER:
			if( ((TBL_MER*)bl)->master )
				return ((TBL_MER*)bl)->master->ranked.current_rank;
			break;
		case BL_ELEM:
			if (((TBL_ELEM*)bl)->master)
				return ((TBL_ELEM*)bl)->master->ranked.current_rank;
			break;
	}

	return 0;
}

float ranked_getbl_points(struct block_list *bl, int flag)
{
	nullpo_ret(bl);
	switch( bl->type )
	{
		case BL_PC:
			return flag ? ((TBL_PC*)bl)->ranked.account_points : ((TBL_PC*)bl)->status.ranked_points;
		case BL_PET:
			if( ((TBL_PET*)bl)->master )
				return flag ? ((TBL_PET*)bl)->master->ranked.account_points : ((TBL_PET*)bl)->master->status.ranked_points;
			break;
		case BL_MOB:
		{
			struct map_session_data *msd;
			struct mob_data *md = (TBL_MOB*)bl;
			if( md->special_state.ai && (msd = map_id2sd(md->master_id)) != NULL )
				return flag ? msd->ranked.account_points : msd->status.ranked_points;
			break;
		}
		case BL_HOM:
			if( ((TBL_HOM*)bl)->master )
				return flag ? ((TBL_HOM*)bl)->master->ranked.account_points : ((TBL_HOM*)bl)->master->status.ranked_points;
			break;
		case BL_MER:
			if( ((TBL_MER*)bl)->master )
				return flag ? ((TBL_MER*)bl)->master->ranked.account_points : ((TBL_MER*)bl)->master->status.ranked_points;
			break;
		case BL_ELEM:
			if (((TBL_ELEM*)bl)->master)
				return flag ? ((TBL_ELEM*)bl)->master->ranked.account_points : ((TBL_ELEM*)bl)->master->status.ranked_points;
			break;
	}

	return 0;
}

int ranked_getbl_status(struct block_list *bl)
{
	nullpo_ret(bl);
	switch( bl->type )
	{
		case BL_PC:
			return ((TBL_PC*)bl)->ranked.display_pos;
		case BL_PET:
			if( ((TBL_PET*)bl)->master )
				return ((TBL_PET*)bl)->master->ranked.display_pos;
			break;
		case BL_MOB:
		{
			struct map_session_data *msd;
			struct mob_data *md = (TBL_MOB*)bl;
			if( md->special_state.ai && (msd = map_id2sd(md->master_id)) != NULL )
				return msd->ranked.display_pos;
			break;
		}
		case BL_HOM:
			if( ((TBL_HOM*)bl)->master )
				return ((TBL_HOM*)bl)->master->ranked.display_pos;
			break;
		case BL_MER:
			if( ((TBL_MER*)bl)->master )
				return ((TBL_MER*)bl)->master->ranked.display_pos;
			break;
		case BL_ELEM:
			if (((TBL_ELEM*)bl)->master)
				return ((TBL_ELEM*)bl)->master->ranked.display_pos;
			break;
	}

	return -1;
}

int ranked_getbl_disable(struct block_list *bl)
{
	nullpo_ret(bl);
	switch( bl->type )
	{
		case BL_PC:
			return ((TBL_PC*)bl)->ranked.disable;
		case BL_PET:
			if( ((TBL_PET*)bl)->master )
				return ((TBL_PET*)bl)->master->ranked.disable;
			break;
		case BL_MOB:
		{
			struct map_session_data *msd;
			struct mob_data *md = (TBL_MOB*)bl;
			if( md->special_state.ai && (msd = map_id2sd(md->master_id)) != NULL )
				return msd->ranked.disable;
			break;
		}
		case BL_HOM:
			if( ((TBL_HOM*)bl)->master )
				return ((TBL_HOM*)bl)->master->ranked.disable;
			break;
		case BL_MER:
			if( ((TBL_MER*)bl)->master )
				return ((TBL_MER*)bl)->master->ranked.disable;
			break;
		case BL_ELEM:
			if (((TBL_ELEM*)bl)->master)
				return ((TBL_ELEM*)bl)->master->ranked.disable;
			break;
	}

	return 0;
}

int ranked_get_rank(float points)
{
	DBIterator* iter = db_iterator(ranked_db);
	struct ranked_data* rank;
	int rank_id = 0;
	float p_points = 0;

	for( rank = (struct ranked_data*)dbi_first(iter); dbi_exists(iter); rank = (struct ranked_data*)dbi_next(iter) )
	{
		if( rank_id && p_points && rank->points < p_points )
		{
			rank_id = rank->rank_id;
			p_points = rank->points;
		}
		else if( points >= rank->points )
		{
			rank_id = rank->rank_id;
			p_points = rank->points;
		}
	}
	dbi_destroy(iter);
	return rank_id;
}

int ranked_get_next(float points)
{
	DBIterator* iter = db_iterator(ranked_db);
	struct ranked_data* rank;
	int rank_id = 0;
	float p_points = 0;

	for (rank = (struct ranked_data*)dbi_first(iter); dbi_exists(iter); rank = (struct ranked_data*)dbi_next(iter))
	{
		if( rank_id > 0 && rank->points > points && rank->points < p_points )
		{
			rank_id = rank->rank_id;
			p_points = rank->points;
		}
		else if( rank_id <= 0 && rank->points > points )
		{
			rank_id = rank->rank_id;
			p_points = rank->points;
		}
	}
	dbi_destroy(iter);
	return rank_id;
}

int ranked_get_guild_id(struct block_list *bl)
{
	nullpo_ret(bl);
	switch (bl->type) {
	case BL_PC:
		return ((TBL_PC*)bl)->status.guild_id;
	case BL_PET:
		if (((TBL_PET*)bl)->master)
			return ((TBL_PET*)bl)->master->status.guild_id;
		break;
	case BL_MOB:
	{
		struct map_session_data *msd;
		struct mob_data *md = (struct mob_data *)bl;
		if (md->guardian_data)	//Guardian's guild [Skotlex]
			return md->guardian_data->guild_id;
		if (md->special_state.ai && (msd = map_id2sd(md->master_id)) != NULL)
			return msd->status.guild_id; //Alchemist's mobs [Skotlex]
	}
		break;
	case BL_HOM:
	  	if (((TBL_HOM*)bl)->master)
			return ((TBL_HOM*)bl)->master->status.guild_id;
		break;
	case BL_MER:
		if (((TBL_MER*)bl)->master)
			return ((TBL_MER*)bl)->master->status.guild_id;
		break;
	case BL_NPC:
	  	if (((TBL_NPC*)bl)->subtype == NPCTYPE_SCRIPT)
			return ((TBL_NPC*)bl)->u.scr.guild_id;
		break;
	case BL_SKILL:
		return ((TBL_SKILL*)bl)->group->guild_id;
	default:
		break;
	}
	return 0;
}

static struct guild *ranked_create_guild(int rank_id, const char *name, const char *emblem)
{
	FILE* fp = NULL;
	char path[256];
	int i;

	i = ranked_counter-1;

	// Maximum ranked db not allowed
	if( i >= MAX_RANKED_DB )
		return NULL;

	ranked_guild[i].emblem_id = 1;
	ranked_guild[i].guild_id = 	RANKED_RESERVED_ID - rank_id;
	ranked_guild[i].guild_lv = 1;
	ranked_guild[i].max_member = 1;
	strncpy(ranked_guild[i].name, name, NAME_LENGTH);
	sprintf(path, "%s/emblems/%s.ebm", db_path, emblem);
	if( (fp = fopen(path, "rb")) != NULL )
	{
		fseek(fp, 0, SEEK_END);
		ranked_guild[i].emblem_len = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		fread(ranked_guild[i].emblem_data, 1, ranked_guild[i].emblem_len, fp);
		fclose(fp);
		ShowStatus("Done reading '" CL_WHITE "%s" CL_RESET "' ranked emblem data file.\n", path);
	}
	return &ranked_guild[i];
}

int ranked_create(int rank_id, const char *name, float points, const char *emblem, int aura[], int a)
{
	
	struct ranked_data *rank;
	int i;
	ranked_counter++;
	CREATE(rank, struct ranked_data, 1);
	rank->rank_id = rank_id;
	rank->points = points;
	rank->g = ranked_create_guild(rank_id,name,emblem);

	for( i=0; i < a && i < MAX_RANKED_AURA; i++ )
		rank->auras[i] = aura[i];

	idb_put(ranked_db, ranked_counter, rank);
	return true;
}

int ranked_change_emblem(int tid, unsigned int tick, int id, intptr_t data)
{
	struct map_session_data *sd;
	struct guild *g = NULL;
	int rank_id = 0, rk_chg_timer = ranked_config.show_timer, flag = RANK_NONE;

	if( (sd = map_id2sd(id)) == NULL )
		return 0;

	if( tid == INVALID_TIMER )
		return 0;

	rank_id = sd->ranked.current_rank;

	if( sd->ranked.display_pos == RANK_NONE && sd->ranked.current_rank && ranked_show_map(sd->bl.m) )
	{
		struct ranked_data *rank;
		if( (rank = ranked_search(sd->ranked.current_rank)) != NULL ) {
			g = rank->g;
			rk_chg_timer = ranked_config.duration_timer;
			flag = RANK_SHOW;
			clif_ranked_belonginfo(sd,rank->g);
		}
	}

// Crossover Battleground Warfare
#ifdef BG_WARFARE_ENABLE
	if( g == NULL && sd->bg_id )
	{
		struct battleground_data *bg = bg_team_search(sd->bg_id);
		if( bg != NULL && bg->g != NULL ) {
			g = bg->g;
			flag = RANK_NONE;
		}
	}
#endif
	
	if( g == NULL && sd->status.guild_id && (g = guild_search(sd->status.guild_id)) == NULL )
		flag = RANK_NONE;

	if( flag != sd->ranked.display_pos )
	{
		guild_send_dot_remove(sd);
		sd->ranked.display_pos = flag;
		clif_name_self(&sd->bl);
		clif_name_area(&sd->bl);
		if( g != NULL )
			clif_guild_emblem(sd,g);
		clif_guild_emblem_area(&sd->bl);

#ifdef BG_WARFARE_ENABLE
		if( flag == RANK_NONE && sd->bg_id )
		{
			struct battleground_data *bg = bg_team_search(sd->bg_id);
			if( bg != NULL && bg->g != NULL ) {
				g = bg->g;
				bg_guild_requestinfo(sd);
			}
		}
		else
#endif
		if( (flag == RANK_NONE || g == NULL) && sd->status.guild_id ) {
			g = guild_search(sd->status.guild_id);

			// Restore Guild Infos
			if( g != NULL ) {
				clif_guild_emblem(sd, g);
				clif_guild_emblem_area(&sd->bl);
				clif_guild_belonginfo(sd);
			}
		}
	}

	if( !sd->ranked.current_rank && !sd->status.guild_id )
		clif_remove_belonginfo(sd);

	delete_timer(sd->ranked.display_timer, ranked_change_emblem);
	sd->ranked.display_timer = add_timer(gettick()+rk_chg_timer,ranked_change_emblem,sd->bl.id,0);
	return 1;
}

void ranked_update_rank(struct map_session_data *sd, float amount, enum ranked_update status, enum ranked_msg_points type)
{
	struct ranked_data *rank;
	char output[200];
	int rank_id;
	int color = COLOR_RED;
	float points, acc_points, char_points;

	nullpo_retv(sd);

	// Char Points
	if( status == RANK_CHAR || status == RANK_BOTH ) {
		char_points = sd->status.ranked_points + amount;
		if( char_points > ranked_last_points )
			char_points = ranked_last_points;

		sd->status.ranked_points = char_points;
	}
		
	// Account Points
	if( status == RANK_ACCOUNT || status == RANK_BOTH ) {
		char acc_val[254];
		acc_points  = sd->ranked.account_points + amount;
		if( acc_points > ranked_last_points )
			acc_points = ranked_last_points;

		sprintf(acc_val, "%f", acc_points);
		pc_setaccountregstr(sd, add_str("#RANKED_ACCOUNT_POINTS$"), acc_val);
		pc_setaccountreg2str(sd, add_str("#RANKED_ACCOUNT_POINTS$"), acc_val);
	}
	
	points = ranked_config.show_mode ? sd->ranked.account_points : sd->status.ranked_points;
	rank_id = ranked_get_rank(points);
	
	if( amount >= 0 )
		color = COLOR_DEFAULT;

	if( type )
	{
		if( type == 1 )
			sprintf(output, msg_txt(sd, RANKED_MSGTXT+(amount>=0?25:26)), fabs(amount));
		else if( type == 2 )
			sprintf(output, msg_txt(sd, RANKED_MSGTXT+(amount>=0?27:28)), fabs(amount));
		else
			sprintf(output, msg_txt(sd, RANKED_MSGTXT+(amount>=0?29:30)), fabs(amount));
		clif_messagecolor(&sd->bl, color_table[color], output, false, SELF);
	}

	if( rank_id != sd->ranked.current_rank )
	{
		rank = ranked_search(rank_id);
		if( rank == NULL )
			sprintf(output, msg_txt(sd, RANKED_MSGTXT+2));
		else if( amount >= 0 )
			sprintf(output, msg_txt(sd, RANKED_MSGTXT+3), rank->g->name);
		else if( amount <= 0 )
			sprintf(output, msg_txt(sd, RANKED_MSGTXT+4), rank->g->name);
		clif_messagecolor(&sd->bl, color_table[COLOR_RED], output, false, SELF);

		// Reset Timer
		delete_timer(sd->ranked.display_timer,ranked_change_emblem);

		// Update Rank
		if( rank_id <= 0 )
			clif_remove_belonginfo(sd);

		ranked_show_aura(&sd->bl);

		sd->ranked.disable = 0;
		sd->ranked.current_rank = rank_id;
		sd->ranked.display_pos = RANK_NONE;
		sd->ranked.display_timer = add_timer(gettick()+1,ranked_change_emblem,sd->bl.id,0);

		// Special Effect
		if( amount >= 0 )
			clif_specialeffect(&sd->bl, 338, AREA);
		else
			clif_specialeffect(&sd->bl, 38, AREA);
	}
}

void ranked_show_aura(struct block_list *bl)
{
	struct ranked_data *rank;
	struct status_change *sc = NULL;
	int i, current_rank = ranked_getbl_rank(bl);

	if( bl->type != BL_PC )
		return;

	sc = status_get_sc(bl);
	if( sc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK|OPTION_INVISIBLE) ) // ranger ->|| sc->data[SC_CAMOUFLAGE] )
		return;

	// Reset Aura
	//for (i = 0; i < MAX_RANKED_AURA; i++) {
	//	clif_specialeffect(bl, 0, AREA);
	//}

	// Show Aura
	if( (rank = ranked_search(current_rank)) != NULL && ranked_map_aura(bl->m) ) {
		for( i = 0; i < MAX_RANKED_AURA; i++ ) {
			if( rank->auras[i] > 0 )
				clif_specialeffect(bl, rank->auras[i], AREA);
		}
	}
}

void ranked_show_list(struct map_session_data *sd)
{
	DBIterator* iter = db_iterator(ranked_db);
	struct ranked_data* rank;
	char output[200];

	nullpo_retv(sd);

	clif_displaymessage(sd->fd, msg_txt(sd,RANKED_MSGTXT));
	for( rank = (struct ranked_data*)dbi_first(iter); dbi_exists(iter); rank = (struct ranked_data*)dbi_next(iter) )
	{
		sprintf(output, msg_txt(sd,RANKED_MSGTXT+1), rank->rank_id, rank->g->name, rank->points);
		clif_displaymessage(sd->fd, output);
	}
	dbi_destroy(iter);
}

int ranked_config_read(void)
{
	char line[1024], w1[1024], w2[1024];
	const char *cfgName;
	FILE *fp;

	// Read Script
	cfgName = "conf/battle/creativesd.conf";

	// Default values
	ranked_config.show_mode = 0;
	ranked_config.show_timer = 10000;
	ranked_config.duration_timer = 5000;
	ranked_config.mode = 0x1;
	ranked_config.show_maps = 0x111;
	ranked_config.show_aura = 0x111;
	ranked_config.kill_points_gain = 2;
	ranked_config.kill_points_drop = 1;
	ranked_config.support_points = 0.5;

	fp = fopen(cfgName,"r");
	if( fp == NULL )
	{
		ShowError("Ranked System file not found at: %s\n", cfgName);
		ShowError("Setting configuration from rank default values\n");
		return 0;
	}

	while( fgets(line, sizeof(line), fp) )
	{
		char* ptr;

		if( line[0] == '/' && line[1] == '/' )
			continue;
		if( (ptr = strstr(line, "//")) != NULL )
			*ptr = '\n'; //Strip comments
		if( sscanf(line, "%[^:]: %[^\t\r\n]", w1, w2) < 2 )
			continue;

		//Strip trailing spaces
		ptr = w2 + strlen(w2);
		while (--ptr >= w2 && *ptr == ' ');
		ptr++;
		*ptr = '\0';
		
		if( stristr(w1,"ranked_") == NULL )
			continue;
		else if( strcmpi(w1,"ranked_show_mode") == 0 )
			ranked_config.show_mode = config_switch(w2);
		else if( strcmpi(w1, "ranked_show_timer") == 0 )
			ranked_config.show_timer = config_switch(w2);
		else if( strcmpi(w1, "ranked_duration_timer") == 0 )
			ranked_config.duration_timer = config_switch(w2);
		else if( strcmpi(w1, "ranked_mode") == 0 )
			ranked_config.mode = config_switch(w2);
		else if( strcmpi(w1, "ranked_show_maps") == 0 )
			ranked_config.show_maps = config_switch(w2);
		else if( strcmpi(w1, "ranked_show_aura") == 0 )
			ranked_config.show_aura = config_switch(w2);
		else if( strcmpi(w1, "ranked_kill_points_gain") == 0 )
			ranked_config.kill_points_gain = (float)atof(w2);
		else if( strcmpi(w1, "ranked_kill_points_drop") == 0 )
			ranked_config.kill_points_drop = (float)atof(w2);
		else if( strcmpi(w1, "ranked_support_points") == 0 )
			ranked_config.support_points = (float)atof(w2);
		else
			ShowWarning("Unknown setting '%s' in file %s\n", w1, cfgName);
	}
	fclose(fp);
	return 1;
}

static bool ranked_parse_dbrow(char** str, const char* source, int line)
{
	struct ranked_data *rank;
	int rank_id, i, a = 0;
	float points;
	char name[NAME_LENGTH];
	char path[256];
	char *aura_tmp;
	const char *emblem;
	int auras[MAX_RANKED_AURA];

	rank_id = atoi(str[0]);
	safestrncpy(name, str[1], sizeof(name));
	points = (float)atof(str[2]);
	emblem = str[3];

	aura_tmp = str[4];
	aura_tmp = strchr(aura_tmp+1,'#');
	for( i=0; i < MAX_RANKED_AURA && aura_tmp; i++ )
	{
		if( !sscanf(aura_tmp, "%d", &auras[a]) && !sscanf(aura_tmp, "#%d", &auras[a]) )
		{
			ShowWarning("ranked_parse_dbrow: Error parsing aura effects in line %d of \"%s\" (rank id: %d), skipping effect\n", line, source, rank_id);
			aura_tmp = strchr(aura_tmp+1,'#');
			continue;
		}
		aura_tmp = strchr(aura_tmp+1,'#');
		a++;
	}

	if( ranked_counter > MAX_RANKED_DB )
	{
		ShowError("ranked_parse_dbrow: The quantity has been exceeded by the limit! Ignoring (rank id: %d)\n", rank_id);
		return false;
	}

	if( points < ranked_last_points )
	{
		ShowError("ranked_parse_dbrow: The number of points is less than the last defined in line %d of \"%s\" (rank id: %d), skipping\n", line, source, rank_id);
		return false;
	}

	if( rank_id <= 0 ) {
		ShowWarning("ranked_parse_dbrow: Invalid  ID %d in line %d of \"%s\", skipping.\n", rank_id, line, source);
		return false;
	}

	rank = ranked_search(rank_id);

	if( rank != NULL ) {
		ShowWarning("ranked_parse_dbrow: Duplicate ID %d in line %d of \"%s\", skipping.\n", rank_id, line, source);
		return false;
	}

	if( strlen(name) <= 0 )
	{
		ShowWarning("ranked_parse_dbrow: No name defined in line %d of \"%s\" (rank id: %d), skipping\n", line, source, rank_id);
		return false;
	}

	if( strlen(emblem) <= 0 )
	{
		ShowWarning("ranked_parse_dbrow: No emblem defined in line %d of \"%s\" (rank id: %d), skipping\n", line, source, rank_id);
		return false;
	}

	sprintf(path, "%s/emblems/%s.ebm", db_path, emblem);
	if( !exists(path) )
	{
		ShowWarning("ranked_parse_dbrow: File not found in line %d of \"%s\" (rank id: %d), skipping\n", line, source, rank_id);
		return false;
	}
	ranked_create(rank_id, name, points, emblem, auras, a);
	ranked_last_points = points;
	return true;
}

void ranked_readdb(void)
{
	uint32 lines = 0, count = 0;
	char line[1024];
	char path[256];
	FILE* fp;

	memset(&ranked_guild, 0, sizeof(ranked_guild));
	sprintf(path, "%s/ranked_db.txt", db_path);
	if( (fp = fopen(path, "r")) != NULL ) {
		// process rows one by one
		while(fgets(line, sizeof(line), fp))
		{
			char *str[32], *p;
			int i;

			lines++;
			if(line[0] == '/' && line[1] == '/')
				continue;
			memset(str, 0, sizeof(str));

			p = line;
			while( ISSPACE(*p) )
				++p;
			if( *p == '\0' )
				continue;// empty line
			for( i = 0; i < 4; ++i )
			{
				str[i] = p;
				p = strchr(p,',');
				if( p == NULL )
					break;// comma not found
				*p = '\0';
				++p;
			}

			if( p == NULL )
			{
				ShowError("ranked_readdb: Insufficient columns in line %d of \"%s\" (rank id %d), skipping.\n", lines, path, atoi(str[0]));
				continue;
			}

			// Script
			if( *p != '{' )
			{
				ShowError("ranked_readdb: Invalid format (Aura column) in line %d of \"%s\" (rank id %d), skipping.\n", lines, path, atoi(str[0]));
				continue;
			}
			str[4] = p;

			if (!ranked_parse_dbrow(str, path, lines))
				continue;
			count++;
		}
		fclose(fp);
		ShowStatus("Done reading '" CL_WHITE "%lu" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", count, path);
	}
	else {
		ShowWarning("ranked_readdb: File not found \"%s\", skipping.\n", path);
	}
}

void do_reload_ranked(void)
{
	ranked_db = idb_alloc(DB_OPT_RELEASE_DATA);
	ranked_counter = 0;
	ranked_readdb();
}

void do_init_ranked(void)
{
	ranked_db = idb_alloc(DB_OPT_RELEASE_DATA);
	ranked_config_read();
	ranked_readdb();

	add_timer_func_list(ranked_change_emblem,"ranked_change_emblem");

	ShowMessage(CL_WHITE"[Ranked System]:" CL_RESET " Ranked System (version: 2.0.00) successfully initialized.\n");
	ShowMessage(CL_WHITE"[Ranked System]:" CL_RESET " by (c) CreativeSD, suport in " CL_GREEN "www.creativesd.com.br" CL_RESET "\n");
}

void do_final_ranked(void)
{
	ranked_counter = 0;
	ranked_db->destroy(ranked_db, NULL);
}

#endif // RANKED_SYSTEM_ENABLE