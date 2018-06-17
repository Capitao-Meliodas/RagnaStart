// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _BATTLEGROUND_HPP_
#define _BATTLEGROUND_HPP_

#include "../common/cbasetypes.h"
#include "../common/mmo.h" // struct party

#define MAX_BG_MEMBERS 30

// [CreativeSD]: Names, Version and Rewards
#ifdef BG_WARFARE_ENABLE
static const char bg_version[7] = "2.0.02";
#endif

#ifndef BG_COLOR
	#define	BG_COLOR	0xFFA500
#endif

enum bg_team_leave_type {
	BGTL_LEFT = 0x0,
	BGTL_QUIT = 0x1,
	BGTL_AFK  = 0x2,
	BGTL_KICK = 0x3
};

#ifdef BG_WARFARE_ENABLE
enum bg_reward_type {
	BGRT_WIN,
	BGRT_LOSS,
	BGRT_WO
};

enum bg_reward_items {
	BGRI_ITEM,
	BGRI_ZENY,
	BGRI_CASH,
	BGRI_KAFRA,
	BGRI_BEXP,
	BGRI_JEXP,
	BGRI_BLVL,
	BGRI_JLVL,
	BGRI_BONUS_STATUS,
	BGRI_CHAR_VAR,
	BGRI_ACC_VAR
};

// [CreativeSD]: Battleground Warfare
struct batleground_statistic {
	// Skills
	unsigned int skills_support_success, skills_support_fail;
	// Healing
	unsigned int heal_hp, heal_sp;
	unsigned int item_heal_hp, item_heal_sp;
	// Items
	unsigned int item_heal, ammos, poison_bottle, fire_bottle, acid_bottle;
	unsigned int oridecon, elunium, steel, emveretarcon, wooden_block, stone, yellow_gemstone, red_gemstone, blue_gemstone;
	// Vs Players
	unsigned int player_deaths, player_kills;
	unsigned int player_damage_taken, player_damage_given;
	// Vs Rune Stones
	unsigned int runestone_kills, runestone_damage, runestone_repair;
	// Vs Emperium
	unsigned int emperium_kills, emperium_damage;
	// Vs Barricades
	unsigned int barrier_kills, barrier_damage, barrier_repair;
	// Vs Objectives and Supplies
	unsigned int objective_kills, objective_damage;
	// Vs Flags
	unsigned int flag_kills, flag_damage, flag_capture, flag_recapture;
	// Vs Crystals
	unsigned int cristal_kills, cristal_damage;
	// Vs Guardians
	unsigned int guardian_kills, guardian_deaths;
	unsigned int guardian_damage_taken, guardian_damage_given;
};

struct battleground_rewards_objects {
	int object_type, value, flag_type, rate;
	char object[32], desc[32];
};

struct battleground_rewards {
	int arena_id;
	unsigned char count;
	struct battleground_rewards_objects items[MAX_BG_REWARDS];
};

extern struct guild bg_guild[];
#endif

// [CreativeSD]: Modify for Battleground Warfare
struct battleground_member_data {
	unsigned short x, y;
	struct map_session_data *sd;
	unsigned afk : 1;
};

struct battleground_data {
	unsigned int bg_id;
	unsigned char count;
	struct battleground_member_data members[MAX_BG_MEMBERS];
	// BG Cementery
	unsigned short mapindex, x, y;
	// Logout Event
	char logout_event[EVENT_NAME_LENGTH];
	char die_event[EVENT_NAME_LENGTH];
#ifdef BG_WARFARE_ENABLE
	// Skills
	int skill_block_timer[MAX_GUILDSKILL];
	// Fake Guild
	struct guild *g;
	int army, master_id, timerdigit_count, timerdigit;
	// Respawn Timer
	unsigned short respawn_x, respawn_y;
	// Without Event
	char without_event[EVENT_NAME_LENGTH];
#endif
};

void do_init_battleground(void);
void do_final_battleground(void);

struct battleground_data* bg_team_search(int bg_id);
int bg_send_dot_remove(struct map_session_data *sd);
int bg_team_get_id(struct block_list *bl);
struct map_session_data* bg_getavailablesd(struct battleground_data *bg);

int bg_create_sub(int bg_id, unsigned short mapindex, short rx, short ry, short rsx, short rsy, int army, const char *ev, const char *dev, const char *wev);
#ifdef BG_WARFARE_ENABLE
#define bg_create(bg_id, mapindex, rx, ry, rsx, rsy, army, ev, dev, wev) bg_create_sub(bg_id, mapindex, rx, ry, rsx, rsy, army, ev, dev, wev)
#else
#define bg_create(mapindex, rx, ry, ev, dev) bg_create_sub(0,mapindex,rx,ry,0,0,0,ev,dev,"")
#endif
int bg_team_join(int bg_id, struct map_session_data *sd, int flag);
int bg_team_delete(int bg_id);
int bg_team_leave(struct map_session_data *sd, enum bg_team_leave_type flag);
int bg_team_warp(int bg_id, unsigned short mapindex, short x, short y);
int bg_member_respawn(struct map_session_data *sd);
int bg_send_message(struct map_session_data *sd, const char *mes, int len);

#ifdef BG_WARFARE_ENABLE
/*==========================================
 * CreativeSD: Battleground Warfare
 *------------------------------------------*/
#include "battleground_def.inc"
#endif

#endif /* _BATTLEGROUND_HPP_ */
