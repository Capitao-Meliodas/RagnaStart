// © Creative Services and Development
// Site Oficial: www.creativesd.com.br
// Termos de Contrato e Autoria em: http://creativesd.com.br/?p=termos

/*==========================================
 * CreativeSD: Ranked System
 *------------------------------------------*/
 void clif_ranked_belonginfo(struct map_session_data *sd, struct guild *g)
{
	int fd;
	nullpo_retv(sd);
	//nullpo_retv(g);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x16c));
	WFIFOW(fd,0)=0x16c;
	WFIFOL(fd,2)=g->guild_id;
	WFIFOL(fd,6)=g->emblem_id;
	WFIFOL(fd,10)=0;
	WFIFOB(fd,14)=1;
	WFIFOL(fd,15)=0;  // InterSID (unknown purpose)
	memcpy(WFIFOP(fd,19),g->name,NAME_LENGTH);
	WFIFOSET(fd,packet_len(0x16c));
}

void clif_remove_belonginfo(struct map_session_data *sd)
{
	int fd;
	nullpo_retv(sd);

	fd=sd->fd;
	WFIFOHEAD(fd,packet_len(0x16c));
	WFIFOW(fd,0)=0x16c;
	WFIFOL(fd,2)=0;
	WFIFOL(fd,6)=0;
	WFIFOL(fd,10)=0;
	WFIFOB(fd,14)=0;
	WFIFOL(fd,15)=0;  // InterSID (unknown purpose)
	memcpy(WFIFOP(fd,19),"",NAME_LENGTH);
	WFIFOSET(fd,packet_len(0x16c));
}