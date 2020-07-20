///////////////////////////////////////////////////////////////////////////////
//
//
// CTF General Bits
//
// -Yorvik
//
///////////////////////////////////////////////////////////////////////////////

#ifndef CTF_GLOBAL_H_DEFINED
#define CTF_GLOBAL_H_DEFINED

#include "g_local.h"
#include "g_utils.h"

#pragma warning(push, 1)
#include <string>
#pragma warning(pop)

class Player;
class CTF_Flag;
class CTF_Team;
class TeamIterator;

namespace CTF {

	enum reward_t
	{
		REWARD_ASSIST,
		REWARD_DEFENCE,
		REWARD_CAPTURE,

		REWARD_MAX
	};

//
//bonus's
//
	const int	BONUS_DEFEND			= 2;		//defence bonus
	const int	BONUS_CAPTURE_FLAG		= 5;		//capture flag
	const int	BONUS_DEFEND_BASE		= 1;		//defend base
	const int	BONUS_RETURN_FLAG		= 1;		//return flag
	const int	BONUS_PICKUP_FLAG		= 1;		//pickup flag
	const int	BONUS_KILL_FC			= 1;		//kill enemy FC
	const int	BONUS_ASSIST_RETURN		= 2;		//return your flag immediately before a cap
	const int	BONUS_KILL_TEAMMATE		= -1;		//kill someone on your team (BAD)

	const float	MAX_DEFENCE_BONUS_DIST	= 1500.0f;	//if you kill someone this far from the home position of your flag, you get a defence bonus
	const float MAX_ASSIST_TIME			= 10.0f;	//if you return your flag this many seconds before a capture, you get an assist bonus
	const float	MULTIKILL_AWARD_TIME	= 4.0f;		//if you kill one or more players in less than this time, you will get some feedback (EXCELLENT...)
	const float DEFEND_FC_TIME			= 5.0f;		//if you kill an enemy this many seconds after he hurt your fc, you will get a defence bonus


	extern const char* HUD_MSG_COLOR;
	extern const char* HUD_TEAMMSG_COLOR;

// Returns the string version of 'team'
	str	TeamName(teamtype_t team, bool colored = true);
	str OtherTeamName(teamtype_t team, bool colored = true);

//returns true if we are on a proper team (ie red or blue)
	bool OnTeam(Player* player);

//Kills all entities that would touch the proposed new positioning
//of ent if it were scaled to 'scale'.
//Ent should be unlinked before calling this!
	void ScaledKillBox(Entity* ent, float scale);
	bool ScaledKillBoxHurtsTeammate(Vector& origin, Player* player, float scale);

// Send info about people to given ent
	qboolean UpdateScores(gentity_t* ent);
	qboolean UpdateLocations(gentity_t* ent);
	void UpdateLocationsToAll(gentity_t* ent);
	void UpdateScoresToAll(gentity_t* ent);

// Returns the team in 'pszTeam'
	teamtype_t GetTeam(const char* pszTeam);

// prints the message to all players except 'exclude'
	void CenterPrint(Entity* ent, const char* text);
	void CenterPrintToAll(Entity* exclude, const char* text);

// prints a message on the scoreboard to ent
	void ScoreboardPrint(Entity* ent, const char* text, bool centerPrint = true);

//prints the message to all players hud's except 'exclude'
	void HudPrintToAll(Entity* exclude, const char* text);
	void HudPrint(Entity* ent, const char* text);

//prints the message to all players screens except 'exclude'
	void ClientPrint(Entity* ent, const char* msg, const char* x, const char* y, float scale = 1.0f, float life = 5.0f);
	void ClientPrintToAll(Entity* exclude, const char* msg, const char* x, const char* y, float scale = 1.0f, float life = 5.0f);

//prints a message to the hud's message window for all members of our team
	void TeamHudPrint(Player* sender, const char* msg, teamtype_t backupteam = TEAM_NONE);

//returns the 'opposite' team to the team passed in
	teamtype_t OtherTeam(teamtype_t team);
	CTF_Team&  OtherTeam(const CTF_Team& team);

//distance check between 2 vectors
	bool DistanceLess(const Vector& vec1, const Vector& vec2, float distance);

//calculate distance between 2 vectors
	float DistanceTo(const Vector& vec1, const Vector& vec2);

//returns the given strng + a white color flag
	const char* WhiteEnd(const char* s);

//returns given string sans color bits
	const char* TrimColor(const char* s);

//find the blue or red flag
	CTF_Flag* FindFlag(teamtype_t team);

//check interpissoin progress
	void CheckIntermissionProgress();

//replace common symbols in given string with current data
	str ConvertSymbolString(const char* text, Player* player);

//string functions
	void RemWhite(std::string& s);
	void RemChar(std::string& s, char ch);
	std::string Replace(std::string string, const std::string& oldstr, const std::string& newstr);
	std::string ToUpper(std::string string);

//load message of the day message from motd.txt
	void LoadMOTD();

//play a sound only a single player hears
	void LocalSound(Player* player, const char* sound);
	void LocalSoundToAll(Player* exception, const char* sound);

//gets player entnum for given name (case insensitive)
	int GetPlayerNum(const std::string& name, bool ignoreColor = false);

// Forces the teams to be balanced
	qboolean BalanceTeams(gentity_t* ent);

	CTF_Team& GetTeamMostNeedingANewPlayer();

//GetName
	const char* GetName(const Entity* ent);
	const char* GetName(Entity* ent);
	const char* GetName(gentity_t* ent);
	const char* GetName(TeamIterator& entIT);

// true if the teams are uneven
	bool TeamsAreUneven();

} //~namespace CTF

#endif