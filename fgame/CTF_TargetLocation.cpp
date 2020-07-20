///////////////////////////////////////////////////////////////////////////////
//
//
// target_location entity
// Used in ctf games to evaluate 'where' a player is
//
// -Yorvik
//
///////////////////////////////////////////////////////////////////////////////

#include "CTF_TargetLocation.h"

//
//Events
//
Event EV_CTF_SetTargetLocationString("message", EV_DEFAULT, "s", NULL, "'message' is the string displayed on the team overlay when a player is in this location");

//
//event handling
//
CLASS_DECLARATION(Entity, CTF_TargetLocation, "ctf_target_location")
{
	{&EV_CTF_SetTargetLocationString, SetLocationString},
	{NULL, NULL}
};

//
//Constructor
//
CTF_TargetLocation::CTF_TargetLocation() : m_location("unknown"), m_locationNum(-1)
{}

//
//Destructor
//
CTF_TargetLocation::~CTF_TargetLocation()
{}

//
//return location name
//
str CTF_TargetLocation::GetLocationString()
{
	return m_location;
}

//
//set location name
//
void CTF_TargetLocation::SetLocationString(Event* ev)
{
	if(!ev)
	{
		assert(false);
		return;
	}

	m_location = ev->GetString(1);
}