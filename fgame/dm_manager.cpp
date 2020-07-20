//-----------------------------------------------------------------------------
//
//  $Logfile:: /fakk2_code/fakk2_new/fgame/dm_manager.cpp                     $
// $Revision:: 13                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 9/22/00 6:29p                                                  $
//
// Copyright (C) 2000 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// $Log:: /fakk2_code/fakk2_new/fgame/dm_manager.cpp                          $
// 
// 13    9/22/00 6:29p Aldie
// Fix for more than 1 team selection
// 
// 12    9/22/00 6:10p Aldie
// Skin support
// 
// 11    9/20/00 6:58p Aldie
// Another temporary checkin for deathmatch stuff
// 
// 10    9/18/00 6:00p Aldie
// Periodic update for FakkArena deathmatch code.
// 
// 9     9/12/00 7:03p Aldie
// Complete overhaul of the deathmatch stuff I wrote 2 weeks ago.  More to
// come.  Intermediate checkin
// 
// 8     9/01/00 7:09p Aldie
// More fakk arena work
// 
// 7     8/30/00 6:35p Aldie
// More updates for deathmatch
// 
// 6     30.08.00 5:30p Ericf
// Added more stuff, almost ready for testing
// 
// 5     8/30/00 4:31p Aldie
// Adding more functionality to dm
// 
// 4     30.08.00 3:36p Ericf
// Another Revision...
// 
// 3     30.08.00 3:15p Ericf
// Second Initial Update
// 
// 2     30.08.00 11:37a Ericf
// First Version
//
// DESCRIPTION:
// 

#include "player.h"
#include "dm_manager.h"


// Global DM Manager
DM_Manager dmManager;

//==========================================================================
//
// DM_Team Object
//
//==========================================================================
CLASS_DECLARATION( Listener, DM_Team, NULL )
   {
      { NULL, NULL }
   };

DM_Team::DM_Team
   (
   )

   {
   m_teamwins      = 0;
   m_wins_in_a_row = 0;
   m_index         = 0;
   m_teamnumber    = 0;
   }

void DM_Team::AddPlayer
   (
   Player *player 
   )

   {
   m_players.AddObject( player );
   }

void DM_Team::RemovePlayer
   (
   Player *player 
   )

   {
   m_players.RemoveObject( player );
   }

void DM_Team::UpdateTeamStatus
   (
   void
   )

   {
   int i;

   for ( i=1; i<=m_players.NumObjects(); i++ )
      {
      Player *player = m_players.ObjectAt( i );

      assert( player );
      if ( !player )
         {
         continue;
         }

      player->UpdateStatus( va( "%d wins (%d in a row)", m_teamwins, m_wins_in_a_row ) );
      }
   }

void DM_Team::TeamWin
   (
   void
   )

   {
   m_teamwins++;
   m_wins_in_a_row++;
   UpdateTeamStatus();
   }

void DM_Team::TeamLoss
   (
   void
   )

   {
   m_wins_in_a_row = 0;
   UpdateTeamStatus();
   }

//==========================================================================
// DM_Arena Object
//
// A DM_Arena is created for each arena that resides in a level.  The level designer
// can set the attributes of this arena using script commands
//==========================================================================
Event EV_Arena_SetName
   (
   "name",
   EV_DEFAULT,
   "s",
   "name",
   "set the name of the arena"
   );
Event EV_Arena_SetHealth
   (
   "starting_health",
   EV_DEFAULT,
   "i",
   "health",
   "set the starting health of each player"
   );
Event EV_Arena_SetWater
   (
   "starting_water",
   EV_DEFAULT,
   "i",
   "water",
   "set the starting water of each player"
   );
Event EV_Arena_SetPlayersPerTeam
   (
   "playersperteam",
   EV_DEFAULT,
   "i",
   "num",
   "set the number of players for each team"
   );
Event EV_Arena_Countdown
   (
   "countdown",
   EV_DEFAULT,
   NULL,
   NULL,
   "Start the countdown for the match"
   );
Event EV_Arena_EndMatch
   (
   "endmatch",
   EV_DEFAULT,
   NULL,
   NULL,
   "End the current match"
   );
Event EV_Arena_AddWeapon
   (
   "addweapon",
   EV_DEFAULT,
   "s",
   "weaponmodel",
   "Add this weapon to the list of weapons that will be used in this match"
   );
Event EV_Arena_AddAmmo
   (
   "addammo",
   EV_DEFAULT,
   "si",
   "ammotype amount",
   "Add this ammo to the players when they start the match"
   );

CLASS_DECLARATION( Listener, DM_Arena, NULL )
   {
      { &EV_Arena_SetName,                   SetName },
      { &EV_Arena_SetHealth,                 SetHealth },
      { &EV_Arena_SetWater,                  SetWater },
      { &EV_Arena_SetPlayersPerTeam,         SetPlayersPerTeam },
      { &EV_Arena_Countdown,                 Countdown },
      { &EV_Arena_EndMatch,                  EndMatch },
      { &EV_Arena_AddWeapon,                 AddWeapon },
      { &EV_Arena_AddAmmo,                   AddAmmo },
      { NULL, NULL }
   };

//=================================================================
//DM_Arena::CenterPrintClientsInArena - Centerprint the string to all
//clients in this arena
//=================================================================
void DM_Arena::CenterPrintClientsInArena
   (
   str string
   )

   {
   int i;
   
   for( i=1; i<=m_players.NumObjects(); i++ )
		{
      Player *player = m_players.ObjectAt( i );

      assert( player );
      if ( !player )
         continue;

      gi.centerprintf( player->edict, string );
      }
   }

//=================================================================
//DM_Arena::HUDPrintClientsInArena - HUDprint the string to all
//clients in this arena
//=================================================================
void DM_Arena::HUDPrintClientsInArena
   (
   str string
   )

   {
   int i;
   
   for( i=1; i<=m_players.NumObjects(); i++ )
		{
      Player *player = m_players.ObjectAt( i );

      assert( player );
      if ( !player )
         continue;

      player->HUDPrint( string );
      }
   }

//=================================================================
//DM_Arena::IsTeamAlive - Checks the team to see if any players are
//still alive.
//=================================================================
bool DM_Arena::IsTeamAlive
   (
   DM_TeamPtr team
   )

   {
   int i;

   for ( i=1; i<=team->m_players.NumObjects(); i++ )
      {
      Player *player;

      player = team->m_players.ObjectAt( i );      
      assert( player );
      if ( !player )
         {
         continue;
         }

      if ( !player->deadflag )
         {
         return true;
         }
      }
   return false;
   }

//=================================================================
//DM_Arena::EndMatch - Ends the current match.  Puts the players still
//in the arena into the non-fighting state so they can't use their weapons.
//Calls ResetTeams() to try and start another match
//=================================================================
void DM_Arena::EndMatch
   (
   Event *ev
   )

   {
   int i,j;
   
   m_fightInProgress = false;

   // Turn off fighting for any players that are still alive.
   for ( i=1; i <= m_fightingTeams.NumObjects(); i++ )
      {
      DM_TeamPtr team;

      team = m_fightingTeams.ObjectAt( i );

      assert( team );

      if ( !team )
         {
         return;
         }

      for ( j=1; j<=team->m_players.NumObjects(); j++ )
         {
         Player *player = team->m_players.ObjectAt( j );

         assert( player );
         if ( player )
            {
            player->EndFight();
            }
         }
      }

   ResetTeams();
   }

//=================================================================
//DM_Arena::ResetArena - Resets the arena.  This is called when 
//something bad or unexpected happens.
//=================================================================
void DM_Arena::ResetArena
   (
   void
   )

   {
   // Something bad happened reset the entire arena
   }

int DM_Arena::GetLinePosition
   (
   Player *player
   )

   {
   DM_Team *team;

   team = player->GetDM_Team();

   if ( team )
      {
      return m_teamQueue.IndexOfObject( team );
      }
   else
      {
      return 0;
      }
   }

//=================================================================
//DM_Arena::TeamForfeitMatch - The team has forfeited the match.  Usually
//happens when a player is the last one to leave the team.
//=================================================================
void DM_Arena::TeamForfeitMatch
   (
   DM_TeamPtr forfeitTeam
   )

   {
   DM_TeamPtr t1,t2;
   
   // There should be at least 2 teams in the list
   assert( m_fightingTeams.NumObjects() == 2 );
   if ( m_fightingTeams.NumObjects() != 2 )
      {
      warning( "DM_Arena::TeamForfeitMatch", "Invalid number of fighting teams\n" );
      return;
      }

   // This team has left the arena, so forfeit to the other team
   t1 = m_fightingTeams.ObjectAt( 1 );
   t2 = m_fightingTeams.ObjectAt( 2 );

   if ( forfeitTeam == t1 ) // Team 2 wins by forfeit
      {
      m_fightingTeams.RemoveObject( t1 );
      m_teamQueue.Enqueue( t1 );
      CenterPrintClientsInArena( va( "%s Wins by Forfeit", t2->getName().c_str() ) );            
      }
   else if ( forfeitTeam == t2 ) // Team 1 wins by forfeit
      {
      m_fightingTeams.RemoveObject( t2 );
      m_teamQueue.Enqueue( t2 );
      CenterPrintClientsInArena( va( "%s Wins by Forfeit", t1->getName().c_str() ) );
      }

   PostEvent( EV_Arena_EndMatch, 5 );
   }

//=================================================================
//DM_Arena::CheckEndMatch - Checks to see if a match has ended.  A win
//is defined as the other team being completely dead.
//=================================================================
void DM_Arena::CheckEndMatch
   (
   void
   )

   {
   DM_TeamPtr t1,t2;
   bool team1_alive = false;
   bool team2_alive = false;

   // The fight is over when one team is completely eliminated.
   assert( m_fightingTeams.NumObjects() == 2 );

   t1 = m_fightingTeams.ObjectAt( 1 );
   t2 = m_fightingTeams.ObjectAt( 2 );
   
   team1_alive = IsTeamAlive( t1 );
   team2_alive = IsTeamAlive( t2 );
   
   if ( team1_alive && team2_alive ) // Game still going on
      {
      return;
      }
   else if ( !team1_alive && !team2_alive ) // Draw
      {
      // Leave both teams on the active list and reset
      CenterPrintClientsInArena( "Match Draw\n" );
      }   
   else if ( !team1_alive && team2_alive ) // Team 2 wins      
      {
      // Remove team 1
      m_fightingTeams.RemoveObject( t1 );
      m_teamQueue.Enqueue( t1 );

      t1->TeamLoss();
      t2->TeamWin();
      CenterPrintClientsInArena( va( "%s Wins", t2->getName().c_str() ) );
      }
   else if ( team1_alive && !team2_alive ) // Team 1 wins
      {
      // Remove team 2
      m_fightingTeams.RemoveObject( t2 );
      m_teamQueue.Enqueue( t2 );

      t1->TeamWin();
      t2->TeamLoss();
      CenterPrintClientsInArena( va( "%s Wins", t1->getName().c_str() ) );
      }

   // Post the end of the match in 5 seconds, so the winning team can run around a bit
   PostEvent( EV_Arena_EndMatch, 5 );
   }

//=================================================================
//DM_Arena::PlayerKilled - When a player is killed, put him into spectator
//mode.  Also check to see if the fight is over.
//=================================================================
void DM_Arena::PlayerKilled
   (
   Player *player 
   )

   {
   // When a player is killed, put them into spectator mode and check to see if the fight is over.   
   assert( player );
   if ( !player )
      {
      return;
      }

   // Put into spectator mode after a few seconds
   player->PostEvent( EV_Player_Spectator, 3 );

   // Check to see if the fight is over
   CheckEndMatch();
   }

//=================================================================
//DM_Arena::CountdownFinished - When the fight countdown is finished,
//update all the players so that they can fight.
//=================================================================
void DM_Arena::CountdownFinished
   (
   void
   )

   {
   int i,j;

   CenterPrintClientsInArena( "FIGHT!" );

   for ( i=1; i <= m_fightingTeams.NumObjects(); i++ )
      {
      DM_TeamPtr team;

      team = m_fightingTeams.ObjectAt( i );

      assert( team );
      if ( !team )
         {
         return;
         }

      for ( j=1; j<=team->m_players.NumObjects(); j++ )
         {
         Player *player = team->m_players.ObjectAt( j );

         assert( player );
         if ( player )
            {
            player->BeginFight();
            }
         }
      }
   }

//=================================================================
//DM_Arena::Countdown - Start off a countdown for the match
//=================================================================
void DM_Arena::Countdown
   (
   Event *ev
   )

   {
   if ( m_countdown == 0 )
      {
      CountdownFinished();
      }
   else 
      {
      DM_TeamPtr t1, t2;

      if ( m_fightingTeams.NumObjects() != 2 )
         {
         // One of the teams left before the countdown was finished, 
         // so reset the teams.
         ResetTeams();
         return;
         }

      t1 = m_fightingTeams.ObjectAt( 1 );
      t2 = m_fightingTeams.ObjectAt( 2 );

      if ( t1 && t2 )
         {
         CenterPrintClientsInArena( va( "%s versus %s\n...%d...", 
                                    t1->getName().c_str(),
                                    t2->getName().c_str(),
                                    m_countdown
                                  )
                              );
         }

      // Repost the event
      m_countdown--;
      PostEvent( EV_Arena_Countdown, 1 );
      }
   }

//=================================================================
//DM_Arena::AddAmmo - Add amoo to the ammo list
//=================================================================
void DM_Arena::AddAmmo
   (
   Event *ev
   )

   {
   SimpleAmmoType  *ammo;

   ammo = new SimpleAmmoType;

   ammo->type   = ev->GetString( 1 );
   ammo->amount = ev->GetInteger( 2 );

   m_ammoList.AddObject( ammo );
   }

//=================================================================
//DM_Arena::AddWeapon - Add a weapon to the weapon list
//=================================================================
void DM_Arena::AddWeapon
   (
   Event *ev
   )

   {
   str string;

   string = ev->GetString( 1 );
   m_weaponList.AddObject( string );
   }

//=================================================================
//DM_Arena::GiveWeaponsAndAmmo - Give the specified weapons and ammo to the player
//=================================================================
void DM_Arena::GiveWeaponsAndAmmo
   (
   Player *player
   )

   {
   int i,count;
   // Give the player the appropriate weapons

   assert( player );
   if ( !player )
      {  
      warning( "DM_Arena::GiveWeapons", "NULL player specified.\n" );
      return;
      }

   count = m_weaponList.NumObjects();
   if ( !count )
      {
      // Give the default weapons
      }
   else
      {
      for ( i=1; i<=count; i++ )
         {
         Event *ev = new Event( "weapon" );
         ev->AddToken( m_weaponList.ObjectAt( i ) );
         player->ProcessEvent( ev );
         }
      }

   count = m_ammoList.NumObjects();
   if ( !count )
      {
      // Give the default ammo
      }
   else
      {
      for ( i=1; i<=count; i++ )
         {
         Event *ev = new Event( "ammo" );
         ev->AddString( m_ammoList.ObjectAt( i )->type );
         ev->AddInteger( m_ammoList.ObjectAt( i )->amount );
         player->ProcessEvent( ev );
         }
      }

   }

//=================================================================
//DM_Arena::ActivatePlayers - Take all of the fighting players and put
//them into the arena.
//=================================================================
#define NUM_SKINS 5
static char teamSkins[NUM_SKINS][32] = 
   {
   "models/julie.tik",
   "models/julie_battle.tik",
   "models/julie_leather.tik",
   "models/julie_swamp.tik",
   "models/julie_torn.tik"
   };

void DM_Arena::ActivatePlayers
   (
   void
   )

   {
   int i,j;
   DM_TeamPtr team;

   m_spawncounter = 1;

   // Take the active teams players and add spawn them into the arena
   for ( i=1; i<=m_fightingTeams.NumObjects(); i++ )
      {
      team = m_fightingTeams.ObjectAt( i );

      assert( team );
      if ( !team )
         continue;

      team->UpdateTeamStatus();
         
      for ( j=1; j<=team->m_players.NumObjects(); j++ )
         {
         Player *player = team->m_players.ObjectAt( j );

         assert( player );
         if ( !player )
            {
            continue;
            }
         
         player->WarpToPoint( this->GetRandomSpawnpoint( true ) );
         player->EnterArena( this->m_starting_health, this->m_starting_water );

         // If this is a 1 on 1 match, then let the players set their own skins, otherwise use the red and blue ones
         if ( m_num_players_per_team )
            {
            player->setModel( player->client->pers.dm_playermodel );
            }
         else
            {
            // Depending on the team index pick a skin
            player->setModel( teamSkins[ j%NUM_SKINS ] );
            }

         GiveWeaponsAndAmmo( player );
         }
      }
   }

//=================================================================
//DM_Arena::BeginMatch - Kick off a match.
//=================================================================
void DM_Arena::BeginMatch
   (
   void
   )

   {
   // Start the countdown for fighting
   m_fightInProgress = true;
   m_countdown       = 5;
   ProcessEvent( EV_Arena_Countdown );
   }

//=================================================================
//DM_Arena::ResetTeams: Resets the teams and adds players to 
//the active list.  Then call BeginFight if the teams are ready
//=================================================================
void DM_Arena::ResetTeams
   (
   void
   )

   {
   int i,j;

   // Only reset the teams if a fight is not in progress
   if ( m_fightInProgress )
      {
      return;
      }

   // Make sure there are 2 active teams in the list
   while ( m_fightingTeams.NumObjects() < 2 )
      {
      DM_TeamPtr team = ( DM_Team * )m_teamQueue.Dequeue();

      if ( team )
         {
         m_fightingTeams.AddObject( team );            
         }
      else
         {
         break;
         }
      }

   if ( m_fightingTeams.NumObjects() == 2 )
      {
      // Put the active teams into the arena
      ActivatePlayers();
      BeginMatch();
      }
   else
      {
      // Couldn't get a match started, so spectate the players that are waiting
      for ( i=1; i<=m_fightingTeams.NumObjects(); i++ )
         {
         DM_TeamPtr team = m_fightingTeams.ObjectAt( i );

         assert( team );
         if ( !team )
            {
            continue;
            }

         for ( j=1; j<=team->m_players.NumObjects(); j++ )
            {
            Player *player;

            player = team->m_players.ObjectAt( j );

            assert( player );
            if ( !player )
               {
               continue;
               }

            player->Spectator();
            player->UpdateStatus( "Waiting for Opponent" );
            }
         }
      }
   }

//=================================================================
//DM_Arena::JoinTeam: Join the specified team
//=================================================================
void DM_Arena::JoinTeam
   (
   Player *player,
   int team_number
   )

   {
   DM_TeamPtr team;

   assert( player );
   if ( !player )
      {
      return;
      }

   if ( ( team_number < 0 ) || ( team_number > m_teams.NumObjects() ) )
      {
      warning( "DM_Arena::JoinTeam", "Invalid team number\n" );
      return;
      }
   
   team = m_teams.ObjectAt( team_number );

   assert( team );
   if ( !team )
      {
      warning( "DM_Arena::JoinTeam", "Team not found in list\n" );
      return;
      }

   // Check to make sure that we can join this team based on the player limit for this arena
   if ( team->m_players.NumObjects() >= m_num_players_per_team )
      {
      return;
      }

   // Leave any other teams first
   if ( player->GetTeam() )
      {
      LeaveTeam( player );
      }

   team->AddPlayer( player );
   player->SetDM_Team( team );
   }

//=================================================================
//DM_Arena::RemoveTeam: Remove the team from the arena
//=================================================================
void DM_Arena::RemoveTeam
   (
   DM_TeamPtr team
   )

   {
   int i;

   // Team is about to be removed, check to see if this team is currently involved in a match         
   if ( m_fightingTeams.ObjectInList( team ) )
      {
      if ( ( m_fightingTeams.NumObjects() == 2 ) && ( m_fightInProgress ) )
         {
         TeamForfeitMatch( team );
         }
      m_fightingTeams.RemoveObject( team );
      }

   // Spectate all the members of the team
   for ( i=1; i<=team->m_players.NumObjects(); i++ )
      {
      Player *player;

      player = team->m_players.ObjectAt( i );

      assert( player );
      if ( !player )
         {
         continue;
         }

      player->Spectator();
      }

   // Remove team from queue and list
   if ( m_teamQueue.Inqueue( team ) )
      {
      m_teamQueue.Remove( team );
      }

   m_teams.RemoveObject( team );
   delete team;
   }

//=================================================================
//DM_Arena::LeaveTeam: Leave the current team that the player is on
//=================================================================
void DM_Arena::LeaveTeam
   (
   Player *player
   )

   {
   DM_TeamPtr team;

   assert( player );
   if ( !player )
      {
      return;
      }

   // Remove the player from the team
   team = player->GetDM_Team();

   if ( !team )
      {
      warning( "DM_Arena::LeaveTeam", "Could not find a team for this player\n" );
      return;
      }

   if ( m_teams.ObjectInList( team ) )
      {
      team->RemovePlayer( player );
      player->Spectator();

      // Check for empty team
      if ( !team->m_players.NumObjects() )
         {
         RemoveTeam( team );
         }
      }
   else
      {
      warning( "DM_Arena::LeaveTeam", "Could not find team in the arena\n" );
      return;
      }
   
   player->SetDM_Team( NULL );
   }

//=================================================================
//DM_Arena::CreateTeam: Creates a new team by the specified player
//=================================================================
void DM_Arena::CreateTeam
   (
   Player *player
   )

   {
   DM_TeamPtr team;

   // Create a new team based on the player's name
   if ( !player )
      {
      return;
      }

   team = new DM_Team;
   if ( !team )
      {
      warning( "DM_Arena::CreateTeam", "Could not create the team\n" );
      return;
      }

   // Set up the team
   team->setName( va( "%s's Team", player->client->pers.netname ) );
   team->AddPlayer( player );

   // Add it to the list of teams
   m_teams.AddObject( team );

   // Set the id number to the index in the list
   team->setNumber( m_teams.IndexOfObject( team ) );

   player->SetDM_Team( team );

   // Add this team to the queue
   m_teamQueue.Enqueue( team );
   }

//=================================================================
//DM_Arena::GetRandomSpawnpoint a spawnpoint in the arena. If
//useCounter is set, then the spawnpoints are generated sequentially
//=================================================================
Entity *DM_Arena::GetRandomSpawnpoint
   (
   bool useCounter
   )

   {
   Entity *spot=NULL;

   int numPoints = m_spawnpoints.NumObjects();
   
   if ( !numPoints )
      {
      warning( "DM_Arena::GetRandomSpawnpoint", "No spawnpoints found in arena\n" );
      return NULL;
      }
   else
      {
      if ( useCounter )
         {
         if ( m_spawncounter > numPoints )
            m_spawncounter = 1;

         spot = ( Entity * )m_spawnpoints.ObjectAt( m_spawncounter );
         m_spawncounter++;
         return spot;
         }
      else
         {
         int selection = ( G_Random() * numPoints ) + 1;
         spot = ( Entity * )m_spawnpoints.ObjectAt( selection );
         return spot;
         }
      }
   }

//=================================================================
//DM_Arena::SetName
//=================================================================
void DM_Arena::SetName
   (
   Event *ev
   )

   {
   m_name = ev->GetString( 1 );
   UpdateArenaInfo();
   }

//=================================================================
//DM_Arena::UpdateArenaInfo - Update the configstrings with all the 
//team names
//=================================================================
void DM_Arena::UpdateArenaInfo
   (
   void
   )

   {
   assert( m_id > 0 );

   if ( m_id > 0 )
      {
      str s;
      
      s = va( "#%d-%s-(%d) player(s)", m_id, m_name.c_str(), m_players.NumObjects() );
      gi.setConfigstring( CS_ARENA_INFO + m_id, s );
      }
   else
      {
      warning( "DM_Arena::SetName", "Arena id number is not set\n" );
      }
   }

//=================================================================
//DM_Arena::AddPlayer - Add a player to the list of active players in this arena
//=================================================================
void DM_Arena::AddPlayer
   (
   Player *player
   )

   {
   // Add a player to this arena
   m_players.AddObject( player );   
   
   player->SetDM_Arena( this );

   UpdateArenaInfo();
   HUDPrintClientsInArena( va( "%s joined the arena\n", player->client->pers.netname ) );
   }

//=================================================================
//DM_Arena::RemovePlayer - Remove the specified player from this arena
//=================================================================
void DM_Arena::RemovePlayer
   (
   Player *player
   )

   {
   int i,count;

   // Remove player from the list and print message to clients in this arena
   if ( m_players.ObjectInList( player ) )
      {
      m_players.RemoveObject( player );
      HUDPrintClientsInArena( va( "%s left the arena\n", player->client->pers.netname ) );
      }

   // Check to see if any teams have this player and remove that player
   count = m_teams.NumObjects();

   for ( i=count; i>0; i-- )
      {
      DM_TeamPtr team = m_teams.ObjectAt( i );

      if ( team->m_players.ObjectInList( player ) )
         {
         team->m_players.RemoveObject( player );

         // Removed the player, now try to remove the team
         if ( !team->m_players.NumObjects() )
            {
            RemoveTeam( team );
            }
         }
      }

   // Player is out of arena and out of any teams
   player->SetDM_Arena( NULL );
   player->SetDM_Team( NULL );

   // Update information and build the configstrings
   UpdateArenaInfo();
   dmManager.RebuildTeamConfigstrings();
   }

//=================================================================
//DM_Arena::SetNumPlayersPerTeam - Sets the number of players that can be on a single team. 
//=================================================================
void DM_Arena::SetPlayersPerTeam
   (
   Event *ev
   )

   {
   m_num_players_per_team = ev->GetInteger( 1 );
   }

//=================================================================
//DM_Arena::SetHealth - Set the starting health of the players
//=================================================================
void DM_Arena::SetHealth
   (
   Event *ev
   )

   {
   m_starting_health = ev->GetInteger( 1 );
   }

//=================================================================
//DM_Arena::SetWater - Set the starting water of the players
//=================================================================
void DM_Arena::SetWater
   (
   Event *ev
   )

   {
   m_starting_water = ev->GetInteger( 1 );
   }

//=================================================================
//DM_Arena::setID - set the ID number of the arena
//=================================================================
void DM_Arena::setID
   (
   int id
   )

   {
   m_id = id;

   // Find all the deathmatch starts that match this id number
   PlayerDeathmatchStart *start = NULL;

   // Count the number of deathmatch starts in this arena
   for( start = ( PlayerDeathmatchStart * )G_FindClass( start, "info_player_deathmatch" ); 
        start ; 
        start = ( PlayerDeathmatchStart * )G_FindClass( start, "info_player_deathmatch" ) )
      {
      // See if this spot matches this arena id number
      if ( start->arena == this->m_id )
         {
         m_spawnpoints.AddObject( start );
         }
      }
   }
//=================================================================
//DM_Arena::~DM_Arena
//=================================================================
DM_Arena::~DM_Arena
   (
   )

   {
   int i;

   // delete all the teams
   for ( i=m_teams.NumObjects(); i>0; i-- )
      {
      DM_Team *team;

      team = m_teams.ObjectAt( i );
      assert( team );

      if ( !team )
         continue;

      delete team;
      }
   }

//=================================================================
//DM_Arena::DM_Arena
//=================================================================
DM_Arena::DM_Arena
   (
   )

   {
   m_name                  = "Unnamed Arena";
   m_num_players_per_team  = 1;
   m_starting_health       = 100;
   m_starting_water        = 100;
   m_id                    = -1;
   m_countdown             = 5;
   m_fightInProgress       = false;
   }

CLASS_DECLARATION( Listener, DM_Manager, NULL )
	{
		{ NULL, NULL }
	};

//=================================================================
//DM_Manager::DM_Manager
//=================================================================
DM_Manager::DM_Manager
   (
   )

   {
   }

//=================================================================
//DM_Manager::~DM_Manager
//=================================================================
DM_Manager::~DM_Manager
   (
   )
   
   {
   Reset();
   }

//=================================================================
//DM_Manager::Reset - Delete all the arenas
//=================================================================
void DM_Manager::Reset
   (
   void
   )

   {
   int i;

   // Delete all the arenas and all the teams
   for ( i=m_arenas.NumObjects(); i>0; i-- )
      {
      DM_Arena *arena = m_arenas.ObjectAt( i );
      
      assert( arena );
      if ( ! arena )
         {
         continue;
         }

      m_arenas.RemoveObjectAt( i );
      delete arena;
      }
   }

//=================================================================
//DM_Manager::AddPlayer - Adds a player to the active players list
//=================================================================
void DM_Manager::AddPlayer
   (
   Player *player
   )

   {
   assert( player );

   if ( !player )
      {
      return;
      }

   m_activePlayers.AddObject( player );
   }

//=================================================================
//DM_Manager::RemovePlayer - Remove the player from the game
//=================================================================
void DM_Manager::RemovePlayer
   (
   Player *player
   )

   {
   assert( player );

   if ( !player )
      {
      return;
      }

   RemovePlayerFromAllArenas( player );
   m_activePlayers.RemoveObject( player );
   }

//=================================================================
//DM_Manager::RemovePlayerFromAllArenas - Remove the specified player from all arenas
//=================================================================
void DM_Manager::RemovePlayerFromAllArenas
   (
   Player *player
   )

   {
   int i;

   // Go through all the arenas and remove the player
   for ( i=1; i<=m_arenas.NumObjects(); i++ )
      {
      DM_Arena *arena = m_arenas.ObjectAt( i );

      assert( arena );
      if ( arena )
         {
         arena->RemovePlayer( player );
         }
      }
   }

//=================================================================
//DM_Manager::LeaveArena - Leave the arena and go back to the spectator mode
//=================================================================
void DM_Manager::LeaveArena
   (
   Player *player
   )

   {
   // Leave the arena
   RemovePlayerFromAllArenas( player );

   // Warp to the lobby
   player->InitArenaSpawnPoint();

    // Build the team list again for this player
   dmManager.RebuildTeamConfigstrings();
   player->RefreshArenaUI();
   }

//=================================================================
//DM_Manager::JoinArena - Join the specified arena
//=================================================================
void DM_Manager::JoinArena
   (
   Player *player,
   int arena_id_num
   )

   {
   DM_Arena *old_arena;

   // If this player is not already in the active player list, then add them in
   if ( !m_activePlayers.ObjectInList( player ) )
      {
      m_activePlayers.AddObject( player );
      }
   
   // Leave the old_arena
   old_arena = player->GetDM_Arena();
   if ( old_arena )
      {
      dmManager.LeaveArena( player );
      }

   // Make sure that the new arena exists
   if ( ( arena_id_num < 1 ) || ( arena_id_num > m_arenas.NumObjects() ) )
      {
      // This arena is a bad arena number
      warning( "DM_Manager::JoinArena", "Arena number (%d) is out of range\n", arena_id_num );
      return;
      }
   
   // Find the arena
   DM_Arena *arena = m_arenas.ObjectAt( arena_id_num );

   assert( arena );
   if ( !arena )
      {
      warning( "DM_Manager::JoinArena", "Arena number (%d) was not found\n" );
      return;
      }
   
   // Add player to the arena, spectator them, and warp them to the new point in that arena
   RemovePlayerFromAllArenas( player );

   arena->AddPlayer( player );
   player->Spectator();
   player->WarpToPoint( arena->GetRandomSpawnpoint() );
   player->UpdateStatus( "Spectating Arena" );

   // Build the team list again for this player
   dmManager.RebuildTeamConfigstrings();

   // Send a command to the player to bring up the team menu
   player->RefreshArenaUI();
   }

//=================================================================
//DM_Manager::JoinTeam - Joins a team
//=================================================================
void DM_Manager::JoinTeam
   ( 
   Player *player,
   int team_number
   )

   {   
   DM_Arena *arena;

   assert( player );
   if ( !player )
      {
      return;
      }

   // Try to join the specified team
   arena = player->GetDM_Arena();

   if ( !arena )
      {
      return;
      }

   arena->JoinTeam( player, team_number );
   RebuildTeamConfigstrings();   
   }

//=================================================================
//DM_Manager::PlayerKilled - A player is killed
//=================================================================
void DM_Manager::PlayerKilled
   ( 
   Player *player
   )

   {
   DM_Arena *arena;

   if ( !player )
      {
      return;
      }

   arena = player->GetDM_Arena();
   if ( arena )
      {
      arena->PlayerKilled( player );
      }
   }

//=================================================================
//DM_Manager::InitArenas - Initialize all the arenas
//=================================================================
void DM_Manager::InitArenas
   (
   void
   )

   {
   int i;

   if ( level.m_numArenas <= 0 )
      {
      gi.Error( ERR_DROP, "Invalid number of arenas specified: %d\n", level.m_numArenas );
      return;
      }

   // Set the number of arenas and the "No Arena" name 
   gi.setConfigstring( CS_NUM_ARENAS, va( "%d", level.m_numArenas ) );  
   gi.setConfigstring( CS_ARENA_INFO, "No Arena" );
   gi.setConfigstring( CS_TEAM_INFO, "No Team" );

   // Create all the arena data structures
   for ( i=0; i<level.m_numArenas; i++ )
      {
      DM_Arena *arena;

      arena = new DM_Arena;
      
      assert( arena );
      
      if ( !arena )
         {
         warning( "DM_Manager::InitArenas", "Could not allocate new arena\n" );
         continue;
         }

      // Set the id number of the arena based on the location in the list
      arena->setID( i+1 );

      // add the arena to the current arena list
      m_arenas.AddObject( arena );
      }
   }

//=================================================================
//DM_Manager::ArenaCommand - Process commands and send them to the specified arena
//=================================================================
void DM_Manager::ArenaCommand
   (
   Event *ev
   )

   {
   str         string;
   int         i,num;

   string = ev->GetString( 1 );

   if ( !stricmp( string, "all" ) )
      {
      num = m_arenas.NumObjects();

      for ( i=1; i<=num; i++ )
         {
         DM_Arena *arena = m_arenas.ObjectAt( num );
         
         assert( arena );         
         if ( !arena )
            {
            gi.Error( ERR_DROP, "Arena number %d was not found in the Arena Manager.\n", num );
            }

         // Create a new event without the first parm
         Event *ev2;
         int numargs = ev->NumArgs();
	      int argc = numargs - 2 + 1;

         ev2 = new Event( ev->GetToken( 2 ) );

	      for( i = 1; i < argc; i++ )
		      {
            ev2->AddToken( ev->GetToken( 2 + i ) );
		      }
         arena->ProcessEvent( ev2 );
         }
      }
   else
      {
      num = ev->GetInteger( 1 );
      // Bounds check the num to make sure the arena exists
      if ( num > m_arenas.NumObjects() )
         {
         gi.Error( ERR_DROP, "Arena number (%d) is greater than the total number of arenas in this level (%d)\n", num, m_arenas.NumObjects() );
         }

      DM_Arena *arena = m_arenas.ObjectAt( num );

      assert( arena );
      if ( !arena )
         {
         gi.Error( ERR_DROP, "Arena number %d was not found in the Arena Manager.\n", num );
         }
   
      // Create a new event without the first parm
      Event *ev2;
      int numargs = ev->NumArgs();
	   int argc = numargs - 2 + 1;

      ev2 = new Event( ev->GetToken( 2 ) );

	   for( i = 1; i < argc; i++ )
		   {
         ev2->AddToken( ev->GetToken( 2 + i ) );
		   }
      arena->ProcessEvent( ev2 );
      }
   }

//=================================================================
//DM_Manager::CreateTeam - Create a deathmatch team in the specified arena
//=================================================================
void DM_Manager::CreateTeam
   (
   Player *player
   )

   {
   DM_Arena *arena;

   assert( player );
   if ( !player )
      {
      return;
      }

   arena = player->GetDM_Arena();

   // Create a team in the specified arena   
   if ( !arena )
      {
      warning( "DM_Manager::CreateTeam", "player is not in an arena" );
      return;
      }
   
   arena->CreateTeam( player );
   RebuildTeamConfigstrings();
   
   // Check for team reset
   arena->ResetTeams();
   }

//=================================================================
//DM_Manager::LeaveTeam - Removes the player from the current team they are on
//=================================================================
void DM_Manager::LeaveTeam
   (
   Player *player
   )

   {
   DM_Arena *arena;

   assert( player );
   if ( !player )
      {
      return;
      }

   arena = player->GetDM_Arena();

   // Leave the team
   if ( !arena )
      {
      warning( "DM_Manager::LeaveTeam", "Player is not in an arena" );
      return;
      }
   
   arena->LeaveTeam( player );
   RebuildTeamConfigstrings();
   player->UpdateStatus( "Spectating Arena" );
   }

//=================================================================
//DM_Manager::RebuildTeamConfigstrings - Rebuild all the team configstrings
//=================================================================
void DM_Manager::RebuildTeamConfigstrings
   (
   void
   )

   {
   int      i,j;
   int      num_arenas;
   int      num_teams;
   int      index=1;
   int      teamcount=0;
   str      s;

   DM_Arena    *arena;
   DM_TeamPtr  team;

   // Go through all the arenas and set up the team names and numbers for the configstrings
   num_arenas = m_arenas.NumObjects();

   for ( i=1; i<=num_arenas; i++ )
      {
      arena = m_arenas.ObjectAt( i );

      assert( arena );
      if ( !arena )
         {
         continue;
         }

      num_teams = arena->m_teams.NumObjects();
      
      for ( j=1; j<=num_teams; j++ )
         {
         team = arena->m_teams.ObjectAt( j );

         assert( team );
         if ( !team )
            {
            continue;
            }
        
         s = va( "%d %d %s %d player(s)", arena->getID(), team->getNumber(), team->getName().c_str(), team->m_players.NumObjects() );

         gi.setConfigstring( CS_TEAM_INFO + index, s );
         team->setIndex( CS_TEAM_INFO + index );
         index++;
         teamcount++;
         }
      }

   gi.setConfigstring( CS_NUM_TEAMS, va( "%d", teamcount ) );
   }

//=================================================================
//DM_Manager::ArenaPrint - Print the string to the arena that the player is in
//=================================================================
void DM_Manager::ArenaPrint   
   (
   Entity *ent,
   const char *txt 
   )

   {
   assert( ent );

   if ( !ent )
      {
      warning( "DM_Manager::ArenaPrint", "Null entity\n" );
      return;
      }

   // Make sure this is a player
   if ( !ent->isSubclassOf( Player ) )
      {
      return;
      }

   Player *player = ( Player * )ent;
   DM_Arena *arena;

   arena = player->GetDM_Arena();
   if ( arena )
      {
      arena->HUDPrintClientsInArena( txt );
      }
   }
