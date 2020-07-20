//-----------------------------------------------------------------------------
//
//  $Logfile:: /fakk2_code/fakk2_new/cgame/cg_servercmds.c                    $
// $Revision:: 12                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 9/20/00 6:58p                                                  $
//
// Copyright (C) 1998 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// $Log:: /fakk2_code/fakk2_new/cgame/cg_servercmds.c                         $
// 
// 12    9/20/00 6:58p Aldie
// Another temporary checkin
// 
// 11    9/18/00 6:00p Aldie
// Periodic update for FakkArena deathmatch code.
// 
// 10    9/01/00 7:09p Aldie
// More fakk arena work
// 
// 9     8/30/00 6:35p Aldie
// More updates for deathmatch
// 
// 8     7/24/00 6:46p Steven
// Changed sv_cinematic from a cvar to a player stat.
// 
// 7     7/14/00 9:52p Markd
// added global volume dampener on ambient sound effects for cinematics
// 
// 6     2/29/00 5:51p Jimdose
// added alternate spawnpoint support
// 
// 5     12/11/99 5:51p Markd
// First wave of bug fixes after q3a gold merge
// 
// 4     12/11/99 3:37p Markd
// q3a gold checkin, first time
// 
// 3     10/05/99 6:01p Aldie
// Added headers
//
// DESCRIPTION: 
// cg_servercmds.c -- text commands sent by the server

#include "cg_local.h"


//Yorvik
#include "ctf_cg_drawtools.h"

void CTF_Location(void);
void CTF_Scores(void);
void CTF_UpdateTechs(void);
void CTF_InitLocations(void);
void CTF_IntermissionReady(void);
void CTF_AddReward(int type, int count, qboolean playsound);
void CTF_ClearRewards(void);
void CTF_DrawMOTD(void);
void CTF_FollowEnt(int entnum);
void CTF_ItemPickup(const char*, const char*);
//Y


/*
================
CG_ParseServerinfo

This is called explicitly when the gamestate is first received,
and whenever the server updates any serverinfo flagged cvars
================
*/
void CG_ParseServerinfo( void ) {
	const char	*info;
	const char	*mapname;
   char        map[ MAX_QPATH ];
   char        *spawnpos;

	info = CG_ConfigString( CS_SERVERINFO );
	cgs.gametype = atoi( Info_ValueForKey( info, "g_gametype" ) );
	cgs.dmflags = atoi( Info_ValueForKey( info, "dmflags" ) );
	cgs.teamflags = atoi( Info_ValueForKey( info, "teamflags" ) );
	cgs.fraglimit = atoi( Info_ValueForKey( info, "fraglimit" ) );
	cgs.timelimit = atoi( Info_ValueForKey( info, "timelimit" ) );
	cgs.maxclients = atoi( Info_ValueForKey( info, "sv_maxclients" ) );

	//Yorvik
	cgs.capturelimit = atoi(Info_ValueForKey(info, "capturelimit"));
	//Y

   // set up some ROM cvars so we can update the UI properly
   cgi.Cvar_Set( "cg_gametype",   Info_ValueForKey( info, "g_gametype" ) );
   cgi.Cvar_Set( "cg_fraglimit",  Info_ValueForKey( info, "fraglimit" ) );
   cgi.Cvar_Set( "cg_timelimit",  Info_ValueForKey( info, "timelimit" ) );
   cgi.Cvar_Set( "cg_maxclients", Info_ValueForKey( info, "sv_maxclients" ) );

	mapname = Info_ValueForKey( info, "mapname" );

   spawnpos = strchr( mapname, '$' );
   if ( spawnpos )
      {
      Q_strncpyz( map, mapname, spawnpos - mapname + 1 );
      }
   else
      {
      strcpy( map, mapname );
      }

	Com_sprintf( cgs.mapname, sizeof( cgs.mapname ), "maps/%s.bsp", map );
}


/*
================
CG_ConfigStringModified

================
*/
static void CG_ConfigStringModified( void )
   {	
	int		num;

	num = atoi( cgi.Argv( 1 ) );

	// get the gamestate from the client system, which will have the
	// new configstring already integrated
	cgi.GetGameState( &cgs.gameState );

   CG_ProcessConfigString( num );
   }

/*
===============
CG_MapRestart
===============
*/
static void CG_MapRestart( void ) {
	if ( cg_showmiss->integer ) {
		cgi.Printf( "CG_MapRestart\n" );
	}
   CG_Shutdown();
   CG_Init( &cgi, cgs.processedSnapshotNum, cgs.serverCommandSequence );
}

/*
=================
CG_ServerCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
static void CG_ServerCommand( void )
   {
	const char	*cmd;
	int i;

	cmd = cgi.Argv(0);

	if ( !cmd[0] ) 
      {
		// server claimed the command
		return;
	   }

	if ( !strcmp( cmd, "cs" ) )
      {
		CG_ConfigStringModified();
		return;
	   }

	if ( !strcmp( cmd, "print" ))
      {
		//Yorvik - make print commands output to screen also
		CG_HudPrint_UI_f();
		//Y
      
      	return;
	   }

	if ( !strcmp( cmd, "map_restart" ) ) 
      {
		CG_MapRestart();
		return;
	   }

   if ( !Q_stricmp(cmd, "stufftext") )
      {
	   cgi.Cmd_Stuff( cgi.Argv( 1 ) );
      cgi.Cmd_Stuff( "\n" );
		return;
	   }

   if ( !Q_stricmp( cmd, "scores" ) )
      {
      cgi.UI_ParseScores();
      return;
      }

   if ( !strcmp( cmd, "status" ) )
      {
      CG_StatusUI_f();
      return;
      }

   //Yorvik
	if(!Q_stricmp(cmd, "ctf_singlescore"))
	{
		CTF_Scores();
		return;
	}

	if(!Q_stricmp(cmd, "ctf_singlelocation"))
	{
		CTF_Location();
		return;
	}

	if(!Q_stricmp(cmd, "ctf_updatetechs"))
	{
		CTF_UpdateTechs();
		return;
	}

	if(!Q_stricmp(cmd, "ctf_initlocations"))
	{
		CTF_InitLocations();
		return;
	}

	if(!Q_stricmp(cmd, "intermissionready"))
	{
		CTF_IntermissionReady();
		return;
	}

	if(!Q_stricmp(cmd, "votestring"))
	{
		strcpy(ctf_voteString, cgi.Argv(1));

		for(i=2; i<cgi.Argc(); ++i)
		{
			strcat(ctf_voteString, " ");
			strcat(ctf_voteString, cgi.Argv(i));
		}

		return;
	}

	if(!Q_stricmp(cmd, "addreward"))
	{
		CTF_AddReward(atoi(cgi.Argv(1)), atoi(cgi.Argv(2)), qtrue);
		return;
	}

	if(!Q_stricmp(cmd, "clear_rewards"))
	{
		CTF_ClearRewards();
		return;
	}

	if(!Q_stricmp(cmd, "addreward_nosound"))
	{
		CTF_AddReward(atoi(cgi.Argv(1)), atoi(cgi.Argv(2)), qfalse);
		return;
	}

	if(!Q_stricmp(cmd, "motd"))
	{
		CTF_DrawMOTD();
		return;
	}

	if(!Q_stricmp(cmd, "followent"))
	{
		CTF_FollowEnt(atoi(cgi.Argv(1)));
		return;
	}

	if(!Q_stricmp(cmd, "item_pickup"))
	{
		int i = 0;
		char buf[1024];
		strcpy(buf, "");
		
		for(i=2; i<=cgi.Argc(); ++i)
			strcat(buf, va("%s ", cgi.Argv(i)));
		
		CTF_ItemPickup(cgi.Argv(1), buf);
		return;
	}

	if(!Q_stricmp(cmd, "redbasepos"))
	{
		cgs.redBaseOrigin[0] = atof(cgi.Argv(1));
		cgs.redBaseOrigin[1] = atof(cgi.Argv(2));
		cgs.redBaseOrigin[2] = atof(cgi.Argv(3));
		return;
	}
	
	if(!Q_stricmp(cmd, "bluebasepos"))
	{
		cgs.blueBaseOrigin[0] = atof(cgi.Argv(1));
		cgs.blueBaseOrigin[1] = atof(cgi.Argv(2));
		cgs.blueBaseOrigin[2] = atof(cgi.Argv(3));
		return;
	}
   //Y



	cgi.Printf( "Unknown client game command: %s\n", cmd );
   }


/*
====================
CG_ExecuteNewServerCommands

Execute all of the server commands that were received along
with this this snapshot.
====================
*/
void CG_ExecuteNewServerCommands( int latestSequence ) {
	while ( cgs.serverCommandSequence < latestSequence ) {
		if ( cgi.GetServerCommand( ++cgs.serverCommandSequence ) ) {
			CG_ServerCommand();
		}
	}
}
