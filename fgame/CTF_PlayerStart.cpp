//
//
//CTF_PlayerStart.cpp
//
//Implementation for the player start entities (spawn points)
//
//
//-Yorvik (yorvik@ritualistic.com)
//
//

#include "CTF_PlayerStart.h"

//
//Base CTF Player Start
//
CLASS_DECLARATION(PlayerStart, CTF_PlayerStart, NULL)
{
	{NULL, NULL}
};

//Constructor
CTF_PlayerStart::CTF_PlayerStart() : m_team(TEAM_NONE)
{}

//
//SetTeam - Set's which team players must be to spawn here
//
void CTF_PlayerStart::SetTeam(teamtype_t team)
{
	m_team = team;
}

//
//GetTeam
//
teamtype_t CTF_PlayerStart::GetTeam() const
{
	return m_team;
}

//
//RED CTF Player Start
//
CLASS_DECLARATION(CTF_PlayerStart, CTF_PlayerStart_Red, "info_player_red")
{
	{NULL, NULL}
};

//Constructor
CTF_PlayerStart_Red::CTF_PlayerStart_Red()
{
	SetTeam(TEAM_RED);
}

//
//BLUE CTF Player Start
//
CLASS_DECLARATION(CTF_PlayerStart, CTF_PlayerStart_Blue, "info_player_blue")
{
	{NULL, NULL}
};

//Constructor
CTF_PlayerStart_Blue::CTF_PlayerStart_Blue()
{
	SetTeam(TEAM_BLUE);
}