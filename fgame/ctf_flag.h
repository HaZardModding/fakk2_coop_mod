//////////////////////////////////////////////////////////////////////
//
// CTF Flag classes - base class and one for each team. 
//
////////////////////////////////////////////////////////////////////////
#ifndef CTF_FLAG_H_INCLUDED
#define CTF_FLAG_H_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "ctf_inventoryitem.h"
#include "g_utils.h"

class Player;

/////////////////////////////////////////////////////////////////////////////
// CTF_Flag
class CTF_Flag : public CTF_InventoryItem
{
	CLASS_PROTOTYPE(CTF_Flag);
public:

	CTF_Flag();

	void CTF_DisplayPickup(Entity* ent = NULL);

	// Called when a player picks up this flag
	void Pickup(Event* ev);

	//Called when a flag needs to be returned to it's base (e.g. after drop/capture)
	void ReturnFlag(Event* ev);

	// Returns the team this flag belongs to
	teamtype_t GetTeam() const	{return m_Team;}

	// Sets the team for this flag
	void SetTeam(Event* ev);

	// pPlayer has just droped us
	void Droped(Player* pPlayer);

	//get the player who returned the flag JUST before a capture
	Player* GetReturner();

	void DropThink(Event* ev);

private:

	// Called when pPlayer picks up this flag, and is on the same team
	void PickupSameTeam(Player* pPlayer);

	// Called when pPlayer picks up this flag, and is on the enemy team
	void PickupDifferentTeam(Player* pPlayer);

	virtual void OnSpawn();

	

/////////////////////////////////////////////////////////////////////////////
// Data
	teamtype_t	m_Team;
	Player*		m_returner;		//player that last returned the flag
	float		m_returnTime;	//time the flag was last returned
};

#endif // ~CTF_FLAG_H_INCLUDED
