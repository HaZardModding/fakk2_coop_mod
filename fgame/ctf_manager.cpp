#include "ctf_manager.h"
#include "ctf_global.h"
#include "ctf_flag.h"
#include "ctf_PlayerStart.h"
#include "Player.h"
#include "TeamIterator.h"
#include "EntityClassIterator.h"
#include "CTF_TargetLocation.h"
#include "Vote.h"

typedef EntityClassIterator<CTF_PlayerStart>	CtfStartIter;
typedef EntityClassIterator<CTF_TargetLocation>	MapLocationIter;

//
//Events
//
extern  Event EV_CTF_Player_DeadStopAnim;

#pragma warning(push, 1)
#include <algorithm>
#include <string>
#pragma warning(pop)

namespace {
	const char RED_SKIN[] = "models/julie_leather.tik";
	const char BLUE_SKIN[] = "models/julie.tik";

	const float INTERMISSION_TIME = 10.0f;
}


CTF_Manager& GetCTFManager()
{
	static CTF_Manager instance;
	return instance;
}


/////////////////////////////////////////////////////////////////////////////
// CTF_Manager
/////////////////////////////////////////////////////////////////////////////

CTF_Manager::CTF_Manager() : 
	m_RedTeam(TEAM_RED),
	m_BlueTeam(TEAM_BLUE),
	m_intermission(false),
	m_intermissionTime(0.0f),
	m_intermissionCamera(NULL)
{
}


// put pPlayer on the team 'team'
void CTF_Manager::JoinTeam(Player* pPlayer, const teamtype_t team)
{
	//reset player - flag's & techs are dropped here
	pPlayer->CTF_Reset();

	CTF_Team* ctfteam = NULL;

	//joining red
	if(!g_teamForceBalance->integer && team == TEAM_RED)
	{
		ctfteam = &m_RedTeam;
	}
	
	//joining blue
	else if(!g_teamForceBalance->integer && team == TEAM_BLUE)
	{
		ctfteam = &m_BlueTeam;
	}
	
	//joining the team that needs the player most
	else
	{
		assert(team == TEAM_NONE);
		ctfteam = &CTF::GetTeamMostNeedingANewPlayer();
	}

	//add the player to the proper team
	ctfteam->AddPlayer(pPlayer);
	pPlayer->SetTeam(ctfteam->GetTeamType());

	// They're on a team now, so spawn them in the game
	SpawnPlayer(pPlayer);

	ctfteam->UpdateTeamStatus();
	
	//tell client that he was forced on to a team, to avoid confusion
	if(g_teamForceBalance->integer)
	{
		std::string forcedTeamStr = "forcing team to: ";
		forcedTeamStr += CTF::TeamName(ctfteam->GetTeamType(), true);
		CTF::HudPrint(pPlayer, forcedTeamStr.c_str());
	}

	//tell people about joinage
	CTF::CenterPrintToAll(pPlayer, va("%s joined the %s team", 
				CTF::GetName(pPlayer), CTF::TeamName(ctfteam->GetTeamType()).c_str()));
}

// Remove pPlayer from their team
void CTF_Manager::LeaveTeam(Player* pPlayer)
{
	// Remove the player from their team
		if(m_RedTeam.m_players.ObjectInList(pPlayer))
		{
			m_RedTeam.RemovePlayer(pPlayer);
		}
		else if(m_BlueTeam.m_players.ObjectInList(pPlayer))
		{
			m_BlueTeam.RemovePlayer(pPlayer);
		}
	
	// Make sure we update the players team
		pPlayer->SetTeam(TEAM_NONE);

	//remove crosshair
		pPlayer->CTF_RemoveCrosshair();
}


// Spawns pPlayer at a ctf spawn point
void CTF_Manager::SpawnPlayer(Player* pPlayer)
{
	teamtype_t team = pPlayer->GetTeam();

	//set model
		if(team == TEAM_RED)
			gi.cvar_set("g_playermodel", RED_SKIN);
		else
			gi.cvar_set("g_playermodel", BLUE_SKIN);

		pPlayer->InitModel();

	//force 3rd person mode
		gi.SendServerCommand(pPlayer->edict-g_entities, "stufftext \"set cg_3rd_person 1\"");

	//enter game
		pPlayer->WarpToPoint(GetRandomTeamSpawn(team));
		pPlayer->CTF_Init();

		pPlayer->UpdateStats();
}


// Returns a random player spawn for team 'team'
Entity* CTF_Manager::GetRandomTeamSpawn(teamtype_t team)
{
	//if we are on a team, spawn at one of our own spawn points
	if(CTF_Team* pTeam = GetTeam(team))
		return pTeam->GetRandomSpawn();
	
	//we are a spectator, so spawn at the intermission
	Entity* spawnPos = G_FindClass(NULL, "info_player_intermission");

	if(spawnPos)
		return spawnPos;

	//no intermission was found, so spawn at a random spawn point
	if(!m_NoneTeamSpawnPoints.empty())
	{
		spawnPos = m_NoneTeamSpawnPoints.at(rand() % m_NoneTeamSpawnPoints.size());
		
		if(spawnPos)
			return spawnPos;
	}

	//no player starts or deathmatch starts, so use a red or blue one
	spawnPos = m_RedTeam.GetRandomSpawn();
	if(!spawnPos)
		spawnPos = m_BlueTeam.GetRandomSpawn();

	return spawnPos;
}

//
//Init - called at level start
//
void CTF_Manager::Init()
{
	m_RedTeam.Init();
	m_BlueTeam.Init();

	// Copy pointers to all the none team spawns into m_NoneTeamSpawnPoints
	std::copy(CtfStartIter("info_player_start"), CtfStartIter(), std::back_inserter(m_NoneTeamSpawnPoints));
	std::copy(CtfStartIter("info_player_deathmatch"), CtfStartIter(), std::back_inserter(m_NoneTeamSpawnPoints));	

	//store map locations
	std::copy(MapLocationIter("ctf_target_location"), MapLocationIter(), std::back_inserter(m_locations));

	//init the map locations configstring
	str cs = va("count\\%d", m_locations.size());
	std::vector<CTF_TargetLocation*>::iterator it = m_locations.begin();

	for(int i=0; it != m_locations.end(); ++it, ++i)
		cs += va("\\loc%d\\%s", i, (*it)->GetLocationString().c_str());

	gi.setConfigstring(CS_MAPLOCATIONS, cs.c_str());

	//not in intermission
	m_intermission = false;
}

//
//Reset - called when level ends
//
void CTF_Manager::Reset()
{

	m_RedTeam.Reset();
	m_BlueTeam.Reset();

	// Get rid of the none team spawns
	m_NoneTeamSpawnPoints.clear();
}

//
//Flag Capture
//
void CTF_Manager::FlagCapture(Player* player, CTF_Flag* pFlag)
{
	CenterPrintTeam(TEAM_NONE, va("%s captured the %s flag!", 
		CTF::GetName(player), CTF::TeamName(pFlag->GetTeam()).c_str()));

	if( CTF_Team* pTeam = GetTeam(player->GetTeam()) )
	{
		pTeam->CaptureFlag();
	}
	else
	{
		assert(false); // A player with no team just captured the flag!
	}

	//reward player
	player->AddScore(CTF::BONUS_CAPTURE_FLAG);
	player->AddCaps(1);
	player->AddReward(CTF::REWARD_CAPTURE);
	
	//give assist bonus
	CTF_Flag* otherFlag = CTF::FindFlag(CTF::OtherTeam(pFlag->GetTeam()));
	if(otherFlag)
	{
		Player* assist = otherFlag->GetReturner();
		if(assist != NULL)
		{
			assist->AddScore(CTF::BONUS_ASSIST_RETURN);

			CTF::HudPrint(NULL, va("%s get's an assist bonus\n", CTF::GetName(assist)));

			//send reward
			assist->AddReward(CTF::REWARD_ASSIST);
		}
	}

	// This block is in the wrong place, it should go in CTF_Team
	// but we need to ask the flag for it's sound - or maybe it should be set 
	// in the map script?
		// Get the capture sound from the flag being captured
		str captureSound = pFlag->GetRandomAlias("snd_capture");

		if(captureSound.length() > 1)
		{
			//TODO: send a message to all clients telling them the flag was captured
			// then they can play the sound localy and do any other 'effects' instead of the
			// sound comming from the player that got the capture

			// Player a sound to this player at full volume
			player->Sound(captureSound, CHAN_BODY, -1, LEVEL_WIDE_MIN_DIST);
		}

	m_RedTeam.UpdateTeamStatus();
	m_BlueTeam.UpdateTeamStatus();


	if(level.suddenDeath)
	{
		TeamWin(player->GetTeam());
	}
}

//
//FlagReturn
//
void CTF_Manager::FlagReturn(Player* player, CTF_Flag* pFlag)
{
	if(player)
	{
		CTF::HudPrintToAll(NULL, va("%s returned the %s flag", 
			CTF::GetName(player), CTF::TeamName(pFlag->GetTeam()).c_str()));

		//reward player
			player->AddScore(CTF::BONUS_RETURN_FLAG);
	}
	else
	{
		CTF::HudPrintToAll(NULL, va("The %s flag has returned", 
			CTF::TeamName(pFlag->GetTeam()).c_str()));
	}

	//play a sound
		str snd = pFlag->GetRandomAlias("snd_return");

		if(snd .length() > 1)
			pFlag->Sound(snd, CHAN_BODY, -1, LEVEL_WIDE_MIN_DIST);
}

//
//FlagDrop
//
void CTF_Manager::FlagDrop(Player* player, CTF_Flag* pFlag)
{
	CTF::HudPrintToAll(NULL, va("%s dropped the %s flag\n", 
		CTF::GetName(player), CTF::TeamName(pFlag->GetTeam()).c_str()));

	//play a sound
}

//
//FlagPickup
//
void CTF_Manager::FlagPickup(Player* player, CTF_Flag* flag)
{
	CTF::CenterPrint(player, va(S_COLOR_WHITE "You got the %s flag!", 
		CTF::TeamName(flag->GetTeam()).c_str()));
	CTF::CenterPrintToAll(player, va(S_COLOR_WHITE "%s has the %s flag", 
		CTF::GetName(player), CTF::TeamName(flag->GetTeam()).c_str()));

	// Play the pickup sound - this should be sent as a message to the clients so the sound
	// is heard by all players
	str pickupSound = flag->GetRandomAlias( "snd_pickup" );

	if(pickupSound.length() > 1)
	{
		// Play the sound at full volume over the whole level
		player->Sound(pickupSound, CHAN_BODY, DEFAULT_VOL, LEVEL_WIDE_MIN_DIST);
	}

	player->AddScore(CTF::BONUS_PICKUP_FLAG);
}

// Center prints pszMessage to all players on 'team'
void CTF_Manager::CenterPrintTeam(teamtype_t team, const char* pszMessage) const
{
	// Iterate over all the players in 'team' and print 'pszMessage' to their screen
	// if team is TEAM_NONE it will iterate over all clients on the server
	TeamIterator end;
	TeamIterator iter(team);

	for(; iter != end; ++iter)
	{
		CTF::CenterPrint(&*iter, pszMessage);
	}
}

// Prints pszMessage to all players on 'team'
void CTF_Manager::PrintTeam(teamtype_t team, const char* pszMessage) const
{
	// Iterate over all the players in 'team' and print 'pszMessage' to their console
	// if team is TEAM_NONE it will iterate over all clients on the server
	TeamIterator end;
	TeamIterator iter(team);

	for(; iter != end; ++iter)
	{
		gi.SendServerCommand(iter->entnum, "print \"%s\"", pszMessage);
	}
}

//
//prints msg to the hud's of all players on 'team'
//
void CTF_Manager::HudPrintTeam(teamtype_t team, const char* msg) const
{
	// Iterate over all the players in 'team' and print 'msg' to their huds
	// if team is TEAM_NONE it will iterate over all clients on the server
	TeamIterator end;
	TeamIterator iter(team);

	for(; iter != end; ++iter)
	{
		gi.SendServerCommand(iter->entnum, "hudprint \"%s\"", msg);
	}
}


// Returns a pointer to the team for 'team' - NULL if there is no team
CTF_Team* CTF_Manager::GetTeam(teamtype_t team)
{
	switch(team)
	{
	case TEAM_RED:
		return &m_RedTeam;

	case TEAM_BLUE:
		return &m_BlueTeam;

	default:
		return NULL;
	}
}

// Returns the capture limit for the current match
int CTF_Manager::GetCaptureLimit() const
{
	return capturelimit->integer; 
}

// Makes 'team' win this match
void CTF_Manager::TeamWin(teamtype_t team)
{
	EnterIntermission();
}

//
//GetFlagCarrier - returns player carrying the 'flagteam' flag
//
Player* CTF_Manager::GetFlagCarrier(teamtype_t flagteam)
{
	TeamIterator end;
	TeamIterator iter(CTF::OtherTeam(flagteam));

	for(; iter != end; ++iter)
	{
		if(iter->GetFlag())
			return &*iter;
	}

	return NULL;
}

//
//returns index of closest ctf_target_location
//
int CTF_Manager::GetPlayerLocation(Player* player)
{
	float	bestDist	= 3*8192.0; //max distance for location ent to be valid
	int		best		= -1;

	for(MapLocations::iterator it = m_locations.begin(); it != m_locations.end(); ++it)
	{
		//ignore any locations not in our PVS
		if(!gi.inPVS(player->origin, (*it)->origin))
			continue;

		if(CTF::DistanceLess(player->origin, (*it)->origin, bestDist))
		{
			bestDist = CTF::DistanceTo(player->origin, (*it)->origin);
			best = it - m_locations.begin();
		}
	}

	return best;
}

//
//Put player in intermission state - dont use G_BeginIntermission cos it sux
//
void CTF_Manager::EnterIntermission()
{
	//find intermision ent
		Entity* intermission = G_FindClass(NULL, "info_player_intermission");

	//if no intermission was found, use a random start position
	if(!intermission)
	{
		teamtype_t team = (G_Random() >= 0.5) ? TEAM_BLUE : TEAM_RED;
		intermission = GetRandomTeamSpawn(team);
	}

	//make a camera
		m_intermissionCamera = new Camera;
		m_intermissionCamera->origin = intermission ? intermission->origin : Vector(0, 0, 0);
		m_intermissionCamera->angles = intermission ? intermission->angles : Vector(0, 0, 0);

	//make everyone use the new camera
		TeamIterator iter(TEAM_NONE);
		TeamIterator end;

		for(; iter != end; ++iter)
		{
			iter->deadflag = DEAD_DEAD;
			iter->health = -10;
			iter->hideModel();
			iter->SetCamera(m_intermissionCamera, 0);
			iter->CancelPendingEvents();
			iter->PostEvent(EV_CTF_Player_DeadStopAnim, FRAMETIME);
			iter->UpdateStats();

			gi.SendServerCommand(iter->edict-g_entities, "stufftext \"+scores\"");
		}

	//we are in intermission now
		if(!m_intermission)
			level.intermissiontime = level.time;

		m_intermission = true;
		
}

//
//ExitIntermission & switch to next map
//
void CTF_Manager::ExitIntermission()
{
	//cleanup intermission
		delete m_intermissionCamera;
		m_intermissionCamera = NULL;
		

	//reset player cameras
		TeamIterator iter(TEAM_NONE);
		TeamIterator end;

		for(; iter != end; ++iter)
		{
			iter->SetCamera(NULL, 0);
		}

	//switch to next map
		str nextmap = "";

	//check the map cycle cvar.
		//note: nextmap is assumed to be a ready-to-execute string (eg "map map1" - not just 
		//"map1"), this is for complex map cycle stuff like per map timelimit/caplimits etc
		cvar_t* nm = gi.cvar("nextmap", "", CVAR_SERVERINFO);
		nextmap = nm->string;
	
	//try the next map specified by the mapper
		if(nextmap.length() <= 0  && level.nextmap.length() > 0)
			nextmap = str("map ") + level.nextmap;

	//use current map as a last resort
		if(nextmap.length() <= 0)
			nextmap = str("map ") + level.mapname;
		
	//switch maps...
		gi.SendServerCommand(0, "stufftext \"%s\"", nextmap.c_str());
}

//
//Called once per server frame
//
void CTF_Manager::RunFrame()
{
	CTF::VoteSystem::Instance().Ping();
}

//
//check exit rules - ie check if any team has exceeded the caplimit
//
void CTF_Manager::CheckExitRules()
{
	//if in intermission - dont do this
	if(m_intermission)
		return;

	//timelimit
	if(level.suddenDeath)
	{
		if(level.time >= (timelimit->value*60 + g_suddenDeath->value*60.0f))
		{
			CTF::HudPrintToAll(NULL, "Sudden Death Elapsed.\n");
			EnterIntermission();
			return;
		}
	}
	else
	{
		if(timelimit->value)
		{
			if(level.time >= timelimit->value*60.0f)
			{
				CTF::HudPrintToAll(NULL, "Timelimit hit.\n");

				//enter sudden death mode
				if(g_suddenDeath->integer > 0 && m_RedTeam.GetCaptureCount() == m_BlueTeam.GetCaptureCount())
				{
					CTF::ClientPrintToAll(NULL, "SUDDEN DEATH", "c", "100", 2.5f, 3.0f);
					level.suddenDeath = true;
					return;
				}

				EnterIntermission();
				return;
			}
		}
	}

	// Has any team hit the limit?
	if(capturelimit->integer > 0)
	{
		teamtype_t team = TEAM_BLUE;

		for(int i=0; i<2; ++i)
		{
			if(GetTeam(team)->GetCaptureCount() >= capturelimit->integer)
			{
				CTF::HudPrintToAll(NULL, "Capturelimit hit.\n");
				EnterIntermission();
				return;
			}
			
			team = TEAM_RED;
		}

	}

	//fraglimit
	if(fraglimit->value)
	{
		TeamIterator iter(TEAM_NONE);
		TeamIterator end;

		for(; iter != end; ++iter)
		{
			if(iter->client->ps.stats[STAT_FRAGS] >= fraglimit->integer)
			{
				CTF::HudPrintToAll(NULL, "Fraglimit hit.\n");
				EnterIntermission();
				return;
			}
		}
	}		
}

//
//Return name of a location from our locations vector
//
str CTF_Manager::GetLocationString(unsigned int index)
{
	if(m_locations.empty())
		return "";

	if(index >= m_locations.size())
	{
		assert(false); // out of range
		return "unknown location";
	}

	return m_locations[index]->GetLocationString();
}