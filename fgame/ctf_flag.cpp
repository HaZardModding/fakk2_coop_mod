#include "ctf_flag.h"
#include "ctf_manager.h"
#include "Player.h"
#include "ctf_global.h"
#include "TeamIterator.h"

const float FLAG_SCALE_ONGROUND	= 1.2f;
const float FLAG_SCALE_ONBACK	= 1.0f;

//
//EVENTS
//
Event EV_Set_Team(
		"set_ctf_team",
		EV_DEFAULT,
		"s",
		"flagTeam",
		"Sets the team this flag belongs to"
);

Event EV_CTF_FlagDropThink(
		"flagreturnthink",
		EV_DEFAULT,
		"s",
		"flagReturnThink",
		"Checks the surf contents at flags origin and returns flag if wrong place"
);

namespace CTF {
qboolean DropFlag(gentity_t* ent)
{
	if(!ent->entity->isSubclassOf(Player))
	{
		assert(false);
		return qtrue;
	}

	static_cast<Player*>(ent->entity)->DropFlag();

	return qtrue;
}
}//~namespace CTF

/////////////////////////////////////////////////////////////////////////////
// Flag base
CLASS_DECLARATION(CTF_InventoryItem, CTF_Flag, NULL)
{
	{&EV_Item_Pickup,			Pickup		},
	{&EV_Item_Respawn,			ReturnFlag	},
	{&EV_Set_Team,				SetTeam		},
	{&EV_CTF_FlagDropThink,		DropThink	},
	{NULL, NULL}
};

CTF_Flag::CTF_Flag() : 
	CTF_InventoryItem(),
	m_Team(TEAM_NONE),
	m_returner(NULL),
	m_returnTime(0.0f)
{
	AtSpawnLocation(true);
}

//
//OnSpawn - called just after the flag initially spawns
//
void CTF_Flag::OnSpawn()
{
	setScale(FLAG_SCALE_ONGROUND);
	RandomAnimate("idle");
}

//
//check to see if we should return...
//
void CTF_Flag::DropThink(Event* ev)
{
	CancelEventsOfType(EV_CTF_FlagDropThink);

	int cont = gi.pointcontents(origin, 0);

	if(
		cont & CONTENTS_LAVA ||  
		cont & CONTENTS_SLIME ||
		cont & CONTENTS_NODROP
	)
	{
		Event* ev = new Event(EV_Item_Respawn);
		ev->AddInteger(1);
		ProcessEvent(ev);
		return;
	}


	PostEvent(EV_CTF_FlagDropThink, 1);
}

void CTF_Flag::CTF_DisplayPickup(Entity* ent)
{
	if(!ent)
		ent = static_cast<Entity*>(&*owner);

	if(!ent)
		return;

	gi.SendServerCommand(ent->edict-g_entities, "item_pickup flag %s", getName().c_str());
}

// Called when a player picks up this flag
void CTF_Flag::Pickup(Event* ev)
{
	// If we're not here - there's nowt to do
		if(hidden())
			return;

	// Get the entity that picked us up
		Entity* pEnt = ev->GetEntity(1);

	// We only care if we are collected by a player
		if(!pEnt || !pEnt->isSubclassOf(Player))
			return;

		Player* pPlayer = static_cast<Player*>(pEnt);

	//make sure our player is on a proper team
	//this fixes spectators picking up flags
		if(pPlayer->GetTeam() != TEAM_RED && pPlayer->GetTeam() != TEAM_BLUE)
		{
			assert(false);
			return;
		}

	// Make sure this flag is on a valid team
		if(GetTeam() != TEAM_RED && GetTeam() != TEAM_BLUE)
		{
			assert(false);
			return;
		}

	// Call the correct pickup function depending on the team they are on
		if(pPlayer->GetTeam() == GetTeam())
		{
			PickupSameTeam(pPlayer);
		}
		else
		{
			PickupDifferentTeam(pPlayer);
		}
}

// Called when pPlayer picks up this flag, and is on the same team
void CTF_Flag::PickupSameTeam(Player* pPlayer)
{
	//If flag is NOT at spawn point (ie it has been dropped someplace),
	//send it home
		if(!IsAtSpawnLocation())
		{
			m_returnTime = level.time;
			m_returner = pPlayer;

			ReturnFlag(NULL);
			GetCTFManager().FlagReturn(pPlayer, this);
			return;
		}

	// If they have a flag
		if(CTF_Flag* pFlag = pPlayer->GetFlag())
		{
		// Remove it form the player
			pPlayer->SetFlag(NULL);

		// And send the flag back to it's base
			pFlag->ReturnFlag(NULL);

		// They just captured it
			GetCTFManager().FlagCapture(pPlayer, pFlag);
		}
}

// Called when pPlayer picks up this flag, and is on the enemy team
void CTF_Flag::PickupDifferentTeam(Player* pPlayer)
{
	// The can't have more than one flag
		if(pPlayer->GetFlag())
			return;

	//make the flag smaller for being on the players back
		setScale(FLAG_SCALE_ONBACK);

	//update player stats
		GetCTFManager().GetTeam(GetTeam())->SetFlagStatus(FLAG_STOLEN);
		pPlayer->SetFlag(this);	
		GetCTFManager().FlagPickup(pPlayer, this);

	// Make sure we dont return to base on our own
		CancelEventsOfType(EV_Item_Respawn);

	//attatch to players back
		RandomAnimate("carry");

		attach(
			pPlayer->entnum,
			gi.Tag_NumForName(pPlayer->edict->s.modelindex, "tag_back")
		);

	//call the pickup func
		CTF_DisplayPickup(pPlayer);
}

//
//ReturnFlag
//
void CTF_Flag::ReturnFlag(Event* ev)
{
	detach();
	setSolidType(SOLID_TRIGGER);
	setScale(FLAG_SCALE_ONGROUND);
	RandomAnimate("idle");

	// Put us back to our spawn location, and make sure we're visible
		ReturnToSpawnPos();
		showModel();

	// We're back now, and we dont want any double-returns. Should this go 
	// in CTF_InventoryItem::ReturnToSpawnPos?
		CancelEventsOfType(EV_Item_Respawn);
		CancelEventsOfType(EV_CTF_FlagDropThink);

	//update player stats
		GetCTFManager().GetTeam(GetTeam())->SetFlagStatus(FLAG_AT_BASE);

	//if this is an auto return, play sounds etc
		if(ev && ev->GetBoolean(1) == true)
			GetCTFManager().FlagReturn(NULL, this);

#ifdef _DEBUG
	// Make sure that we're not being held by a player anymore
	// If a player does still have us, something is b0rked
	TeamIterator end;

	for(TeamIterator iter(TEAM_NONE); iter != end; ++iter)
	{
		assert(iter->GetFlag() != this);
	}
#endif
}


// Sets the team for this flag
void CTF_Flag::SetTeam(Event* ev)
{
	m_Team = CTF::GetTeam(ev->GetString(1));	
}


// pPlayer has just droped us
void CTF_Flag::Droped(Player* pPlayer)
{
	//update player stats
		GetCTFManager().GetTeam(GetTeam())->SetFlagStatus(FLAG_ON_GROUND);

	//detatch
		RandomAnimate("idle");
		detach();

		setScale(FLAG_SCALE_ONGROUND);

	// Then make us be 'droped'
		setOrigin(pPlayer->origin);		
		PlaceItem();

	// Post the event to make use go back to base
		Event* ev = new Event(EV_Item_Respawn);
		ev->AddInteger(1); //this assures the correct sounds are played...
		PostEvent(ev, 30);

	//not home anymore
		AtSpawnLocation(false);

	//feedback
		GetCTFManager().FlagDrop(pPlayer, this);


	//check our location
		ProcessEvent(EV_CTF_FlagDropThink);
}

//get the player who returned the flag JUST before a capture
//returns null is the flag was returned ages ago
Player* CTF_Flag::GetReturner()
{
	if(level.time - m_returnTime > CTF::MAX_ASSIST_TIME)
		return NULL;

	return m_returner;
}
