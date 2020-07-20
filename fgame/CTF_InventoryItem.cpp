///////////////////////////////////////////////////////////////////////////////
//
//
// CTF InventoryItem implementation
//
// A ctf inv item remembers it's initial spawn location even after being dropped.
//
// -Yorvik
//
///////////////////////////////////////////////////////////////////////////////

#include "CTF_InventoryItem.h"

//
//message handling
//
CLASS_DECLARATION(InventoryItem, CTF_InventoryItem, NULL)
{
	{&EV_SetOrigin,	SetSpawnPosEvent},
	{NULL,			NULL			}
};

//
//constructor
//
CTF_InventoryItem::CTF_InventoryItem() :
	InventoryItem(),
	m_gotOrigin(false),
	m_spawnPos(0,0,0)
{
}

//
//SetSpawnPosEvent - called when EV_SetOrigin message is recieved
//
void CTF_InventoryItem::SetSpawnPosEvent(Event* ev)
{
	//let the entity do its stuff
	Entity::SetOrigin(ev);

	//only store this origin if we havent already
	if(!m_gotOrigin)
	{
		m_spawnPos = origin;		
		m_gotOrigin = true;
	}


	OnSpawn();
}

//
//GetSpawnPos
//
const Vector& CTF_InventoryItem::GetSpawnPos() const
{
	return m_spawnPos;
}

//
//InvalidateOrigin - sets the m_gotOrigin flag to false, so next time
//the SetOrigin event is called, the spawn point will take the value of
//the given vector
//
void CTF_InventoryItem::InvalidateOrigin()
{
	m_gotOrigin = false;
}


// Function:	ReturnToSpawnPos()
// Overview:	Returns 'this' to its spawn location
// 
void CTF_InventoryItem::ReturnToSpawnPos()
{
	// Move us back, and drop to the floor. Will this be ok for items that
	// are set 'no drop' in the map?
		NoLerpThisFrame();
		setOrigin(GetSpawnPos());
		ProcessEvent(EV_Item_DropToFloor);

	//we are back home now
		AtSpawnLocation(true);
}
// End of function 'ReturnToSpawnPos'




