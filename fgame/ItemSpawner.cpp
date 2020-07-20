/*#include "g_local.h"
#include "ItemSpawner.h"
#include "Item.h"

Event EV_Misc_Spawn_Item( "spawm_item" );
Event EV_Misc_Spawn_Item_Picked_Up( "spawm_item_picked_up" );

Event EV_Misc_Spawn_SetItems(
   "items",
   EV_DEFAULT,
   "s",
   "items",
   "Set the items you want to be spawned from this spawner");

Event EV_Misc_Spawn_SetWait(
   "wait",
   EV_DEFAULT,
   "i",
   "wait",
   "Set the time to wait before spawning the next item when one is collected");
Event EV_Misc_Spawn_SetRandom(
   "random",
   EV_DEFAULT,
   "i",
   "random",
   "Set the ammount to +/- from 'wait' when spawning items");


CLASS_DECLARATION(Entity, ItemSpawner, "misc_item_spawner")
{
	{ &EV_Misc_Spawn_Item,				SpawnItem },
	{ &EV_Misc_Spawn_Item_Picked_Up,	SpawnItemPickedUp },
	{ &EV_Misc_Spawn_SetItems,			SetItems},
	{ &EV_Misc_Spawn_SetWait,			SetWait},
	{ &EV_Misc_Spawn_SetRandom,			SetRandom},
	{ NULL, NULL }
};


ItemSpawner::ItemSpawner()
{
	Event* ev = new Event(EV_Misc_Spawn_Item);

	// post an event to spawn something in a tiny amount of time, so 
	// that we are properly constructed before we try to do any spawning
	PostEvent(ev, 0.001f);
}
ItemSpawner::~ItemSpawner()
{
}

void ItemSpawner::SpawnItem(Event* ev)
{
	//pass this call in to the normal - none event SpawnItem func
	SpawnItem();
}

//this gets called by the dtor of the item, which gets called when it's picked up
void ItemSpawner::SpawnItemPickedUp(Event* ev)
{
	gi.DebugPrintf("ItemSpawner::SpawnItemPickedUp(Event* ev)\n");

	//create a new spawn event now the item has gone	
	Event* newEv = new Event(EV_Misc_Spawn_Item);

	//post a spawn event to us, getting a random from the range
	PostEvent(newEv, GetSpawnTime());
}

void ItemSpawner::SpawnItem()
{
	gi.DebugPrintf("ItemSpawner::SpawnItem()\n");

	//get a random item from our vector
	std::string strItem = GetRandomItem();

	//if we don't have anything to spawn, do nothing
	if(strItem.length() == 0)
		return;

	//spawn the item
	SpawnThing(strItem.c_str());
}

//builds m_itemVector from  pszItems
void ItemSpawner::BuildList(const char* pszItems)
{
	std::string strItem;

	// Get a pointer to the char, so we can use it to walk along the string
		const char* p = pszItems;

	//keep going until we hit a NULL
	while(*p)
	{
		//if it's a separator
		if(*p == ';')
		{
			// We need to add it to the vector
				m_itemVector.push_back(strItem);

			// Then set the string to nothing, so
			// it's ready to be used next time round the loop
				strItem = "";
		}
		else
		{
			//this is a normal char, so just add it to the item string
			strItem.append(1, *p);
		}

		// Update the pointer to the next letter
		++p;
	}

	//now add the last one if we need to
	if(strItem.length() > 0)
	{
		m_itemVector.push_back(strItem);
	}
}

//spawns a 'pszClassName' at our location
void ItemSpawner::SpawnThing(const char* pszClassName)
{
	Vector vec;
	
	vec.x = G_Random(size.x) + mins.x;
	vec.y = G_Random(size.y) + mins.y;
	vec.z = G_Random(size.z) + mins.z;

	vec += centroid;

	gi.DebugPrintf("\n(%f, %f, %f)\n", vec.x,vec.y,vec.z);

	SpawnArgs args;

	// Set the class name to the one passed in
	args.setArg("classname", pszClassName);
	args.setArg("origin", va("%d %d %d", static_cast<int>(vec.x), static_cast<int>(vec.y), static_cast<int>(vec.z)));

	//TODO: add all our spawn args to this new ent - but not 'items'

	//spawn it
	Entity* pEnt = args.Spawn();

	//tell it that we are it's spawner
	pEnt->SetSpawner(this);
}

//returns a random item from m_itemVector
std::string ItemSpawner::GetRandomItem()
{
	//don't do anything if the list is empty
	if(m_itemVector.empty())
		return "";

	//return a random class name
	int iItem = rand() % m_itemVector.size();

	return m_itemVector[iItem];
}

//returns the spawn time based on the key/value settings for this ent
float ItemSpawner::GetSpawnTime()
{
	//set iTime to a number in the range m_iWait +/- m_iRandom
	int iTime = m_iWait - m_iRandom;

	iTime += rand() % (m_iRandom*2);

	return static_cast<float>(iTime);
}


void ItemSpawner::SetItems(Event* ev)
{
	BuildList(ev->GetString(1));
}

void ItemSpawner::SetWait(Event* ev)
{
	m_iWait = ev->GetInteger(1);
}

void ItemSpawner::SetRandom(Event* ev)
{
	m_iRandom = ev->GetInteger(1);
}*/