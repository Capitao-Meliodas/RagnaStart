// © Creative Services and Development
// Site Oficial: www.creativesd.com.br
// Termos de Contrato e Autoria em: http://creativesd.com.br/?p=termos

/*==========================================
 * CreativeSD: Battleground Warfare
 *------------------------------------------*/
struct guild* bg_get_guild(int idx);
int bg_refresh_patent(int bg_id);
void bg_clean_skill(struct battleground_data *bg);
void bg_block_skill(struct map_session_data *sd, int time);
void bg_block_skill_change(struct map_session_data *sd);
void bg_block_skill_end(struct map_session_data *sd);
int bg_report_afk(int tid, unsigned int tick, int id, intptr_t data);
int bg_kick_player(struct map_session_data* sd, struct map_session_data *pl_sd, const char* mes);
int bg_change_master(int bg_id, int char_id, struct map_session_data *sd);
int bg_respawn_timer(int tid, unsigned int tick, int id, intptr_t data);
void bg_respawn_timer_delete(struct map_session_data *sd);
int bg_digit_timer(int tid, unsigned int tick, int id, intptr_t data);
void bg_create_guild();
void bg_guild_requestinfo(struct map_session_data *sd);
void bg_score_damage(struct block_list *src, struct block_list *dst, int damage);
void bg_score_kills(struct block_list *src, struct block_list *dst);
void bg_score_heal(struct block_list *bl, int hp, int sp);
void bg_score_item_heal(struct map_session_data *sd, int amount, int hp, int sp);
void bg_score_del_item(struct map_session_data *sd, int item_id, int amount);
struct battleground_rewards* bg_create_reward(int arena_id);
struct battleground_rewards* bg_search_reward(int arena_id);
void bg_reload_rewards();
int bg_load_rewards_from_sql();
int bg_reward(struct map_session_data *sd, int arena_id, enum bg_reward_type flag);
int bg_flag_alarm_npc(int tid, unsigned int tick, int id, intptr_t data);
int bg_flag_alarm_player(int tid, unsigned int tick, int id, intptr_t data);
int bg_set_npc(int bg_id, struct npc_data *nd, int npc_id, short x, short y, const char *ev, const char *rev);
int bg_flag_drop(struct map_session_data *sd);
int bg_flag_capture_area(struct map_session_data* sd, int m, int x, int y);
int bg_flag_catch_area(struct map_session_data* sd, int m, int x, int y);
int bg_flag_respawn(int m, int bg1_id, int bg2_id);
int bg_guild_refresh_all(struct battleground_data *bg);
void do_bg_reward_reload(void);
