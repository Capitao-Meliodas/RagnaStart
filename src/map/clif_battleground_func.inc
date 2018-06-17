// © Creative Services and Development
// Site Oficial: www.creativesd.com.br
// Termos de Contrato e Autoria em: http://creativesd.com.br/?p=termos

/*==========================================
 * CreativeSD: BattleGround Queue Expansive
 *------------------------------------------*/
// Adiciona o icone de Batalhas no topo dos monstros.
void clif_sendbgmobemblem_single(int fd, struct mob_data *md)
{
	nullpo_retv(md);

	if( !(battle_config.bg_enable_cross_swords&0x01) )
		return;

	WFIFOHEAD(fd, 32);
	WFIFOW(fd, 0) = 0x2dd;
	WFIFOL(fd, 2) = md->bl.id;
	safestrncpy((char*)WFIFOP(fd, 6), md->name, NAME_LENGTH);
	WFIFOW(fd, 30) = md->bg_id;
	WFIFOSET(fd, packet_len(0x2dd));
}

// Envia informações básicas do Clã nas Batalhas.
void clif_bg_basicinfo(struct map_session_data *sd)
{
	int fd;
	struct battleground_data *bg;
#if PACKETVER < 20160622
	int cmd = 0x1b6;
	int offset = NAME_LENGTH;
#else
	int cmd = 0xa84;
	int offset = 0;
#endif

	nullpo_retv(sd);
	fd = sd->fd;

	if( !sd->bg_id || (bg = bg_team_search(sd->bg_id)) == NULL )
		return;

	WFIFOHEAD(fd,packet_len(cmd));
	WFIFOW(fd, 0)=cmd;
	WFIFOL(fd, 2)=bg->g->guild_id;
	WFIFOL(fd, 6)=bg->g->guild_lv;
	WFIFOL(fd,10)=bg->count;
	WFIFOL(fd,14)=bg->g->max_member;
	WFIFOL(fd,18)=0;
	WFIFOL(fd,22)=0;
	WFIFOL(fd,26)=0;
	WFIFOL(fd,30)=0;	// Tax Points
	WFIFOL(fd,34)=0;	// Honor: (left) Vulgar [-100,100] Famed (right)
	WFIFOL(fd,38)=0;	// Virtue: (down) Wicked [-100,100] Righteous (up)
	WFIFOL(fd,42)=bg->g->emblem_id;
	safestrncpy(WFIFOCP(fd,46),bg->g->name, NAME_LENGTH);
#if PACKETVER < 20160622
	safestrncpy(WFIFOCP(fd,70),bg->g->master, NAME_LENGTH);
#endif
	safestrncpy(WFIFOCP(fd,70+offset),bg->g->master,16);
	WFIFOL(fd,70+offset+16) = 0;
#if PACKETVER >= 20160622
	WFIFOL(fd,70+offset+20) = bg->master_id;  // leader
#endif

	WFIFOSET(fd,packet_len(cmd));
}

// Envia informações do Clã nas Batalhas.
void clif_bg_belonginfo(struct map_session_data *sd)
{
	int fd;
	struct battleground_data *bg;
	nullpo_retv(sd);

	if( !sd->bg_id || (bg = bg_team_search(sd->bg_id)) == NULL )
		return;

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x16c));
	WFIFOW(fd,0)=0x16c;
	WFIFOL(fd,2)=bg->g->guild_id;
	WFIFOL(fd,6)=bg->g->emblem_id;
	WFIFOL(fd,10)=0x10;
	WFIFOB(fd,14)=0;
	WFIFOL(fd,15)=0;  // InterSID (unknown purpose)
	memcpy(WFIFOP(fd,19),bg->g->name,NAME_LENGTH);
	WFIFOSET(fd,packet_len(0x16c));
}

// Cria e envia a lista de membros das Batalhas.
void clif_bg_memberlist(struct map_session_data *sd)
{
	int fd;
	int i,c;
	struct battleground_data *bg;
	struct map_session_data *psd;
#if PACKETVER < 20161026
	int cmd = 0x154;
	int size = 104;
#else
	int cmd = 0xaa5;
	int size = 34;
#endif

	nullpo_retv(sd);

	if( (fd = sd->fd) == 0 )
		return;

	if( !(sd->bg_id && (bg = bg_team_search(sd->bg_id)) != NULL) )
		return;

	WFIFOHEAD(fd, MAX_BG_MEMBERS * size + 4);
	WFIFOW(fd, 0)=cmd;
	for( i = 0, c = 0; i < MAX_BG_MEMBERS; i++ )
	{
		if( (psd = bg->members[i].sd) == NULL )
			continue;
		
		WFIFOL(fd,c*size+ 4)=psd->status.account_id;
		WFIFOL(fd,c*size+ 8)=psd->status.char_id;
		WFIFOW(fd,c*size+12)=psd->status.hair;
		WFIFOW(fd,c*size+14)=psd->status.hair_color;
		WFIFOW(fd,c*size+16)=psd->status.sex;
		WFIFOW(fd,c*size+18)=psd->status.class_;
		WFIFOW(fd,c*size+20)=psd->status.base_level;
		WFIFOL(fd,c*size+22)=(int)cap_value(psd->status.base_exp,0,INT32_MAX);
		WFIFOL(fd,c*size+26)=1;
		WFIFOL(fd,c*size+30)=psd->bg_position;
#if PACKETVER < 20161026
		memset(WFIFOP(fd,c*size+34),0,50);	//[Ind] - This is displayed in the 'note' column but being you can't edit it it's sent empty.
		safestrncpy(WFIFOCP(fd,c*size+84),psd->status.name,NAME_LENGTH);
#else
		WFIFOL(fd,c*size+34)=0;
#endif
		c++;
	}
	WFIFOW(fd, 2)=c*size+4;
	WFIFOSET(fd,WFIFOW(fd,2));
}