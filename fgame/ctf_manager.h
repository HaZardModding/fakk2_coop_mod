///////////////////////////////////////////////////////////////////////////////
//
//
// Class for taking care of the CTF game related stuff, team managment etc.
//
//
///////////////////////////////////////////////////////////////////////////////
//
#ifndef CTF_MANAGER_H_INCLUDED
#define CTF_MANAGER_H_INCLUDED

#include "ctf_team.h"
#include "ctf_types.h"

class Player;
class CTF_Flag;

class CTF_Manager
{
public:
	CTF_Manager();

	// put pPlayer on the team 'team'
	void JoinTeam(Player* pPlayer, const teamtype_t team);

	// Remove pPlayer from their team
	void LeaveTeam(Player* pPlayer);

	// Spawns pPlayer at a ctf spawn point
	void SpawnPlayer(Player* pPlayer);

	// Returns a random player spawn for team 'team'
	Entity* GetRandomTeamSpawn(teamtype_t team);

	//flag events
	void FlagCapture(Player* player, CTF_Flag* pFlag);
	void FlagReturn(Player* player, CTF_Flag* pFlag);
	void FlagDrop(Player* player, CTF_Flag* pFlag);
	void FlagPickup(Player* player, CTF_Flag* pFlag);

	//Init & Reset
	void Init();
	void Reset();

	// Center prints pszMessage to all players on 'team'
	void CenterPrintTeam(teamtype_t team, const char* pszMessage) const;

	// Prints pszMessage to all players on 'team'
	void PrintTeam(teamtype_t team, const char* pszMessage) const;

	// Prints msg to the hud's of all players on 'team'
	void HudPrintTeam(teamtype_t team, const char* msg) const;

	// Returns a pointer to the team for 'team' - NULL if there is no team
	CTF_Team* GetTeam(teamtype_t team);

	// Returns the capture limit for the current match
	int GetCaptureLimit() const;

	// Makes 'team' win this match
	void TeamWin(teamtype_t team);

	//player access
	void	GetPlayers(teamtype_t team, std::vector<Player*>& destVector);

	//returns player carrying the 'flagteam' flag
	Player* GetFlagCarrier(teamtype_t flagteam);

	//player locations
	int GetPlayerLocation(Player* player);
	str GetLocationString(unsigned int index);

	//intermission bits
	void EnterIntermission();
	void ExitIntermission();
	bool Intermission()	const	{return m_intermission;}

	//per frame control
	void RunFrame();

	//level exiting
	void CheckExitRules();

private:

/////////////////////////////////////////////////////////////////////////////
// Data
	bool				m_intermission;
	float				m_intermissionTime;
	Camera*				m_intermissionCamera;

	CTF_Team			m_RedTeam;
	CTF_Team			m_BlueTeam;
	SpawnPoints			m_NoneTeamSpawnPoints;

	MapLocations		m_locations; //list of all target_location ents in map
};


//shorthand singleton access
CTF_Manager& GetCTFManager();


#endif // ~CTF_MANAGER_H_INCLUDED

