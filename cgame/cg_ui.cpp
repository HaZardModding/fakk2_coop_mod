//-----------------------------------------------------------------------------
//
//  $Logfile:: /fakk2_code/fakk2_new/cgame/cg_ui.cpp                          $
// $Revision:: 4                                                              $
//   $Author:: Aldie                                                          $
//     $Date:: 9/20/00 6:58p                                                  $
//
// Copyright (C) 2000 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// $Log:: /fakk2_code/fakk2_new/cgame/cg_ui.cpp                               $
// 
// 4     9/20/00 6:58p Aldie
// Another temporary checkin
// 
// 3     9/18/00 6:00p Aldie
// Periodic update for FakkArena deathmatch code.
//
// DESCRIPTION:
// User interface stuff from the client game

#include "cg_local.h"

//Yorvik
#include "ctf_cg_drawtools.h"
//Y

//say (normal)
void CG_MessageMode_f
   (
   void
   )

   {
   // Put the focus into the dm_console widget
   //cgi.Cmd_Execute( EXEC_NOW, "globalwidgetcommand dm_console activate" );

	//Yorvik
	cgi.Cvar_Set("cg_saybuf", "");
	cgi.Cmd_Execute(EXEC_NOW, "forcemenu ctf_say");	
	//Y
   }

//
//Yorvik - MessageMode2 (say_team)
//
void CG_MessageMode2_UI_f(void)
{
	//color the bits according to team
	//if we're not on a team, bail
	str pic = "";
	if(cg.snap->ps.stats[STAT_TEAM] == TEAM_RED)
		pic = "textures/ctf_menu/ingame/sayteam_title_red";
	else if(cg.snap->ps.stats[STAT_TEAM] == TEAM_BLUE)
		pic = "textures/ctf_menu/ingame/sayteam_title_blue";
	else
		return;

	//clear the say field
	cgi.Cvar_Set("cg_saybuf", "");

	//show the menu
	cgi.Cmd_Execute(EXEC_NOW, "forcemenu ctf_say_team");

	//apply color
	cgi.Cmd_Execute(EXEC_NOW, va("globalwidgetcommand say_team_title shader %s", pic.c_str()));
}

void CG_HudPrint_f
   (
   void
   )

   {
   char  string[MAX_STRING_CHARS];

   // Print the string to the hud
   Com_sprintf( string, sizeof( string ), "globalwidgetcommand dm_console print \"%s\"\n", cgi.Argv( 1 ) );
   cgi.Cmd_Execute( EXEC_NOW, string );
   }

void CG_StatusUI_f
   (
   void
   )

   {
   // Set the status string on the hud
   char  string[MAX_STRING_CHARS];

   // Print the string to the hud
   Com_sprintf( string, sizeof( string ), "globalwidgetcommand dm_status title \"%s\"\n", cgi.Argv( 1 ) );
   cgi.Cmd_Execute( EXEC_NOW, string );
   }

void CG_RefreshArenaUI_f
   (
   void
   )

   {
   int arena;

   arena = atoi( cgi.Argv( 1 ) );

   // Refresh the UI
   CG_JoinArenaUI_f();
   CG_JoinTeamUI( arena );
   }

void CG_JoinArenaUI_f
   (
   void
   )

   {
   int i,count;

   // Bring up the UI for joining an arena
   cgi.Cmd_Execute( EXEC_NOW, "forcemenu joinarena\n" );

   // Send the widget commands to the menu and build out all the choices
   count = atoi( CG_ConfigString( CS_NUM_ARENAS ) );

   if ( count > 0 )
      {
      cgi.Cmd_Execute( EXEC_NOW, "globalwidgetcommand dm_arenalist deleteallitems\n" );
      }

   for ( i=1; i<=count; i++ )
      {
      // add items to the dm_arenalist widget and set the command to join the arena
      cgi.Cmd_Execute( EXEC_NOW, va( "globalwidgetcommand dm_arenalist addconfigstringitem %d \"join_arena %d\"\n", CS_ARENA_INFO+i, i ) );
      }
   }

void CG_JoinTeamUI
   (
   int arena_num
   )

   {
   int i,j,count;

   // Bring up the UI for joining/creating a team
   cgi.Cmd_Execute( EXEC_NOW, "forcemenu joinarena\n" );

   // Go through all the teams and list the ones that are in this arena.  Also 
   // provide the option for the player to create a team
   count = atoi( CG_ConfigString( CS_NUM_TEAMS ) );      

   cgi.Cmd_Execute( EXEC_NOW, "globalwidgetcommand dm_teamlist deleteallitems\n" );
   
   if ( ( arena_num != 0 ) && ( count == 0 ) )
      {
      cgi.Cmd_Execute( EXEC_NOW, "globalwidgetcommand dm_teamlist additem \"No Teams In Arena\"" );
      }
   else if ( count > 0 )
      {
      for( i=1; i<=count; i++ )
         {
         char  team_name[MAX_STRING_CHARS];
         int   team_number,n;

         team_name[0] = 0;

         cgi.Cmd_TokenizeString( CG_ConfigString( CS_TEAM_INFO + i ) );

         // Get the arena number and team_number
         n           = atoi( cgi.Argv( 0 ) );
         team_number = atoi( cgi.Argv( 1 ) );

         for ( j=2; j<cgi.Argc(); j++ )
            {
            if ( ( strlen( cgi.Argv( j ) ) + strlen( team_name ) ) > MAX_STRING_CHARS-1 )
               break;

            strcat( team_name, cgi.Argv( j ) );
            strcat( team_name, " " );
            }

         // Only display this team if the arena numbers match
         if ( arena_num == n )
            {
            cgi.Cmd_Execute( EXEC_NOW, va( "globalwidgetcommand dm_teamlist additem \"%s\" \"join_team %d; popmenu 0\"\n", team_name, team_number ) );
            }
         }
      }

   if ( arena_num != 0 )
      {
      // Create the item that lets the player create a team
      cgi.Cmd_Execute( EXEC_NOW, "globalwidgetcommand dm_teamlist additem \"\"\n" );
      cgi.Cmd_Execute( EXEC_NOW, "globalwidgetcommand dm_teamlist additem \"Create New Team\" \"create_team; popmenu 0\"\n" );
      }

   // Set the refresh button command so it works properly
   cgi.Cmd_Execute( EXEC_NOW, va( "globalwidgetcommand dm_refresh stuffcommand \"cg_refresharena_ui %d\"\n", arena_num ) );   
   }

void CG_InitArenaUI_f
	(
	void
	)

   {  
   // Initialize the arena UI

   // Load up the arena hud and add it to the display list
   cgi.Cmd_Execute( EXEC_NOW, "ui_addhud arenahud\n" );
   }

//Wilka
void CG_InitCTF_UI_f(void)
{
	//Yorvik - dont show ctf hud - it's client drawn now
	//cgi.Cmd_Execute(EXEC_NOW, "ui_addhud ctf_hud\n");
	//cgi.Cmd_Execute(EXEC_NOW, "ui_addhud ctf_hud_teamscore\n");
	//Y
}
//W

//
//Yorvik
//show host game menu
//
void CG_ShowHostGameMenu_UI_f(void)
{
	cgi.Cmd_Execute(EXEC_NOW, "forcemenu ctf_hostgame");

	cgi.Cmd_Execute(EXEC_NOW, "globalwidgetcommand HostMenuMapList deleteallitems");

	//populate map list
	//TODO: get an actual list of maps from dirs AND pk3's
	cgi.Cmd_Execute(EXEC_NOW, "globalwidgetcommand HostMenuMapList additem test");
}

//
//show join team menu
//
void CG_JoinTeamMenu(void)
{
	cgi.Cmd_Execute(EXEC_NOW, "forcemenu ctf_teamjoin");
}

//
//Y
//