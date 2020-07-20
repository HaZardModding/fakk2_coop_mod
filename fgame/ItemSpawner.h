/*//
// misc_item_spawner
// keys:
//		items  : <ent names>    - a list of entities to spawn, separated by;
//		wait   : <seconds = 30> - time to wait between spawn
//		random : <seconds = 0>  - possible variance in 'wait' time. Ent will spawn inside wait +/- random
//
//////////////////////////////////////////////////////////////////////

#if !defined(ITEMSPAWNER_H_INCLUDED)
#define ITEMSPAWNER_H_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "entity.h"

#pragma warning(disable : 4786)
#pragma warning(push, 1)
#include <vector>
#include <string>
#pragma warning(pop)

extern Event EV_Misc_Spawn_Item;
extern Event EV_Misc_Spawn_Item_Picked_Up;

class ItemSpawner : public Entity
{
public:
	CLASS_PROTOTYPE(ItemSpawner);
	
	ItemSpawner();
	~ItemSpawner();

	void SpawnItem(Event* ev);
	void SpawnItemPickedUp(Event* ev);

	void SetItems(Event* ev);
	void SetWait(Event* ev);
	void SetRandom(Event* ev);
private:
	void SpawnItem();

	//builds m_itemVector from  pszItems
	void BuildList(const char* pszItems);

	//spawns a 'pszClassName' at our location
	void SpawnThing(const char* pszClassName);

	//returns a random item from m_itemVector
	std::string ItemSpawner::GetRandomItem();

	//returns the spawn time based on the key/value settings for this ent
	float GetSpawnTime();

//////////////////////////////////////////////////////////////////////
//data
	std::vector<std::string> m_itemVector;
	int m_iWait;
	int m_iRandom;
};

#endif // ~ITEMSPAWNER_H_INCLUDED*/
