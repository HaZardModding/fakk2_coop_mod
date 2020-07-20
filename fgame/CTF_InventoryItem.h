///////////////////////////////////////////////////////////////////////////////
//
//
// CTF InventoryItem interface
//
// -Yorvik
//
///////////////////////////////////////////////////////////////////////////////

#if !defined(CTF_INVENTORYITEM_H_INCLUDED)
#define CTF_INVENTORYITEM_H_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "InventoryItem.h"



class CTF_InventoryItem : public InventoryItem
{
	CLASS_PROTOTYPE(CTF_InventoryItem);

public:
	CTF_InventoryItem();

	bool			IsAtSpawnLocation() const	{return m_atHome;}
	void			AtSpawnLocation(bool bHome) {m_atHome = bHome;}

	void			InvalidateOrigin();
	virtual void	SetSpawnPosEvent(Event* ev);
	const Vector&	GetSpawnPos() const;

	void			ReturnToSpawnPos();

private:
	virtual void	OnSpawn() {}


/////////////////////////////////////////////////////////////////////////////
// Data
	bool	m_gotOrigin;
	bool	m_atHome;
	Vector	m_spawnPos;
};



#endif // ~CTF_INVENTORYITEM_H_INCLUDED
