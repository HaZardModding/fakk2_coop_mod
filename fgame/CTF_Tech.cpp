///////////////////////////////////////////////////////////////////////////////
//
//
// CTF Tech implementation
//
// -Yorvik
//
///////////////////////////////////////////////////////////////////////////////

#include "CTF_Tech.h"
#include "Player.h"
#include "ctf_global.h"
#include "ctf_itemspawner.h"

extern "C"{
	char g_CTF_TechShader[1024];
}

namespace CTF{

//drop our tech - only used when single tech mode is on
qboolean DropTech(gentity_t* ent)
{
	if(!g_singleTech->integer || !ent->entity || !ent->entity->isSubclassOf(Player))
		return qtrue;

	static_cast<Player*>(ent->entity)->DropTechs();

	return qtrue;
}

} //~namespace CTF

//
//Events
//
Event	EV_CTF_Tech_Drop				("droptech",			EV_DEFAULT, NULL,	NULL,	"");
Event	EV_CTF_Tech_Trigger				("triggertech",			EV_DEFAULT, NULL,	NULL,	"");
Event	EV_CTF_Tech_Second				("triggersecond",		EV_DEFAULT, NULL,	NULL,	"");

Event	EV_CTF_Tech_PickupSound			("snd_pickup",			EV_DEFAULT, "s",	NULL,	"filename of a wave");
Event	EV_CTF_Tech_UseSound			("snd_use",				EV_DEFAULT, "s",	NULL,	"filename of a wave");
Event	EV_CTF_Tech_DieSound			("snd_die",				EV_DEFAULT, "s",	NULL,	"filename of a wave");
Event	EV_CTF_Tech_SetEffect			("const_effect",		EV_DEFAULT, "s",	NULL,	"filename of a tiki that will be attatched to the player");

Event	EV_CTF_Tech_HealthRegenerate	("health_regen",		EV_DEFAULT, "f",	NULL,	"");
Event	EV_CTF_Tech_WaterRegenerate		("water_regen",			EV_DEFAULT, "f",	NULL,	"");
Event	EV_CTF_Tech_EmpathyFactor		("empathy_factor",		EV_DEFAULT, "f",	NULL,	"");
Event	EV_CTF_Tech_DamageFactor		("damage_factor",		EV_DEFAULT, "f",	NULL,	"");
Event	EV_CTF_Tech_ProtectionFactor	("protection_factor",	EV_DEFAULT, "f",	NULL,	"");
Event	EV_CTF_Tech_SetAcroFactor		("acro_factor",			EV_DEFAULT,	"f",	NULL,	"");
Event	EV_CTF_Tech_Delay				("trigger_delay",		EV_DEFAULT, "f",	NULL,	"");
Event	EV_CTF_Tech_Duration			("duration",			EV_DEFAULT, "f",	NULL,	"");
Event	EV_CTF_Tech_ID					("tech_id",				EV_DEFAULT, "i",	NULL,	"");

Event	EV_CTF_Tech_Shrink				("shrink",				EV_DEFAULT, NULL,	NULL,	"");
Event	EV_CTF_Tech_Grow				("grow",				EV_DEFAULT, NULL,	NULL,	"");

extern	Event EV_Item_RespawnDone;
extern	Event EV_SpawnFlags;
extern	Event EV_SetScale;

//////////////////////////////////////////////////////////////////////
// CTF_Tech
//////////////////////////////////////////////////////////////////////

CLASS_DECLARATION(CTF_InventoryItem, CTF_Tech, NULL)
{
	{&EV_SetScale,					SetScale				},
	{&EV_Item_Pickup,				Pickup					},
	{&EV_Item_Respawn,				Respawn					},
	{&EV_CTF_Tech_Drop,				DropTech				},
	{&EV_CTF_Tech_Trigger,			TechTrigger				},
	{&EV_CTF_Tech_Second,			TriggerSecond			},
	{&EV_CTF_Tech_Shrink,			Shrink					},
	{&EV_CTF_Tech_Grow,				Grow					},

	{&EV_CTF_Tech_PickupSound,		SetPickupSound			},
	{&EV_CTF_Tech_UseSound,			SetUseSound				},
	{&EV_CTF_Tech_DieSound,			SetDieSound				},
	{&EV_CTF_Tech_SetEffect,		SetEffect				},

	{&EV_CTF_Tech_ID,				SetID					},
	{&EV_CTF_Tech_HealthRegenerate,	SetHealthRegenAmount	},
	{&EV_CTF_Tech_WaterRegenerate,	SetWaterRegenAmount		},
	{&EV_CTF_Tech_EmpathyFactor,	SetEmpathyFactor		},
	{&EV_CTF_Tech_DamageFactor,		SetDamageFactor			},
	{&EV_CTF_Tech_ProtectionFactor,	SetProtectionFactor		},
	{&EV_CTF_Tech_SetAcroFactor,	SetAcroFactor			},
	{&EV_CTF_Tech_Delay,			SetDelay				},

	{&EV_CTF_Tech_Duration,			SetDuration				},

	{NULL, NULL}
};

//constructor
CTF_Tech::CTF_Tech() : 
	CTF_InventoryItem(),
	m_pickupSnd(""),
	m_useSnd(""),
	m_dieSnd(""),
	m_healthRegenAmount(0.0f),
	m_waterRegenAmount(0.0f),
	m_empathyFactor(0.0f),
	m_damageFactor(1.0f),
	m_protectionFactor(1.0f),
	m_acroFactor(1.0f),
	m_delay(-1.0f),
	m_duration(30.0f),
	m_lifeleft(m_duration),
	m_scale(1.0f),
	m_effect(NULL)
{	
	if(DM_FLAG(DF_NO_POWERUPS))
	{
		PostEvent(EV_Remove, EV_REMOVE);
		return;
	}
}

//destructor
CTF_Tech::~CTF_Tech()
{
}

//---------------------
//Item Events
//---------------------

void CTF_Tech::CTF_DisplayPickup(Entity* ent)
{
	if(!ent)
		ent = static_cast<Entity*>(&*owner);

	if(!ent)
		return;

	gi.SendServerCommand(ent->edict-g_entities, "item_pickup tech %s", getName().c_str());
}

//
//initialise the scale value
//
void CTF_Tech::SetScale(Event* ev)
{
	m_scale = ev->GetFloat(1);
}

//
//shrinks the tech model till its gone
//
void CTF_Tech::Shrink(Event* ev)
{
	float f = edict->s.scale - 0.03f;

	if(f < 0.0f)
		f = 0.0f;
	
	if(f > 0.0f)
		PostEvent(*ev, FRAMETIME);
	
	edict->s.scale = f;
}

//
//grows the tech model till its back to origional size
//
void CTF_Tech::Grow(Event* ev)
{
	float f = edict->s.scale + 0.03f;

	if(f >= m_scale)
		f = m_scale;
	
	if(f < m_scale)
		PostEvent(*ev, FRAMETIME);
	
	edict->s.scale = f;
}

//
//Pickup - Called when something picks us up
//
void CTF_Tech::Pickup(Event* ev)
{
	Player* player = static_cast<Player*>(ev->GetEntity(1));

	if(!player || (player->GetTeam() != TEAM_RED && player->GetTeam() != TEAM_BLUE))
		return;

	if(!CTF_CheckTeam(player))
		return;

	//if singleTech is ON and we already have a tech, bail
	if(g_singleTech->integer)
	{
		//if the player already has a tech, bail
		if(!player->GetTechList().empty())
		{
			CTF::CenterPrint(player, S_COLOR_WHITE "You already have a TECH powerup");
			trigger_time = level.time + 1;
			return;
		}

		//make SURE the tech lasts forever
		else
		{
			m_lifeleft = -10;
			m_duration = -10;
		}
	}

	//set owner
	owner = player;
	CTF_DisplayPickup(player);

	//remove from world
	hideModel();
	setSolidType(SOLID_NOT);

	//play pickup sound
	player->Sound(m_pickupSnd, CHAN_AUTO);

	//stop any respawn events
	CancelEventsOfType(EV_Item_Respawn);

	//if we dont already have a tech of this type, start the event loops
	if(!player->GiveTech(this))
	{
		//post our trigger event
		if(m_delay > 0.0f)
			PostEvent(EV_CTF_Tech_Trigger, m_delay);

		//our once per second event
			PostEvent(EV_CTF_Tech_Second, 1.0f);

		//add effect to player
			if(m_constantEffect.length() > 0)
			{
				m_effect = new Animate;
				m_effect->setModel(m_constantEffect.c_str());
				m_effect->setMoveType(MOVETYPE_NONE);
				m_effect->setSolidType(SOLID_NOT);
				
				m_effect->attach(
					player->entnum,
					gi.Tag_NumForName(player->edict->s.modelindex, "Bip01 Spine")
				);
			}
	}

	//shrink model down so it grows properly on respawn
	edict->s.scale = 0.0f;

	//spawner bits
	if(spawner)
	{
		spawner->ItemPickedUp();
		spawner = NULL;
		respawnable = false;
	}
}

//
//Respawn
//
void CTF_Tech::Respawn(Event* ev)
{
	if(!respawnable)
		return;

	//drop item
	origin = GetSpawnPos();	
	PlaceItem();

	Vector save = origin;
	if(!droptofloor(8192))
	{
		gi.DPrintf("%s (%d) stuck in world at '%5.1f %5.1f %5.1f'\n", getClassID(), entnum, origin.x, origin.y, origin.z);
		setOrigin(save);
		setMoveType(MOVETYPE_NONE);
	}
	else
	{
		setMoveType(MOVETYPE_NONE);
	}

	
	//play global respawn sound
	if(!g_singleTech->integer)
		Sound("sound/techs/respawn.wav", CHAN_AUTO, -1, LEVEL_WIDE_MIN_DIST);
	else
		Sound("snd_itemspawn");
	

	if(HasAnim("respawn"))
		RandomAnimate("respawn", EV_Item_RespawnDone);

	look_at_me = true;
	has_been_looked_at = false;

	//reset lifetime
	m_lifeleft = m_duration;

	//make sure there arent anymore respawns lined up...
	CancelEventsOfType(EV_Item_Respawn);

	//restore the model's scale
	edict->s.scale = 0.0f;
	ProcessEvent(EV_CTF_Tech_Grow);
}

//
//DropTech - called when player drops the tech (either by dying, switching teams, 
//quitting or manually dropping using console cmd)
//
void CTF_Tech::DropTech(Event* ev)
{
	if(!owner)
		return;

	edict->s.scale = m_scale;

	//clear the team so anyone can pick up the tech
	m_team = TEAM_NONE;
	


	Player* player = static_cast<Player*>(&*owner);

	if(player)
		player->RemoveTech(GetID());
	else
		gi.Printf("tech::droptech: could not cast owner to player\n");

	//remove effect around player
	if(m_effect)
		m_effect->ProcessEvent(EV_Remove);

	setOrigin(owner->origin + Vector(0, 0, 40));
	velocity = owner->velocity * 0.5 + Vector(G_CRandom(150), G_CRandom(150), 90);
	setAngles(owner->angles);
	avelocity = Vector(0, G_CRandom( 360 ), 0);
	PlaceItem();

	trigger_time = level.time + 1;	
	spawnflags |= DROPPED_PLAYER_ITEM;
	
	owner = NULL;

	CancelEventsOfType(EV_CTF_Tech_Trigger);	//dont trigger anymore
	CancelEventsOfType(EV_CTF_Tech_Second);		//dont bother keeping track of time
	CancelEventsOfType(EV_Item_Respawn);

	PostEvent(EV_CTF_Tech_Shrink, RespawnTime()-2.0f);
	PostEvent(EV_Item_Respawn, RespawnTime());
}

//
//Trigger - called once every 'delay' seconds
//
void CTF_Tech::TechTrigger(Event* ev)
{
	if(m_delay > 0.0f)
		PostEvent(EV_CTF_Tech_Trigger, m_delay);

	//get our owner
	Player* player = static_cast<Player*>(&*owner);
	if(!player)
		return;

	bool sound = false;

	//regenerate HEALTH
	if(m_healthRegenAmount != 0.0f && player->health < 100)
	{
		int h = m_healthRegenAmount;
		if(player->health + h > 100)
			h = 100-player->health;
		
		player->health += h;
		sound = true;

		if(m_effect && m_effect->HasAnim("regen_health"))
			m_effect->RandomAnimate("regen_health");
	}

	//regenerate WATER
	if(m_waterRegenAmount != 0.0f && player->GetWaterPower() < 100)
	{
		int w = m_waterRegenAmount;
		if(player->GetWaterPower() + w > 100)
			w = 100-player->GetWaterPower();
		
		player->AddWater(w);
		sound = true;

		if(m_effect && m_effect->HasAnim("regen_water"))
			m_effect->RandomAnimate("regen_water");
	}

	if(sound)
		player->Sound(m_useSnd, CHAN_AUTO);
}

//
//TriggerSecond - called once per second to manage timeouts etc
//
void CTF_Tech::TriggerSecond(Event* ev)
{
	//if we JUST wore off
	if(m_lifeleft > 0.0f && (--m_lifeleft) <= 0.001f)
	{
		static_cast<Player*>(&*owner)->RemoveTech(GetID());
		trigger_time = level.time + RespawnTime();

		owner->Sound(m_dieSnd, CHAN_AUTO);
		owner = NULL;

		if(m_effect)
			m_effect->ProcessEvent(EV_Remove);

		CancelEventsOfType(EV_CTF_Tech_Trigger);
		CancelEventsOfType(EV_CTF_Tech_Second);
		CancelEventsOfType(EV_Item_Respawn);
		PostEvent(EV_Item_Respawn, RespawnTime());
		return;
	}

	//if tech has low life left, play warning sound
	if(m_lifeleft > 0.0f && m_lifeleft <= CTF_TECH_WARNING_TIME)
		owner->Sound("sound/techs/wearoff.wav", CHAN_AUTO);

	PostEvent(EV_CTF_Tech_Second, 1.0f);
}

//
//Damage - called when an ent takes damage
//
int CTF_Tech::TechDamage(Event* ev, int dmg)
{
	Entity* inflictor	= ev->GetEntity(2);
	Entity* attacker	= ev->GetEntity(3);
	Player* player		= static_cast<Player*>(&*owner);

	//
	//damage (quad-esque)
	//
	if(m_damageFactor >= 0.0f)
		dmg *= m_damageFactor;

	//
	//protection
	//
	if(m_protectionFactor >= 0.0f)
		dmg *= m_protectionFactor;

	//
	//empathy
	//
	if(m_empathyFactor >= 0.0f && player != attacker && ev->GetInteger(9) != MOD_EMPATHY)
	{
		attacker->Damage(inflictor, player, dmg - dmg*m_empathyFactor, 
						vec_zero, vec_zero, vec_zero, 0, 0, MOD_EMPATHY);

		dmg *= m_empathyFactor;
	}

	return dmg;
}

//---------------------
//Set's
//---------------------

void CTF_Tech::SetDuration(Event* ev)
{
	if(g_singleTech->integer)
		m_duration = ev->GetFloat(1);
	else
		m_duration = -1.0f;

	m_lifeleft = m_duration;
}

void CTF_Tech::SetEmpathyFactor(Event* ev)
{
	m_empathyFactor = ev->GetFloat(1);
}

void CTF_Tech::SetDamageFactor(Event* ev)
{
	m_damageFactor = ev->GetFloat(1);
}

void CTF_Tech::SetProtectionFactor(Event* ev)
{
	m_protectionFactor = ev->GetFloat(1);
}

void CTF_Tech::SetAcroFactor(Event* ev)
{
	m_acroFactor = ev->GetFloat(1);
}

void CTF_Tech::SetHealthRegenAmount(Event* ev)
{
	m_healthRegenAmount = ev->GetFloat(1);
}

void CTF_Tech::SetWaterRegenAmount(Event* ev)
{
	m_waterRegenAmount = ev->GetFloat(1);
}

void CTF_Tech::SetDelay(Event* ev)
{
	m_delay = ev->GetFloat(1);
}

void CTF_Tech::SetID(Event* ev)
{
	m_id = ev->GetInteger(1);
}

//---------------------
//Sound Set's
//---------------------

void CTF_Tech::SetPickupSound(Event* ev)
{
	m_pickupSnd = ev->GetString(1);
}

void CTF_Tech::SetUseSound(Event* ev)
{
	m_useSnd = ev->GetString(1);
}

void CTF_Tech::SetDieSound(Event* ev)
{
	m_dieSnd = ev->GetString(1);
}

void CTF_Tech::SetEffect(Event* ev)
{
	m_constantEffect = ev->GetString(1);
}