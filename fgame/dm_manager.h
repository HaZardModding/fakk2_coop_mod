//-----------------------------------------------------------------------------
//
//  $Logfile:: /fakk2_code/fakk2_new/fgame/dm_manager.h                       $
// $Revision:: 12                                                             $
//   $Author:: Aldie                                                          $
//     $Date:: 9/20/00 6:58p                                                  $
//
// Copyright (C) 2000 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// $Log:: /fakk2_code/fakk2_new/fgame/dm_manager.h                            $
// 
// 12    9/20/00 6:58p Aldie
// Another temporary checkin for deathmatch stuff
// 
// 11    9/18/00 6:00p Aldie
// Periodic update for FakkArena deathmatch code.
// 
// 10    9/12/00 7:03p Aldie
// Complete overhaul of the deathmatch stuff I wrote 2 weeks ago.  More to
// come.  Intermediate checkin
// 
// 9     9/01/00 7:09p Aldie
// More fakk arena work
// 
// 8     8/30/00 6:35p Aldie
// More updates for deathmatch
// 
// 7     30.08.00 5:30p Ericf
// Added more stuff, almost ready for testing
// 
// 6     8/30/00 4:31p Aldie
// Adding more functionality to dm
// 
// 5     30.08.00 3:35p Ericf
// Another revision...
// 
// 4     30.08.00 3:26p Ericf
// Second Update
// 
// 3     30.08.00 3:15p Ericf
// Second Update
// 
// 2     30.08.00 11:38a Ericf
// First Version
//
// DESCRIPTION:
// 

#ifndef __DM_MANAGER__
#define __DM_MANAGER__

#include "g_local.h"
#include "queue.h"
#include "playerstart.h"

class Player;

class DM_Team : public Listener
   {   
   public:
      Container<Player *>     m_players;
      str                     m_teamname;
      int                     m_teamnumber;
      int                     m_index;
      int                     m_countdown;
      int                     m_teamwins;
      int                     m_wins_in_a_row;

      CLASS_PROTOTYPE( DM_Team );

                           DM_Team();

      virtual void         AddPlayer( Player *player );
      virtual void         RemovePlayer( Player *player );
      inline void          setName( str name ){ m_teamname = name; };
      inline str           getName( void ){ return m_teamname; };
      inline void          setNumber( int n ){ m_teamnumber = n; };
      inline int           getNumber( void ){ return m_teamnumber; };
      inline void          setIndex( int n ){ m_index = n; };
      inline int           getIndex( void ){ return m_index; };
      void                 TeamWin( void );
      void                 TeamLoss( void );
      virtual void         UpdateTeamStatus( void ); // Wilka - made virtual
   };

class SimpleAmmoType : public Class
   {
   public:
      str   type;
      int   amount;
   };

typedef SafePtr<DM_Team> DM_TeamPtr;
typedef SafePtr<Player> PlayerPtr;

class DM_Arena : public Listener
   {
   private:
      friend class DM_Manager;
      str			m_name;
      int			m_num_players_per_team;
      int			m_num_active_players;
      int			m_starting_health;
      int			m_starting_water;
      int			m_id;
      int         m_countdown;
      int         m_spawncounter;
      bool        m_fightInProgress;

      Container<Player *>					   m_players;
      Container<PlayerDeathmatchStart *>	m_spawnpoints;
      Container<DM_TeamPtr>               m_fightingTeams;
      Queue                               m_teamQueue;
      Container<DM_TeamPtr>               m_teams;
      Container<str>                      m_weaponList;
      Container<SimpleAmmoType *>         m_ammoList;

      void                 ResetArena( void );
      void                 SetName( Event *ev );
      void                 SetHealth( Event *ev );
      void                 SetWater( Event *ev );
      void                 SetPlayersPerTeam( Event *ev );
      void                 UpdateArenaInfo( void );
      void                 ActivatePlayers( void );
      void                 Countdown( Event *ev );
      void                 CountdownFinished( void );
      void                 CheckEndMatch( void );
      void                 EndMatch( Event *ev );
      void                 BeginMatch( void );
      bool                 IsTeamAlive( DM_TeamPtr team );
      void                 TeamForfeitMatch( DM_TeamPtr forfeitTeam );
      void                 AddWeapon( Event *ev );
      void                 AddAmmo( Event *ev );
      void                 GiveWeaponsAndAmmo( Player *player );
      void                 HUDPrintClientsInArena( str string );

   public:

      CLASS_PROTOTYPE( DM_Arena );
                           
                           DM_Arena();
                           ~DM_Arena();
      void                 setID( int id );
      inline int           getID( void ){ return m_id; };
      void                 AddPlayer( Player *player );
      void                 RemovePlayer( Player *player );
      Entity *             GetRandomSpawnpoint( bool useCounter = false );
      void                 CreateTeam( Player *creator );
      void                 LeaveTeam( Player *player );
      void                 JoinTeam( Player *player, int team_number );
      void                 RemoveTeam( DM_TeamPtr team );
      void                 ResetTeams( void );
      void                 CenterPrintClientsInArena( str string );
      void                 PlayerKilled( Player *player );
      int                  GetLinePosition( Player *player );
   };

class DM_Manager : public Listener
   {
   private:
      Container<Player *>     m_activePlayers;
      Container<DM_Arena *>   m_arenas;

   public:
      CLASS_PROTOTYPE( DM_Manager );
      
      DM_Manager();
      ~DM_Manager();

      void        AddPlayer( Player *player );

      void        JoinTeam( Player *player, teamtype_t teamtype );

      void        JoinArena( Player *player, int arena_id_num );
      void        LeaveArena( Player *player );

      void        CreateTeam( Player *creator );
      void        LeaveTeam( Player *player );
      void        JoinTeam( Player *player, int team_number );

      void        RemovePlayerFromAllArenas( Player *player );
      void        RebuildTeamConfigstrings( void );

      void        RemovePlayer( Player *player );
      void        PlayerKilled( Player *player );
      void        Countdown(Event *ev);
      void        Reset( void );
      
      void        InitArenas( void );
      void        ArenaCommand( Event *ev );
      void        ArenaPrint( Entity *ent, const char *txt );
   };

extern DM_Manager dmManager;

#endif // __DM_MANAGER__