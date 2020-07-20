// CTF_Team.cpp: implementation of the CTF_Team class.
//
//////////////////////////////////////////////////////////////////////

#include "CTF_Team.h"

#include "CTF_Manager.h"
#include "CTF_Global.h"
#include "CTF_PlayerStart.h"
#include "CTF_Flag.h"

#include "EntityClassIterator.h"
#include "Player.h"

#pragma warning(push, 1)
#include <algorithm>
#pragma warning(pop)

typedef EntityClassIterator<CTF_PlayerStart> CtfStartIter;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTF_Team::CTF_Team(teamtype_t team) 
				:	m_nCaptures(0), 
					m_Team(team),
					m_FlagSatus(FLAG_AT_BASE)
{
	assert(m_Team == TEAM_RED || m_Team == TEAM_BLUE);
}

CTF_Team::~CTF_Team()
{

}


// Sets up this team for the start of a game - must be called at the start of the map
void CTF_Team::Init()
{
	// Reset our status
		m_nCaptures = 0;

	// And build our list of spawn points
		const char* pszTeam = m_Team == TEAM_RED ? "info_player_red" : "info_player_blue";

	// correct team spawns
		std::copy(CtfStartIter(pszTeam), CtfStartIter(), std::back_inserter(m_SpawnPoints));

	// also add the deathmatch starts cos they are used by all teams (makes it easier for the mapper to place spawn points in the middle of the map)
		std::copy(CtfStartIter("info_player_deathmatch"), CtfStartIter(), std::back_inserter(m_SpawnPoints));
		std::copy(CtfStartIter("info_player_start"),      CtfStartIter(), std::back_inserter(m_SpawnPoints));
}

// Resets all inier bits - must be called at the end of the map
void CTF_Team::Reset()
{
	m_SpawnPoints.clear();
}


// Updates the status of the players on this team
void CTF_Team::UpdateTeamStatus()
{
//	const CTF_Team& red = *GetCTFManager().GetTeam(TEAM_RED);
//	const CTF_Team& blue = *GetCTFManager().GetTeam(TEAM_BLUE);

	for(int i=1; i<=m_players.NumObjects(); ++i)
	{
		Player *player = m_players.ObjectAt(i);

		assert(player);
		if (player)
		{
			//TODO: use this for team status updates
			//player->UpdateStatus("<--->");
		}
	}
}

// prints 'pszMessage' to all members of this team
void CTF_Team::CenterPrint(const char* pszMessage)
{
	for(int i=1; i<=m_players.NumObjects(); ++i)
	{
		if(Player *player = m_players.ObjectAt(i))
		{
			CTF::CenterPrint(player, pszMessage);
		}
	}
}

// prints 'pszMessage' to all members of this team
void CTF_Team::Print(const char* pszMessage)
{
	for(int i=1; i<=m_players.NumObjects(); ++i)
	{
		if(Player *player = m_players.ObjectAt(i))
		{
			gi.SendServerCommand(player->entnum, "print \"%s\"", pszMessage);
		}
	}
}

// Returns a random player spawn for this team
CTF_PlayerStart* CTF_Team::GetRandomSpawn()
{
	// If there aren't any , we have to bail
	if(m_SpawnPoints.empty())
	{
		gi.Error(ERR_DROP, "No team %s player spawn position'.  Can't spawn player.\n", 
			CTF::TeamName(m_Team, false).c_str());
		return NULL;
	}

	// Select a random start
	int selection = (G_Random() * m_SpawnPoints.size())-1;
	assert(selection >= 0 && selection < m_SpawnPoints.size());
	return m_SpawnPoints[selection];
}

void CTF_Team::CaptureFlag()
{
	++m_nCaptures;
}

int CTF_Team::GetPlayerCount() const
{
	return const_cast<CTF_Team&>(*this).m_players.NumObjects();
}

// Returns the player that most recently joined this team, or NULL if the team
// is empty
Player* CTF_Team::GetNewestPlayer()
{
	const int numPlayers = GetPlayerCount();
	if(numPlayers == 0)
		return NULL;

	// Start with the first player
	Player* newestPlayer = m_players.ObjectAt(1);

	// then check the rest of them to see if they are newer than this one
	for(int i=2; i<=numPlayers; ++i)
	{
		if(Player *player = m_players.ObjectAt(i))
		{
			// If the time this player joined is greater than the time our
			// current newestPlayer joined, 'player' is newer
			if(player->GetTeamJoinTime() > newestPlayer->GetTeamJoinTime())
			{
				newestPlayer = player;
			}
		}
	}

	return newestPlayer;
}

// Do the normal stuff, but also tell this player the time that it joined the team
void CTF_Team::AddPlayer(Player *player)
{
	assert(player);
	DM_Team::AddPlayer(player);

	player->SetTeamJoinTime(level.time);
}

// Do the normal stuff, but also clear the time it joined the team
void CTF_Team::RemovePlayer(Player *player)
{
	assert(player);
	DM_Team::RemovePlayer(player);

	player->SetTeamJoinTime(0);
}