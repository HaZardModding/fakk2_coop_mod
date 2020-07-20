#include "level.h"
#include "scriptmaster.h"
#include "navigate.h"
#include "gravpath.h"
#include "g_spawn.h"
#include "player.h"
#include "characterstate.h"
#include "dm_manager.h"
#include "ctf_manager.h"

Level level;

CLASS_DECLARATION( Class, Level, NULL )
	{
		{ NULL, NULL }
	};

Level::Level() : preserveCurrentMapCycle(false) //Yorvik
   {
   Init();
   }

void Level::Init
   (
   void
   )

   {
   spawn_entnum = -1;

   restart = qfalse;;

   framenum = 0;
   time     = 0;
   frametime = 0;

   level_name = "";
   mapname = "";
   spawnpoint = "";
   nextmap = "";

   playerfrozen = false;
   intermissiontime = 0;
	exitintermission = 0;

   next_edict = NULL;

   total_secrets = 0;
	found_secrets = 0;

   memset( &impact_trace, 0, sizeof( impact_trace ) );

   earthquake = 0;

   cinematic = false;
	ai_on = true;

	mission_failed = false;
	died_already = false;
   near_exit = false;

   water_color = vec_zero;
   lava_color  = vec_zero;
   water_alpha = lava_alpha = 0;

	saved_soundtrack = "";
	current_soundtrack = "";

   consoleThread = NULL;

   // clear out automatic cameras
   automatic_cameras.ClearObjectList();

   // init level script variables
	levelVars.ClearList();

   m_fade_time_start             = 0;
   m_fade_time                   = -1;
   m_fade_color                  = vec_zero;
   m_fade_alpha                  = 0;
   m_fade_style                  = additive;   
   m_fade_type                   = fadein;
   m_letterbox_fraction          = 0;
   m_letterbox_time              = -1;
   m_letterbox_time_start        = 0;
   m_letterbox_dir               = letterbox_out;

   // Deathmatch stuff
   m_numArenas                   = 1;

   //Yorvik
   suddenDeath = false;
   //Y
   }

void Level::CleanUp
   (
   void
   )

   {
   if ( g_gametype->integer == GT_ARENA )
      {
      dmManager.Reset();
      }
   //Yorvik
   else if(g_gametype->integer == GT_CTF)
   {
	   GetCTFManager().Reset();
   }
   //Y

   ClearCachedStatemaps();

	PathManager.SavePaths();

	assert( active_edicts.next );
	assert( active_edicts.next->prev == &active_edicts );
	assert( active_edicts.prev );
	assert( active_edicts.prev->next == &active_edicts );
	assert( free_edicts.next );
	assert( free_edicts.next->prev == &free_edicts );
	assert( free_edicts.prev );
	assert( free_edicts.prev->next == &free_edicts );

	while( active_edicts.next != &active_edicts )
		{
		assert( active_edicts.next != &free_edicts );
		assert( active_edicts.prev != &free_edicts );

		assert( active_edicts.next );
		assert( active_edicts.next->prev == &active_edicts );
		assert( active_edicts.prev );
		assert( active_edicts.prev->next == &active_edicts );
		assert( free_edicts.next );
		assert( free_edicts.next->prev == &free_edicts );
		assert( free_edicts.prev );
		assert( free_edicts.prev->next == &free_edicts );

		if ( active_edicts.next->entity )
			{
			delete active_edicts.next->entity;
			}
		else
			{
			FreeEdict( active_edicts.next );
			}
		}
   earthquake = 0;

   cinematic = false;
	ai_on = true;

	mission_failed = false;
	died_already = false;
   near_exit = false;

   globals.num_entities = game.maxclients + 1;

   // clear up all AI node information
   AI_ResetNodes();

   // Reset the gravity paths
   gravPathManager.Reset();

   if ( consoleThread )
      {
      Director.KillThread( consoleThread->ThreadNum() );
      consoleThread = NULL;
      }

   // close all the scripts
	Director.CloseScript();

   // invalidate player readiness
   Director.PlayerNotReady();

   // clear out automatic cameras
   automatic_cameras.ClearObjectList();

   // clear out level script variables
	levelVars.ClearList();
   
   // Clear out parm vars
   parmVars.ClearList();

	// initialize the game variables
   // these get restored by the persistant data, so we can safely clear them here
	gameVars.ClearList();

   // clearout any waiting events
  	L_ClearEventList();

   ResetEdicts();

	// Reset the boss health cvar
	gi.cvar_set( "bosshealth", "0" );

	//Yorvik
	suddenDeath = false;
	//Y

   }

/*
==============
ResetEdicts
==============
*/
void Level::ResetEdicts
   (
   void
   )

   {
   int i;

   memset( g_entities, 0, game.maxentities * sizeof( g_entities[ 0 ] ) );

	// Add all the edicts to the free list
	LL_Reset( &free_edicts, next, prev );
	LL_Reset( &active_edicts, next, prev );
	for( i = 0; i < game.maxentities; i++ )
		{
      LL_Add( &free_edicts, &g_entities[ i ], next, prev );
		}

	for( i = 0; i < game.maxclients; i++ )
		{
     	// set client fields on player ents
      g_entities[ i ].client = game.clients + i;

		G_InitClientPersistant (&game.clients[i]);
		}

   globals.num_entities = game.maxclients;
   }

/*
==============
Start

Does all post-spawning setup.  This is NOT called for savegames.
==============
*/
void Level::Start
   (
   void
   )

   {
   ScriptThread *gamescript;
   const char   *scriptname;

   // initialize secrets
   levelVars.SetVariable( "total_secrets", total_secrets );
   levelVars.SetVariable( "found_secrets", found_secrets );

	FindTeams();


	//Yorvik - set nextmap cvar
	if(g_gametype->integer == GT_CTF)
	{
		cvar_t* cvarNextmap = gi.cvar("nextmap", "", 0);

		if(!preserveCurrentMapCycle && strlen(cvarNextmap->string) <= 0)
		{
			if(this->nextmap.length() > 0)
				gi.cvar_set("nextmap", va("map %s", this->nextmap.c_str()));
			else
				gi.cvar_set("nextmap", va("map %s", this->mapname.c_str()));
		}

		//reset to false for next map change
		preserveCurrentMapCycle = false;

		suddenDeath = false;
	}
	//Y

   // call the precache scripts
   Precache();

   //
   // create the console thread
   //
   consoleThread = Director.CreateThread( CONSOLE_SCRIPT );

   //
   // start executing the game script
   //
	scriptname = ScriptLib.GetGameScript();
	if ( scriptname && scriptname[ 0 ] )
		{
   	gamescript = Director.CreateThread( scriptname, LEVEL_SCRIPT );
		if ( gamescript )
			{
			// start at the end of this frame
			gamescript->Start( 0 );
			}
		}
   }

qboolean Level::inhibitEntity
   (
   int spawnflags
   )

   {
   if ( !developer->integer && ( spawnflags & SPAWNFLAG_DEVELOPMENT ) )
      {
      return qtrue;
      }

   if ( !detail->integer && ( spawnflags & SPAWNFLAG_DETAIL ) )
      {
      return qtrue;
		}

   if ( DM_ACTIVE )
		{
		if ( spawnflags & SPAWNFLAG_NOT_DEATHMATCH )
			{
         return qtrue;
			}

      return qfalse;
		}

   switch( skill->integer )
      {
      case 0 :
         return ( spawnflags & SPAWNFLAG_NOT_EASY ) != 0;
         break;

      case 1 :
         return ( spawnflags & SPAWNFLAG_NOT_MEDIUM ) != 0;
         break;

      case 2 :
      case 3 :
         return ( spawnflags & SPAWNFLAG_NOT_HARD );
         break;
      }

   return qfalse;
   }

void Level::setSkill
   (
   int value
   )

   {
	int skill_level;

	skill_level = floor( value );
   skill_level = bound( skill_level, 0, 3 );

   gi.cvar_set( "skill", va( "%d", skill_level ) );

   gameVars.SetVariable( "skill", skill_level );
   }

void Level::setTime
   (
   int levelTime,
   int frameTime
   )

   {
   inttime        = levelTime;
   fixedframetime = 1.0f / sv_fps->value;
   time           = ( ( float )levelTime / 1000.0f );
   frametime      = ( ( float )frameTime / 1000.0f );
   }

/*
==============
SpawnEntities

Creates a server's entity / program execution context by
parsing textual entity definitions out of an ent file.
==============
*/
void Level::SpawnEntities
	(
	const char *themapname,
	const char *entities,
   int         levelTime
	)

	{
	int			inhibit,count=0;
	const char	*value;
   SpawnArgs   args;
   char        *spawnpos;

   // Init the level variables
   Init();

   spawnpos = strchr( themapname, '$' );
   if ( spawnpos )
      {
      mapname = str( themapname, 0, spawnpos - themapname );
      spawnpoint = spawnpos + 1;
      }
   else
      {
      mapname = themapname;
      spawnpoint = "";
      }

   // set up time so functions still have valid times
   setTime( levelTime, 1000 / 20 );

   if ( !LoadingServer )
      {
      // Get rid of anything left over from the last level
      CleanUp();

      // Set up for a new map
   	PathManager.Init( mapname );
      }

   setSkill( skill->integer );

	// reset out count of the number of game traces
	sv_numtraces = 0;

   // parse world
   entities = args.Parse( entities ); 
	spawn_entnum = ENTITYNUM_WORLD;
   args.Spawn();

   // parse ents
	inhibit = 0;
   for( entities = args.Parse( entities ); entities != NULL; entities = args.Parse( entities ) )
		{
		// remove things (except the world) from different skill levels or deathmatch
      spawnflags = 0;
		value = args.getArg( "spawnflags" );
		if ( value )
         {
         spawnflags = atoi( value );
         if ( inhibitEntity( spawnflags ) )
            {
			   inhibit++;
			   continue;
			   }
         }

		args.Spawn();
      count++;
		}

   gi.DPrintf( "%i entities spawned\n", count );
   gi.DPrintf( "%i entities inhibited\n", inhibit );
   
   // Process the spawn events
   L_ProcessPendingEvents();

   // After all the entities are spawned, init the dm arenas
   if ( g_gametype->integer == GT_ARENA )
      {
      dmManager.InitArenas();
      }

   //Yorvik - init the CTF_MANAGER
   else if(g_gametype->integer == GT_CTF)
   {
	   GetCTFManager().Init();
   }
   //Y

   if ( !LoadingServer || game.autosaved )
      {
      Start();
      }

   //
   // if this is a single player game, spawn the single player in now
   // this allows us to read persistant data into the player before the client
   // is completely ready
   //
   if ( game.maxclients == 1 )
      {
	   spawn_entnum = 0;
      new Player;
      }
	}

void Level::NewMap
	(
	const char *mapname,
	const char *entities,
   int         levelTime
	)

	{
   current_map = mapname;
   current_entities = entities;

   SpawnEntities( current_map, current_entities, levelTime );
   }

void Level::Restart
   (
   void
   )

   {
   SpawnEntities( current_map, current_entities, inttime );

   L_ProcessPendingEvents();

   G_ClientConnect( 0, qtrue );
   G_ClientBegin( &g_entities[ 0 ], NULL );
   }

void Level::PlayerRestart
   (
   void
   )

   {
   // we need to restart through the server code
   gi.SendConsoleCommand( "restart\n" );
   //restart = qtrue;
	level.mission_failed = false;
   }

void Level::Archive
	(
	Archiver &arc
	)

   {
   Class::Archive( arc );

   arc.ArchiveInteger( &framenum );
   arc.ArchiveInteger( &inttime );
   arc.ArchiveFloat( &time );
   arc.ArchiveFloat( &frametime );
   arc.ArchiveFloat( &fixedframetime );
   arc.ArchiveInteger( &startTime );
                     
   arc.ArchiveString( &level_name );
   arc.ArchiveString( &mapname );
   arc.ArchiveString( &spawnpoint );
   arc.ArchiveString( &nextmap );

   arc.ArchiveBoolean( &restart );

   arc.ArchiveBoolean( &playerfrozen );
   arc.ArchiveFloat( &intermissiontime );
	arc.ArchiveInteger( &exitintermission );

   arc.ArchiveInteger( &total_secrets );
   arc.ArchiveInteger( &found_secrets );

   arc.ArchiveFloat( &earthquake );
   arc.ArchiveFloat( &earthquake_magnitude );

   arc.ArchiveBoolean( &cinematic );
	arc.ArchiveBoolean( &ai_on );

	arc.ArchiveBoolean( &mission_failed );
	arc.ArchiveBoolean( &died_already );

   arc.ArchiveVector( &water_color );
   arc.ArchiveVector( &lava_color );
   arc.ArchiveFloat( &water_alpha );
   arc.ArchiveFloat( &lava_alpha );

   arc.ArchiveString( &saved_soundtrack );
	arc.ArchiveString( &current_soundtrack );

   arc.ArchiveVector( &m_fade_color );
   arc.ArchiveFloat( &m_fade_alpha );
   arc.ArchiveFloat( &m_fade_time );
   arc.ArchiveFloat( & m_fade_time_start );
   arc.ArchiveInteger( (int *)&m_fade_style );   
   arc.ArchiveInteger( (int *)&m_fade_type );

   arc.ArchiveFloat( &m_letterbox_fraction );
   arc.ArchiveFloat( &m_letterbox_time );
   arc.ArchiveFloat( &m_letterbox_time_start );
   arc.ArchiveInteger( (int *)&m_letterbox_dir );

   if ( arc.Loading() )
      {
		str temp_soundtrack;

		// Change the sound track to the one just loaded

		temp_soundtrack = saved_soundtrack;
		ChangeSoundtrack( current_soundtrack.c_str() );
		saved_soundtrack = temp_soundtrack;

      //
      // create the console thread
      //
      consoleThread = Director.CreateThread( CONSOLE_SCRIPT );
      // not archived since we can't save mid-frame
      next_edict = NULL;
      // not archived since we can't save mid-frame
      memset( &impact_trace, 0, sizeof( impact_trace ) );
      }
   }

/*
==============
Precache

Calls precache scripts
==============
*/
void Level::Precache
   (
   void
   )

   {
   str         filename;
   const char *scriptname;
   int i;

   //
   // load in global0-9.scr
   //
   for( i = 0; i < 100; i++ )
      {
      filename = va( "global/global%i.scr", i );
      if ( !G_LoadAndExecScript( filename.c_str(), NULL, qtrue ) )
         {
         break;
         }
      }

   //
   // load in precache0-9.scr
   //
   if ( precache->integer )
      {
      for( i = 0; i < 100; i++ )
         {
#ifdef PRE_RELEASE_DEMO
         filename = va( "global/precache_demo%i.scr", i );
#else
         filename = va( "global/precache%i.scr", i );
#endif
         if ( !G_LoadAndExecScript( filename.c_str(), NULL, qtrue ) )
            {
            break;
            }
         }
      }

   //
   // load in players0-9.scr
   //
   for( i = 0; i< 100; i++ )
      {
      filename = va( "global/players%i.scr", i );
      if ( !G_LoadAndExecScript( filename.c_str(), NULL, qtrue ) )
         {
         break;
         }
      }

   //
   // load in universal_script.scr
   //
   G_LoadAndExecScript( "global/universal_script.scr", "precache:", qtrue );

   //
   // precache for the game script
   //
	scriptname = ScriptLib.GetGameScript();
	if ( scriptname && scriptname[ 0 ] )
		{
      char stripped[ MAX_QPATH ];
      char precache_script[ MAX_QPATH ];

      COM_StripExtension( scriptname, stripped );
      Com_sprintf( precache_script, sizeof( precache_script ), "%s_precache.scr", stripped );
      G_LoadAndExecScript( precache_script, "precache:" );

      // also try to call the preache in the normal script itself
      G_LoadAndExecScript( scriptname, "precache:", qtrue );
      }
   }

/*
================
FindTeams

Chain together all entities with a matching team field.

All but the first will have the FL_TEAMSLAVE flag set.
All but the last will have the teamchain field set to the next one
================
*/
void Level::FindTeams
   (
   void
   )

	{
   gentity_t  *e;
   gentity_t  *e2;
   gentity_t  *next;
   gentity_t  *next2;
	Entity	*chain;
	Entity	*ent;
	Entity	*ent2;
	int		c;
	int		c2;

	c = 0;
	c2 = 0;

	for( e = active_edicts.next; e != &active_edicts; e = next )
		{
		assert( e );
		assert( e->inuse );
		assert( e->entity );

		next = e->next;

		ent = e->entity;
		if ( !ent->moveteam.length() )
			{
			continue;
			}

		if ( ent->flags & FL_TEAMSLAVE )
			{
			continue;
			}

		chain = ent;
		ent->teammaster = ent;
		c++;
		c2++;
		for( e2 = next; e2 != &active_edicts; e2 = next2 )
			{
			assert( e2 );
			assert( e2->inuse );
			assert( e2->entity );

			next2 = e2->next;

			ent2 = e2->entity;
			if ( !ent2->moveteam.length() )
				{
				continue;
				}

			if ( ent2->flags & FL_TEAMSLAVE )
				{
				continue;
				}

			if ( ent->moveteam == ent2->moveteam )
				{
				c2++;
				chain->teamchain = ent2;
				ent2->teammaster = ent;
				chain = ent2;
				ent2->flags |= FL_TEAMSLAVE;
				}
			}
		}

   gi.DPrintf( "%i teams with %i entities\n", c, c2 );
	}

/*
=================
AllocEdict

Either finds a free edict, or allocates a new one.
Try to avoid reusing an entity that was recently freed, because it
can cause the client to think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated
angles and bad trails.
=================
*/
gentity_t *Level::AllocEdict
	(
	Entity *ent
	)

	{
	int		   i;
   gentity_t   *edict;

	if ( spawn_entnum >= 0 )
		{
      edict = &g_entities[ spawn_entnum ];
      spawn_entnum = -1;

      assert( !edict->inuse && !edict->entity );

      // free up the entity pointer in case we took one that still exists
      if ( edict->inuse && edict->entity )
         {
         delete edict->entity;
         }
      }
	else
		{
      edict = &g_entities[ maxclients->integer ];
      for ( i = maxclients->integer; i < globals.num_entities; i++, edict++ )
		   {
		   // the first couple seconds of server time can involve a lot of
		   // freeing and allocating, so relax the replacement policy
		   if ( 
               !edict->inuse && 
               (
                  ( edict->freetime < ( 2 + startTime ) ) || 
                  ( time - edict->freetime > 0.5 ) 
               )
            )
			   {
            break;
			   }
		   }

      // allow two spots for none and world
	   if ( i == game.maxentities - 2 )
		   {
		   gi.Error( ERR_DROP, "Level::AllocEdict: no free edicts" );
		   }
      }

   assert( edict->next );
	assert( edict->prev );
	LL_Remove( edict, next, prev );
	InitEdict( edict );
	assert( active_edicts.next );
	assert( active_edicts.prev );
	LL_Add( &active_edicts, edict, next, prev );
	assert( edict->next );
	assert( edict->prev );

	assert( edict->next != &free_edicts );
	assert( edict->prev != &free_edicts );

   // Tell the server about our data since we just spawned something
   if ( ( edict->s.number < ENTITYNUM_WORLD ) && ( globals.num_entities <= edict->s.number ) )
      {
      globals.num_entities = edict->s.number + 1;
      gi.LocateGameData( g_entities, globals.num_entities, sizeof( gentity_t ), &game.clients[ 0 ].ps, sizeof( game.clients[ 0 ] ) );
      }

   edict->entity = ent;

	return edict;
	}

/*
=================
FreeEdict

Marks the edict as free
=================
*/
void Level::FreeEdict
	(
   gentity_t *ed
	)

	{
	gclient_t *client;

	assert( ed != &free_edicts );

	// unlink from world
	gi.unlinkentity ( ed );

	assert( ed->next );
	assert( ed->prev );

	if ( next_edict == ed )
		{
		next_edict = ed->next;
		}

	LL_Remove( ed, next, prev );

	assert( ed->next == ed );
	assert( ed->prev == ed );
	assert( free_edicts.next );
	assert( free_edicts.prev );

	client = ed->client;
	memset( ed, 0, sizeof( *ed ) );
	ed->client = client;
	ed->freetime = time;
	ed->inuse = false;
   ed->s.number = ed - g_entities;

	assert( free_edicts.next );
	assert( free_edicts.prev );

	LL_Add( &free_edicts, ed, next, prev );

	assert( ed->next );
	assert( ed->prev );
	}

void Level::InitEdict
	(
   gentity_t *e
	)

	{
   int i;

	e->inuse = true;
   e->s.number = e - g_entities;

   // make sure a default scale gets set
   e->s.scale = 1.0f;
   // make sure the default constantlight gets set, initalize to r 1.0, g 1.0, b 1.0, r 0
   e->s.constantLight = 0xffffff;
	e->s.renderfx |= RF_FRAMELERP;
	e->spawntime = time;
	e->s.frame = 0;

   for( i = 0; i < NUM_BONE_CONTROLLERS; i++ )
      {
      e->s.bone_tag[ i ] = -1;
      VectorClear( e->s.bone_angles[ i ] );
      EulerToQuat( e->s.bone_angles[ i ], e->s.bone_quat[ i ] );
      }
	}

void Level::AddAutomaticCamera
	(
   Camera *cam
	)

	{
   automatic_cameras.AddUniqueObject( cam );
   }
