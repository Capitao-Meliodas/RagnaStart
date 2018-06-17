// © Creative Services and Development
// Site Oficial: www.creativesd.com.br
// Termos de Contrato e Autoria em: http://creativesd.com.br/?p=termos
//

/*==========================================
 * CreativeSD: Ranked System
 *------------------------------------------*/

 // Get Rank info
//
// * getrankinfo(<rank_id>, <type>)
//	<type>
//		0: Rank Name
//		1: Points required
//
BUILDIN_FUNC(getrankinfo)
{
	struct ranked_data *rank = ranked_search(script_getnum(st,2));
	int type = script_getnum(st,3);

	if( rank == NULL )
	{
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	switch(type)
	{
		case 0:
			{
				char points[254];
				sprintf(points, "%f", rank->points);
				script_pushstrcopy(st,points);
			}
			break;
		case 1:
			script_pushstrcopy(st,rank->g->name);
			break;
		default:
			ShowWarning("buildin_ranked_get_info: invalid type '%d'.\n", type);
			script_pushint(st,0);
			break;
	}
	return SCRIPT_CMD_SUCCESS;
}

// Get Rank from Player
//
// * getcharrankinfo(<type>,{<account_id>})
//	<type>
//		0: Rank ID
//		1: Points (Char)
//		2: Points (Account)
//
BUILDIN_FUNC(getcharrankinfo)
{
	TBL_PC *sd;
	int type = script_getnum(st,2);

	if( script_hasdata(st, 3) )
		sd = map_id2sd(script_getnum(st, 3));
	else
		script_rid2sd(sd);

	if( sd == NULL ) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	switch(type)
	{
		case 0:
			script_pushint(st,sd->ranked.current_rank);
			break;
		case 1:
			{
				int rank_id = ranked_get_rank(sd->ranked.account_points);
				script_pushint(st,rank_id);
			}
			break;
		case 2:
			{
				char points[254];
				sprintf(points, "%f", sd->status.ranked_points);
				script_pushstrcopy(st,points);
			}
			break;
		case 3:
			{
				char points[254];
				sprintf(points, "%f", sd->ranked.account_points);
				script_pushstrcopy(st,points);
			}
			break;
		default:
			ShowWarning("buildin_get_rank_info: invalid type '%d'.\n", type);
			script_pushint(st,0);
			break;
	}
	return SCRIPT_CMD_SUCCESS;
}

//	Add points for player
//
// * setrankpoints <points>{<type>,{,<account_id>}}
//	<type>:
//		0: Upate Char Points Only
//		1: Update Account Points Only
//		2: Update Both
//
BUILDIN_FUNC(setrankpoints)
{
	TBL_PC *sd;
	struct script_data* data;
	int type = RANK_BOTH;
	float points;

	data = script_getdata(st,2);
	get_val(st, data);

	if( data_isstring(data) )
		points = (float)atof(conv_str(st,data));
	else
		points = (float)conv_num(st,data);
	
	if( script_hasdata(st,3) )
	{
		type = script_getnum(st,3);
		if( type < RANK_CHAR || type > RANK_BOTH )
		{
			ShowError("buildin_setrankpoints: Invalid type %d.\n", type);
			st->state = END;
			return SCRIPT_CMD_FAILURE;
		}
	}

	if( script_hasdata(st,4) )
		sd = map_id2sd(script_getnum(st,4));
	else
		script_rid2sd(sd);

	if( sd == NULL ) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	ranked_update_rank(sd,(float)points,(enum ranked_update)type,RANK_P_OTHERS);
	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

//	Set Rank for Player
//
// * setrank <rank_id>{,<type>{,<account_id>}}
//	<type>:
//		0: Upate Char Points Only
//		1: Update Account Points Only
//		2: Update Both
//
BUILDIN_FUNC(setrank)
{
	TBL_PC *sd;
	struct ranked_data *rank;
	int rank_id = script_getnum(st,2), type = RANK_BOTH;
	
	if( script_hasdata(st,3) )
	{
		type = script_getnum(st,3);
		if( type < RANK_CHAR || type > RANK_BOTH )
		{
			ShowError("buildin_setrank: Invalid type %d.\n", type);
			st->state = END;
			return SCRIPT_CMD_FAILURE;
		}
	}

	if( (rank = ranked_search(rank_id)) == NULL )
	{
		ShowError("buildin_setrank: Rank not found %d.\n", rank_id);
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	if( script_hasdata(st, 4) )
		sd = map_id2sd(script_getnum(st, 4));
	else
		script_rid2sd(sd);

	if( sd == NULL ) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}
	
	type++;
	if( type&1 )
		sd->ranked.account_points = rank->points;
	if( type&2 )
		sd->status.ranked_points = rank->points;

	ranked_update_rank(sd,0,(enum ranked_update)(type-1),RANK_P_OTHERS);
	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}