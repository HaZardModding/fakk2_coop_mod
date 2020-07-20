///////////////////////////////////////////////////////////////////////////////
//
//
// CTF_Spit class weapon implementation
//
// -Yorvik
//
///////////////////////////////////////////////////////////////////////////////
#include "spit.h"
#include "Actor.h"
#include "Navigate.h"
#include "Player.h"
#include "ctf_global.h"
#include "ctf_tech.h"

//Events
Event EV_CTF_Spit_SetTeleportModel		("teleport_model",	EV_DEFAULT,		"s", "teleportModel",	"filename of the tiki used for the teleport effect.");
Event EV_CTF_SpitModule_SetReturnModel	("return_model",	EV_DEFAULT,		"s", "returnModel",		"filename of the tiki used for the return effect.");
Event EV_CTF_SpitModule_Return			("module_return",	EV_DEFAULT,		"e", NULL,				"returns the module to its parent CTF_Spit");
Event EV_CTF_TakeDamage					("ctf_takedamage",	EV_FROM_CODE,	NULL,NULL,				"makes the module targetable");



//--------------------------------------------------------
//GLOBALS
//--------------------------------------------------------



//
//ReturnCTF_SpitModule - Called when command: "ctf_spit_return" is recieved
//
namespace CTF{
qboolean ReturnSpitModule(gentity_t* ent)
{
	if(!ent->entity->isSubclassOf(Player))
	{
		assert(false);
		return qtrue;
	}

	Player* player	= static_cast<Player*>(ent->entity);
	Weapon* weap	= NULL;
	CTF_Spit* spit	= NULL;

	//check left hand for a CTF_Spit
	if( (weap = player->GetActiveWeapon(WEAPON_LEFT)) != NULL  &&  weap->isSubclassOf(CTF_Spit))
		spit = static_cast<CTF_Spit*>(weap);

	//if left hand had no CTF_Spit, try the right
	else if( (weap = player->GetActiveWeapon(WEAPON_RIGHT)) != NULL  &&  weap->isSubclassOf(CTF_Spit))
		spit = static_cast<CTF_Spit*>(weap);

	//call the CTF_Spit's return func
	if(spit)
		spit->ReturnModule(true);

	return qtrue;
}
}//~namespace CTF



//--------------------------------------------------------
//CTF_Spit (Weapon)
//--------------------------------------------------------



//
//Class Declaration
//
CLASS_DECLARATION(Weapon, CTF_Spit, "ctf_weapon_spit")
{
	{&EV_CTF_Spit_SetTeleportModel,	SetTeleportModel},
	{NULL, NULL}
	
};

CTF_Spit::CTF_Spit() : 
	Weapon(), m_firedModule(NULL), m_teleportModel("fx_spitteleport.tik")
{
	ammo_clip_size[FIRE_PRIMARY] = 0;
	ammo_in_clip[FIRE_PRIMARY] = 0;
	notdroppable = qtrue;
}

CTF_Spit::~CTF_Spit()
{
	//remove any CTF_Spit modules we have
	ReturnModule();
}

//
//Teleport - Zap's our owner to the position of the last CTF_Spit module fired
//
void CTF_Spit::Teleport()
{
	const float scale = 1.3f;
	Player* player =	static_cast<Player*>(
						static_cast<Sentient*>(owner));
	
	assert(m_firedModule);
	
	//make sure there is enough gap above and below us to fit a 96 unit high player
	//if tere is not, return the module harmlessly & play a failed teleport sound
	if(!m_firedModule->CheckPlayerFit(player))
	{
		ReturnModule();
		CTF::LocalSound(player, "sound/weapons/spit/fail.wav");
		return;
	}

	//check target area for teammates
		if(g_friendlyFire->value < 0.0001f && g_friendlyFire->value > -0.0001f)
		{
			if(CTF::ScaledKillBoxHurtsTeammate(m_firedModule->origin, player, scale))
			{
				ReturnModule();			
				Sound("sound/weapons/spit/fail.wav", CHAN_WEAPON);
				return;
			}
		}

	//spawn a teleport effect at current position
		Animate* effect = new Animate;
		effect->setOrigin(player->origin);	
		effect->setModel(m_teleportModel);
		effect->setScale(0.7f);
		effect->RandomAnimate("idle", EV_Remove);

	//make sure killbox doesnt hurt our module or our owner
		m_firedModule->unlink();
		player->unlink();

	//If we have the flag, drop it!
		player->DropFlag();

	//teleport & make sure we dont embed ourdself in the celing
		player->origin = m_firedModule->origin;
		
		trace_t upTrace = G_Trace(m_firedModule->origin, Vector(mins[0], mins[1], 0), Vector(maxs[0], maxs[1], 0), m_firedModule->origin + Vector(0, 0, 97), NULL, MASK_SOLID, false, "spit upward height check");
		if(upTrace.fraction != 1)
			player->origin = upTrace.endpos - Vector(0, 0, 96);
		
		player->origin.copyTo(owner->edict->s.origin2);
		player->CameraCut();

	//clear the velocity and hold them in place briefly
		player->client->ps.pm_time = 100;
		player->client->ps.pm_flags |= PMF_TIME_TELEPORT;

	//Spawn in effect
		effect = new Animate;
		effect->setOrigin(player->origin);	
		effect->setModel(m_teleportModel);
		effect->setScale(0.7f);
		effect->RandomAnimate("idle", EV_Remove);

	//if the module has been disabled (ie blown up!), kill our player
		if(m_firedModule->Dead())
		{
			//update our centroid etc
				player->link();

			//HACK
				Vector tmp = player->origin;

			//hurt our owner
				Event* dmgEv = new Event(EV_Damage);
				dmgEv->AddInteger(player->health+500);		//damage
				dmgEv->AddEntity(m_firedModule);			//inflictor
				dmgEv->AddEntity(player);					//attacker
				dmgEv->AddVector(vec_zero);					//pos
				dmgEv->AddVector(vec_zero);					//dir
				dmgEv->AddVector(vec_zero);					//normal
				dmgEv->AddInteger(0);						//knockback  \/flags\/
				dmgEv->AddInteger(DAMAGE_NO_PROTECTION | DAMAGE_NO_KNOCKBACK | DAMAGE_NO_ARMOR);
				dmgEv->AddInteger(MOD_BROKEN_SPIT);			//means of death

				player->PostEvent(dmgEv, 0.1f);

			//HACK
				player->avelocity = vec_zero;
				player->velocity  = vec_zero;
				player->origin = tmp;
				player->origin.copyTo(player->edict->s.origin2);

				player->NoLerpThisFrame();
		}

	//the module is intact and teleport was sucessful, so clean up some player attribs
		else
		{
			//make sure player is hittable again
				player->flags &= ~FL_IMMOBILE;
				player->takedamage = DAMAGE_AIM;
				player->showModel();
		}

	//remove the module from the world
		m_firedModule->ProcessEvent(EV_Remove);
		m_firedModule = NULL;

	//spawn a killbox that is a bit bigger than julie
		CTF::ScaledKillBox(player, scale);
}

//
//ReturnModule
//
void CTF_Spit::ReturnModule(bool spawnEffect)
{
	if(!m_firedModule)
		return;

	//let the module know it's being pulled back
	m_firedModule->ProcessEvent(EV_CTF_SpitModule_Return);
	owner->Sound("sound/weapons/spit/returnmodule.wav", CHAN_AUTO);

	//kill the module
	m_firedModule->PostEvent(EV_Remove, 0);
	m_firedModule = NULL;
}

//
//EventResponse: SetTeleportModel
//
void CTF_Spit::SetTeleportModel(Event* ev)
{
	m_teleportModel = ev->GetString(1);
}


//
//EventResponse: Shoot
//
void CTF_Spit::Shoot(Event* ev)
{
	//if we have already fired a module, teleport to it
	if(m_firedModule)
	{
		Teleport();
		return;
	}

	//
	//Fire a CTF_Spit module
	//

	//check for firing error
	if(ev->NumArgs() > 0)
	{
		if(WeaponModeNameToNum(ev->GetString(1)) == FIRE_ERROR)
			return;
	}
	
	//If the muzzle is not clear, change to a clear animation
	if(!MuzzleClear())
	{
		//If weapon has a clear animation then change to it.
		if(HasAnim("clear"))
		{
			RandomAnimate( "clear", NULL );
			weaponstate = WEAPON_READY;
		}
		else
		{
			ForceIdle();
		}
		return;
	}
	
	Vector	pos, forward, right, up;
	GetMuzzlePosition(&pos, &forward, &right, &up);
	
	//fire the CTF_Spit module
	m_firedModule = static_cast<CTF_SpitModule*>(
					ProjectileAttack(	owner->centroid,
										forward,
										owner, 
										projectileModel[FIRE_PRIMARY],
										charge_fraction
									));

	//error firing...
	if(!m_firedModule)
	{
		assert(false);
		return;
	}

	m_firedModule->velocity *= 1.3f;

	//make some noise
	if(!quiet)
	{
		Player* player = static_cast<Player*>(&*owner);

		if(player)
			player->TechDamage(true);
			

		//play the 'fire' sound
		Sound("sound/weapons/spit/fire.wav", CHAN_WEAPON);

		if(next_noise_time <= level.time)
		{
			BroadcastSound();
			next_noise_time = level.time + 1;
		}
	}
}



//--------------------------------------------------------
//CTF_Spit Module (Projectile)
//--------------------------------------------------------



CLASS_DECLARATION(Projectile, CTF_SpitModule, NULL)
{
	{&EV_CTF_SpitModule_Return,			Return},
	{&EV_CTF_SpitModule_SetReturnModel,	SetReturnModel},
	{&EV_CTF_TakeDamage,				CTF_TakeDamage},
	{&EV_TakeDamage,					DummyTakeDamage},
	//{&EV_Touch,							Touch},
	{NULL, NULL}
};

CTF_SpitModule::CTF_SpitModule() :
	Projectile(), m_dead(false), m_killer(NULL), m_returnModel("fx_spitreturn.tik")
{
	//dont immediately take damage, sometimes when the spit is used in combos with swords
	//the module gets killed before it lands, which sux
		takedamage = DAMAGE_NO;
		setSolidType(SOLID_NOT);
		PostEvent(EV_CTF_TakeDamage, 1.0f);
}

CTF_SpitModule::~CTF_SpitModule()
{}


//
//Event Response: DamageEvent
//
//When the module is damaged, it is not destroyed, but made unusable
//so when the julie teleports to it, she dies!
//
void CTF_SpitModule::DamageEvent(Event* ev)
{
	if(health <= 0 || takedamage == DAMAGE_NO)
		return;

	int		damage		= ev->GetInteger(1);
	Entity* attacker	= ev->GetEntity(3);
	
	//do the damage
	health -= damage;

	//set dead flag & killer ent
	if(health <= 0)
	{
		m_dead = true;
		m_killer = attacker;

		//throw some eye candy
		Animate* effect = new Animate;
		effect->setOrigin(origin);	
		effect->setModel("fx_grenade_explosion.tik");
		effect->setScale(0.3f);
		effect->RandomAnimate("idle", EV_Remove);

		//set our animation to the 'broken' anim
		RandomAnimate("broken");
	}
	else
	{
		str realName = GetRandomAlias("snd_pain");

		if(realName.length() > 1)
			Sound(realName);
	}
}


//
//Event Response: Touch
//
//When the module touches an entity, it should fall strait to the floor
//
/*void CTF_SpitModule::Touch(Event* ev)
{
	Projectile::Touch(ev);
	return;

	Entity* other = ev->GetEntity(1);
	assert(other);

	if(other == world)
		return;

	//only modify behaviour when we hit a sentient thing
	if(!other->isSubclassOf(Sentient))
		return;

	velocity = vec_zero;
	speed = 0.0f;
	setMoveType(MOVETYPE_TOSS);
}*/

//
//Event Response: Return
//
void CTF_SpitModule::Return(Event* ev)
{
	//spawn an effect
	Animate* effect = new Animate;
	effect->setOrigin(origin);
	effect->setModel(m_returnModel);
	effect->setScale(0.3f);
	effect->RandomAnimate("idle", EV_Remove);
	
	RandomAnimate("smoke_off");
}

//
//Event Response: SetReturnModel
//
void CTF_SpitModule::SetReturnModel(Event* ev)
{
	m_returnModel = ev->GetString(1);
}

//
//TakeDamage - let's the module take damage
//
void CTF_SpitModule::CTF_TakeDamage(Event* ev)
{
	takedamage = DAMAGE_YES;
	setSolidType(SOLID_BBOX);
}

void CTF_SpitModule::DummyTakeDamage(Event* ev)
{
	takedamage = DAMAGE_NO;
	setSolidType(SOLID_NOT);
}

//
//CheckPlayerFit - check the player is going to fit in the area where the module is
//do this by height checking all four corners of players bounding box
//
bool CTF_SpitModule::CheckPlayerFit(Player* player)
{
	Vector min		= Vector(player->mins[0], player->mins[1], 0);
	Vector max		= Vector(player->maxs[0], player->maxs[1], 0);
	Vector height	= Vector(0, 0, abs(player->maxs[2]-player->mins[2]));

	trace_t upTrace = G_Trace(origin, min, max, origin+height, NULL, MASK_SOLID, qfalse, "spit height check");
	trace_t dnTrace = G_Trace(origin, min, max, origin-height, NULL, MASK_SOLID, qfalse, "spit height check");

	if(upTrace.fraction != 1 && dnTrace.fraction != 1 && abs(upTrace.endpos - dnTrace.endpos) < 96)
		return false;
	else
		return true;
}