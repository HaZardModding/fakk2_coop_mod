#ifndef CTF_TEAM_H_INCLUDED
#define CTF_TEAM_H_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "dm_manager.h"
#include "ctf_types.h"

#pragma warning(push, 1)
#include <vector>
#include <string>
#pragma warning(pop)

class CTF_PlayerStart;
class CTF_Flag;


enum EFlagStatus // TODO: move this enum to a file shared with the client
{
	FLAG_AT_BASE,  // Flag is at it's spawn point
	FLAG_ON_GROUND, // Flag is siting on the ground somwhere
	FLAG_STOLEN,	// Flag is in the hands of the enemy
};

class CTF_Team : public DM_Team  
{
public:
	explicit CTF_Team(teamtype_t team);
	virtual ~CTF_Team();

	// Sets up this team for the start of a game - must be called at the start of the map
	void Init();

	// Resets all inier bits - must be called at the end of the map
	void Reset();

	// Updates the status of the players on this team
	virtual void UpdateTeamStatus();

	// Returns the number of captures this team has
	int GetCaptureCount() const {return m_nCaptures;}

	// Adds a capture to this teams capture count
	void CaptureFlag();

	// prints 'pszMessage' to all members of this team
	void CenterPrint(const char* pszMessage);

	// prints 'pszMessage' to all members of this team
	void Print(const char* pszMessage);

	// Returns a random player spawn for this team
	CTF_PlayerStart* CTF_Team::GetRandomSpawn();

	// Returns the current flag satus
	EFlagStatus GetFlagStatus() const {return m_FlagSatus;}

	//Sets flag status
	void SetFlagStatus(EFlagStatus status) {m_FlagSatus = status;}

	teamtype_t GetTeamType() const {return m_Team;}

	int GetPlayerCount() const;

	// Returns the player that most recently joined this team, or NULL if the team
	// is empty
		Player* GetNewestPlayer();

	// Overrides from DM_Team so we can do extra setup stuff when player joins/leaves
	// this team
		virtual void AddPlayer(Player *player);
		virtual void RemovePlayer(Player *player);

private:

/////////////////////////////////////////////////////////////////////////////
// Data
	unsigned int m_nCaptures; // number of captures this team has

	teamtype_t m_Team; // The team we are

	SpawnPoints m_SpawnPoints; // our team spawn points

	EFlagStatus m_FlagSatus;
};




#endif // ~CTF_TEAM_H_INCLUDED
