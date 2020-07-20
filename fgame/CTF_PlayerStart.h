//
//
//CTF_PlayerStart.h
//
//Interfaces for the player start entities (spawn points)
//
//
//-Yorvik (yorvik@ritualistic.com)
//
//

#ifndef CTF_PLAYERSTART_H_DEFINED
#define CTF_PLAYERSTART_H_DEFINED

#include "PlayerStart.h"

//
//CTF_PlayerStart - base player start class
//
class CTF_PlayerStart : public PlayerStart  
{
private:
	CLASS_PROTOTYPE(CTF_PlayerStart);

	//the team we belong to
	teamtype_t m_team;
	
protected:
	void SetTeam(teamtype_t team);

public:
	CTF_PlayerStart();

	teamtype_t GetTeam() const;
};


//
//CTF_PlayerStart_Red
//
class CTF_PlayerStart_Red : public CTF_PlayerStart
{
	CLASS_PROTOTYPE(CTF_PlayerStart_Red);

public:
	CTF_PlayerStart_Red();
};

//
//CTF_PlayerStart_Blue
//
class CTF_PlayerStart_Blue : public CTF_PlayerStart
{
	CLASS_PROTOTYPE(CTF_PlayerStart_Blue);

public:
	CTF_PlayerStart_Blue();
};

#endif //CTF_PLAYERSTART_H_DEFINED