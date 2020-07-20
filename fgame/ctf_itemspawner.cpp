///////////////////////////////////////////////////////////////////////////////
//
//
// CTF_ItemSpawner class implementation
//
// -Yorvik
//
///////////////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)
#pragma warning(disable: 4786)

#include "Item.h"
#include "ctf_itemspawner.h"
#include "ctf_global.h"

#pragma warning(push, 1)
#include <algorithm>
#pragma warning(pop)

//Events
Event EV_CTF_Spawner_AddItems("spawn_items",	EV_DEFAULT, "s", NULL,	"semicolon seperated list of items to add to our arsenal");
Event EV_CTF_Spawner_SetDelay("spawn_delay",	EV_DEFAULT, "f", NULL,	"time (in seconds) between spawns");
Event EV_CTF_Spawner_SetRand ("spawn_random",	EV_DEFAULT, "f", NULL,	"maximum time (in seconds) to be added or subtracted from delay");
Event EV_CTF_Spawner_Commands("spawn_commands",	EV_DEFAULT,	"s", NULL,  "passes these commands to each spawned item");
Event EV_CTF_Spawner_SpawnItem("spawnitem",		EV_FROM_CODE, NULL, NULL,	"spawns a random item from our list");

//Messages
CLASS_DECLARATION(Item, CTF_ItemSpawner, "ctf_item_spawner")
{
	{&EV_CTF_Spawner_AddItems,		AddSpawnItems	},
	{&EV_CTF_Spawner_SpawnItem,		SpawnRandomItem	},
	{&EV_CTF_Spawner_SetDelay,		SetDelay		},
	{&EV_CTF_Spawner_SetRand,		SetRandom		},
	{&EV_CTF_Spawner_Commands,		SetCommands		},
	{NULL, NULL}
};

//
//Construction / Destruction
//
CTF_ItemSpawner::CTF_ItemSpawner() :
	m_delay(30.0f),
	m_random(0.0f)
{
	//FIXME ?
	PostEvent(EV_CTF_Spawner_SpawnItem, 3);
}

CTF_ItemSpawner::~CTF_ItemSpawner()
{
}

//
//the last item we spawned has been picked up, so spawn another one
//
void CTF_ItemSpawner::ItemPickedUp()
{
	PostEvent(EV_CTF_Spawner_SpawnItem, m_delay + G_CRandom(m_random));
}

//
//parses given event args and puts semicolon separated strings in given list
//
void CTF_ItemSpawner::PopulateStringList(StringList& list, Event* ev)
{
	//bail if we have no event or the event has no args
		if(!ev || ev->NumArgs() < 1)
		{
			assert(false);
			return;
		}

	//get the string from the event
		std::string itemString = ev->GetString(1);

	//get rid of whitespace and quotes
		CTF::RemWhite(itemString);
		CTF::RemChar(itemString, '"');

	// If we're empty now, theres nothing to do
		if(itemString.empty())
			return;

	// Then add all the ; seperatred strings to the list
		typedef std::string::iterator Iterator;

		Iterator itemBegin = itemString.begin();
		Iterator end = itemString.end();
		
		while(itemBegin != end) 
		{
			// Find the end of the next item (i.e. the next ';')
				Iterator itemEnd = std::find(itemBegin, end, ';');

			// Then add it to our list
				list.push_back(std::string(itemBegin, itemEnd));

			// set first to whatever last is for next time round
				itemBegin = itemEnd;

			// if we're not at end, we need an extra ++ to move
			// passed the ';' that we just found
				if(itemBegin != end)
					++itemBegin;
		}
}



//
//Parses the string arg for items and adds them to our spawn list
//
void CTF_ItemSpawner::AddSpawnItems(Event* ev)
{
	PopulateStringList(m_items, ev);
}

//
//Spawn a random item
//
void CTF_ItemSpawner::SpawnRandomItem(Event* ev)
{
	if(m_items.empty())
	{
		// Let the mapper know that they forgot to put items into the spawner
		gi.Printf("** Warning: empty ctf_item_spawner - nothing to spawn\n");
		return;
	}

	//pick a random item from our list
		std::string name = m_items[rand() % m_items.size()];	

	//create a new entity
		SpawnArgs args;	
		args.setArg("classname", name.c_str());
		args.setArg("model", name.c_str());
	
	//get class info
		ClassDef* cls = args.getClassDef();
		if(!cls)
			cls = &Entity::ClassInfo;

	//make sure the class is derived from Entity
		if(!checkInheritance(&Entity::ClassInfo, cls))
		{
			ev->Error("%s is not a valid Entity", name);
			return;
		}
		
	//create new instance of the item
		Entity* ent = static_cast<Entity*>(cls->newInstance());
		Event* e = NULL;
	
	//set model
		e = new Event(EV_Model);
		e->AddString(name.c_str());
		ent->PostEvent(e, EV_SPAWNARG);
	
	//set origin
		e = new Event(EV_SetOrigin);
		e->AddVector(GetRandomOrigin());
		ent->PostEvent(e, EV_SPAWNARG);
	
	//set angles
		e = new Event(EV_SetAngles);
		e->AddVector(Vector(0, 0, 0));
		ent->PostEvent(e, EV_SPAWNARG);
	
	//set idle anim
		e = new Event(EV_Anim);
		e->AddString("idle");
		ent->PostEvent(e, EV_SPAWNARG);

	//set the items spawner to this
		ent->SetSpawner(this);

	//make sure the ent does not respawn
		if(ent->isSubclassOf(Item))
		{
			Item* item = static_cast<Item*>(ent);
			item->setRespawn(qfalse);
		}

	//add our commands
		for(StringList::iterator it = m_commands.begin(); it != m_commands.end(); ++it)
		{
			std::string::size_type n = it->find('=');
			e = new Event(it->substr(0, n).c_str());
			e->AddString(it->substr(n+1, it->size()-(n+1)).c_str());
			ent->PostEvent(e, EV_SPAWNARG);
		}

	//set spawned item'steam to our team
		str teamStr = "x";
		if(m_team == TEAM_RED)  teamStr = "R";
		if(m_team == TEAM_BLUE) teamStr = "B";
		
		e = new Event(EV_CTF_Item_SetTeam);
		e->AddString(teamStr.c_str());
		ent->ProcessEvent(e);

	//play a sound
		Sound("sound/weapons/respawn.wav");
}

//returns a random point within the area of the spawner entity.
//if the spawner is a point entity (ie has no assosiated brush), its bare origin is returned.
Vector CTF_ItemSpawner::GetRandomOrigin()
{
	if(!size.x && !size.y && !size.z)
		return origin;

	return Vector(
		centroid.x + ((size.x/2.0f) * G_CRandom()),
		centroid.y + ((size.y/2.0f) * G_CRandom()),
		centroid.z + ((size.z/2.0f) * G_CRandom())
	);
}

//
//Set's...
//
void CTF_ItemSpawner::SetDelay(Event* ev)
{
	if(!ev || ev->NumArgs() < 1)
	{
		assert(false);
		return;
	}

	m_delay = ev->GetFloat(1);
}

void CTF_ItemSpawner::SetRandom(Event* ev)
{
	if(!ev || ev->NumArgs() < 1)
	{
		assert(false);
		return;
	}

	m_random = ev->GetFloat(1);
}

void CTF_ItemSpawner::SetCommands(Event* ev)
{
	PopulateStringList(m_commands, ev);	
}