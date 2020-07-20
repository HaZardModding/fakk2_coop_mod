///////////////////////////////////////////////////////////////////////////////
//
// This is the ctf part of the Player class. The header is included by Player.h
// inside the player class. We need to included player.h in this file so we get 
// the full player class.
//
///////////////////////////////////////////////////////////////////////////////
//
#include "player.h"
#include "Shield.h"
#include "ctf_manager.h"
#include "ctf_flag.h"
#include "ctf_global.h"
#include "ctf_tech.h"
#include "ctf_playerstart.h"
#include "ctf_targetgive.h"
#include "EntityClassIterator.h"
#include "WeapUtils.h"
#include "CTF_DeadBody.h"
#include "TeamIterator.h"
#include "CTF_Obituary.h"
#include <io.h>

typedef EntityClassIterator<CTF_PlayerStart> DmStartIter;

#pragma warning(push, 1)
#include <vector>
#include <string>
#pragma warning(pop)

namespace CTF{
	//
	//Streak stuff
	//
	const int STREAK_NUM_MESSAGES = 5;

	//streak messages
	const str STREAK_MESSAGES[STREAK_NUM_MESSAGES] = {
		S_COLOR_RED "M O N S T E R - K I L L",
		S_COLOR_RED "ULTRA-KILL",
		S_COLOR_RED "MULTI-KILL",
		S_COLOR_RED "RAMPAGE",
		S_COLOR_RED "EXCELLENT"
	};

	//corresponging sounds
	const str STREAK_SOUNDS[STREAK_NUM_MESSAGES] = {
		"sound/feedback/streak5.wav",
		"sound/feedback/streak4.wav",
		"sound/feedback/streak3.wav",
		"sound/feedback/streak2.wav",
		"sound/feedback/streak1.wav"
	};

	//# frags required for each streak 'level'
	const int STREAK_FRAGS_REQUIRED[STREAK_NUM_MESSAGES] = {
		10, 8, 6, 4, 2
	};

	//rewards
	int REWARD_ET[] = {EF_CTF_ASSIST, EF_CTF_DEFEND, EF_CTF_CAPTURE};
	const float REWARD_HEAD_ICON_LIFE = 5.0f;

	//
	//Misc
	//
	const float MIN_RESPAWN_TIME = 1.5f;

} //~namespace CTF

//
// Events
//
extern Event EV_CTF_Player_Respawn;
extern Event EV_Player_Dead;
extern Event EV_CTF_Player_DeadStopAnim;

// Update status for CTF game
void Player::CTF_UpdateStatus()
{
	// If this isn't CTF, we dont need to do anything
	if(g_gametype->integer != GT_CTF)
	{
		assert(false);
		return;
	}

	
	if(GetTeam() == TEAM_RED || GetTeam() == TEAM_BLUE)
		client->ps.stats[STAT_TEAM] = static_cast<int>(GetTeam());
	else
		client->ps.stats[STAT_TEAM] = -1;
	
	//frags & caps
	client->ps.stats[STAT_FRAGS]	= num_kills-num_suicides;
	client->ps.stats[STAT_CAPTURES]	= num_captures;


	//team scores
	client->ps.stats[STAT_REDSCORE] = GetCTFManager().GetTeam(TEAM_RED)->GetCaptureCount();
	client->ps.stats[STAT_BLUESCORE] = GetCTFManager().GetTeam(TEAM_BLUE)->GetCaptureCount();
	
	//flag status
	client->ps.stats[STAT_REDFLAGSTATUS] = GetCTFManager().GetTeam(TEAM_RED)->GetFlagStatus();
	client->ps.stats[STAT_BLUEFLAGSTATUS] = GetCTFManager().GetTeam(TEAM_BLUE)->GetFlagStatus();
	client->ps.stats[STAT_GOTFLAG] = (m_pFlag == NULL) ? 0 : 1;

	//time limit
	static float time;

	if(timelimit->value != 0.0f)
	{
		if(level.suddenDeath)
			time = ((timelimit->value*60 + g_suddenDeath->value*60) - (level.time)) / 60.0f;
		else
			time = ((timelimit->value*60) - (level.time)) / 60.0f;

	}
	else
	{
		time = level.time / 60;
	}

	client->ps.stats[STAT_TIMELEFT_MINUTES] = time;
	client->ps.stats[STAT_TIMELEFT_SECONDS] = (time - static_cast<int>(time)) * 60;

	//rewards
	for(REWARD_LIST_ITERATOR it = m_rewards.begin(); it != m_rewards.end(); ++it)
	{
		if(level.time - it->second < CTF::REWARD_HEAD_ICON_LIFE)
			edict->s.eFlags |= CTF::REWARD_ET[it-m_rewards.begin()];
		else
			edict->s.eFlags &= ~CTF::REWARD_ET[it-m_rewards.begin()];
	}

	//tech info
	if(m_techInfoString.length())
	{
		//update stats so client knows about changes
		gi.setConfigstring(CS_TECHINFO, m_techInfoString.c_str());
		gi.SendServerCommand(edict-g_entities, "ctf_updatetechs");

		//reset string so we dont send infostrings each frame
		m_techInfoString = "";
	}
}

// Init for CTF mode
void Player::CTF_Init()
{
	if(g_gametype->integer != GT_CTF)
	{
		assert(false);
		return;
	}

	//health & water
		health			= 100;
		max_health		= 100;
		water_power		= 50;
		max_water_power	= 100;
		SetWaterPower(water_power);

	//call some init bits from Player
		FreeInventory();
		InitClient();
		InitPhysics();
		InitState();
		InitView();
		InitEdict();		
		InitPowerups();
		InitWorldEffects();
		InitSound();
		LoadStateTable();

	//misc
		SetOutfit(0);
		allow_fighting		= qtrue;
		spectator			= false;
		takedamage			= sv_autoAim->integer ? DAMAGE_AIM : DAMAGE_YES;
		m_gibbed			= false;
		m_lastKillTime		= -1;
		m_lastLocationCheck	= -10.0f;
		m_intermissionReady	= false;

	//only spectate if we are not yet on a team
		if(GetTeam() != TEAM_RED && GetTeam() != TEAM_BLUE)
		{
			// Make them a spectator and bring up the team menu
			Spectator();

			//dont show the menu if we are joining a team automatically
			if(!g_teamAutoJoin->integer)
				gi.SendServerCommand(edict-g_entities, "stufftext \"forcemenu ingame_main\"");
		}

		CTF_InitSpawnPoint();

	//get rid of techs
		m_techs.clear();		

	//give the SPIT & greensword
		giveItem("ctf_weapon_spit.tik");
		giveItem("weapon_sword.tik");
		
	//TEMP - use the greensword, just so players dont start empty handed
		Event* ev = new Event("use");
		ev->AddString("GreenSword");
		ev->AddString("right");
		PostEvent(ev, FRAMETIME);

	//make sure we put the player back into the world
		link();
}

//spawns a player at a random spawn point based on his team
void Player::CTF_InitSpawnPoint()
{
	// Get a random spawn point
		Entity* spot = GetCTFManager().GetRandomTeamSpawn(GetTeam());

	//see if we are linked to a ctf_target_give
	//dont give spectators anything
		if(CTF::OnTeam(this) && spot->target.length() > 0)
		{
			Entity* ent = G_FindTarget(NULL, spot->target.c_str());
			
			if(ent->isSubclassOf(CTF_TargetGive))
				static_cast<CTF_TargetGive*>(ent)->GiveItems(this);
		}

	// Setup this spawn point
		setOrigin(spot->origin + Vector(0, 0, 1));
		origin.copyTo(edict->s.origin2);
		setAngles(spot->angles);
		SetViewAngles(spot->angles);
		CameraCut();
		NoLerpThisFrame();
}

// Change the team of the player
void Player::ChangeTeam(Event* ev)
{
	if(GetCTFManager().Intermission())
		return;

	if(!ev || ev->NumArgs() == 0)
	{		
		gi.SendServerCommand(edict-g_entities, "print \"team is %s. Type 'team <team name>' to change teams.\n\"", 
			CTF::TeamName(GetTeam(), false).c_str());
		return;
	}

	//make sure we get our entnum back after following people
		StopFollowing();

	//make sure cameralook is OFF
		gi.SendServerCommand(edict-g_entities, "stufftext \"-cameralook\"");

	//clear rewards
		for(REWARD_LIST_ITERATOR it = m_rewards.begin(); it != m_rewards.end(); ++it)
		{
			it->first = 0;
			it->second= -100;
		}

		gi.SendServerCommand(edict-g_entities, "clear_rewards");

	//the team we want to join
		const char* pszTeam = ev->GetString(1);
		assert(pszTeam);
		if(!pszTeam)
			return;

	// We only need to go by the first char of the team
		switch(pszTeam[0])
		{
			// Free - ie join the team that needs us most
		case 'f':
		case 'F':
			if(GetTeam() == TEAM_RED || GetTeam() == TEAM_BLUE)
				GetCTFManager().LeaveTeam(this);

			GetCTFManager().JoinTeam(this, TEAM_NONE);
			break;

			// Red
		case 'r':
		case 'R':
			if(GetTeam() != TEAM_RED)
			{
				GetCTFManager().LeaveTeam(this);
				GetCTFManager().JoinTeam(this, TEAM_RED);
			}
			else
			{
				CTF::CenterPrint(this, S_COLOR_WHITE "You are already on " S_COLOR_RED "RED");
			}
			break;

			// Blue
		case 'b':
		case 'B':
			if(GetTeam() != TEAM_BLUE)
			{
				GetCTFManager().LeaveTeam(this);
				GetCTFManager().JoinTeam(this, TEAM_BLUE);
			}
			else
			{
				CTF::CenterPrint(this, S_COLOR_WHITE "You are already on " S_COLOR_BLUE "BLUE");
			}
			break;

			// Spectator
		case 's':
		case 'S':
			if(GetTeam() == TEAM_RED || GetTeam() == TEAM_BLUE)
				GetCTFManager().LeaveTeam(this);

			Spectator();
			UpdateStatus("Spectating");
			return;
			break;

			// Bad team
		default:
			gi.SendServerCommand( edict-g_entities, "print \"%s is not a team. Valid teams are [R]ed, [B]lue, [F]ree and [S]pectator.\n\"", pszTeam);
			return;
			break;
		}


	//cancel some events
		CancelEventsOfType(EV_Damage);
		CancelEventsOfType(EV_Pain);
		CancelEventsOfType(EV_Player_Dead);

	//force 3rd person mode & send update
		gi.SendServerCommand(edict-g_entities, "stufftext \"cg_3rd_person 1\"");
		gi.SendServerCommand(edict-g_entities, "stufftext \"popmenu 0\"");
		CTF::UpdateLocationsToAll(edict);
		CTF::UpdateScoresToAll(edict);
}

//
//DropFlag - if we have the flag - drop it!
//
void Player::DropFlag()
{
	if(!m_pFlag)
		return;

	m_pFlag->Droped(this);

	SetFlag(NULL);
}

//
//AddScore
//
void Player::AddScore(int score)
{
	num_kills += score;
}

//
//AddCaps
//
void Player::AddCaps(int caps)
{
	num_captures += caps;
}

//
//CTF_Respawn - called when player respawns
//
void Player::CTF_Respawn(Event* ev)
{
	//make sure we're in ctf...
		if(g_gametype->integer != GT_CTF)
		{
			assert(false);
			return;
		}

	//cancel some events
		CancelEventsOfType(EV_Damage);
		CancelEventsOfType(EV_Pain);
		CancelEventsOfType(EV_Player_Dead);
		CancelEventsOfType(EV_CTF_Player_Respawn);

	//dont allow respawning if in intermisison
		if(GetCTFManager().Intermission())
			return;

	//if we are not dead yet, post another event in the hope that we weill be by then
	//this prevents respawning during death anims
		if(deadflag != DEAD_DEAD || level.time - respawn_time < CTF::MIN_RESPAWN_TIME)
		{
			PostEvent(EV_CTF_Player_Respawn, CTF::MIN_RESPAWN_TIME);
			return;
		}

	//if we are NOT gibbed, spawn a body
		if(!m_gibbed)
			new CTF_DeadBody(this);

	//init player
		ChangeMusic("normal", "normal", false);
		NoLerpThisFrame();
		allow_fighting = qtrue;
		CameraCut();

		CTF_Init();
		showModel();

	//spawn a respawn model
		Animate* fx = new Animate;
		fx->setModel("fx_spitteleport.tik");
		fx->RandomAnimate("idle", EV_Remove);
		fx->setOrigin(origin);

	//killbox
		unlink();
		CTF::ScaledKillBox(this, 1.1f);

	//TEMP:
		gi.SendServerCommand(edict-g_entities, "stufftext \"-cameralook\"");
}

//
//ClientBegin - called on level start
//
void Player::CTF_ClientBegin()
{
	if(g_teamAutoJoin->integer)
	{
		GetCTFManager().JoinTeam(this, TEAM_NONE);
		gi.SendServerCommand(edict-g_entities, "stufftext \"set cg_3rd_person 1\"");
		gi.SendServerCommand(edict-g_entities, "stufftext \"-cameralook\"");
	}

	gi.SendServerCommand(edict-g_entities, "ctf_initlocations");	
	CTF::UpdateScores(edict);
	CTF::UpdateLocations(edict);
}

//
//GetTech - returns tech with given ID. null if we dont have one!
//
CTF_Tech* Player::GetTech(int id) const
{
	for(TECH_LIST_CONSTITERATOR it = m_techs.begin(); it != m_techs.end(); ++it)
	{
		if((*it)->GetID() == id)
			return *it;
	}

	return NULL;
}

//
//GiveTech
//
bool Player::GiveTech(CTF_Tech* tech)
{
	//get our current tech of this type
		CTF_Tech* current = GetTech(tech->GetID());

	//remember if we have this tech already
		bool got = (current != NULL);

	//if we have this tech type already, give it more life
		if(current)
		{
			current->AddLife(tech->GetTimeLeft());
		}
		else
		{
			current = tech;
			m_techs.push_back(current);
		}

	//update tech infostring
		m_techInfoString += va("\\tech\\%d\\time\\%d", current->GetID(), 
			static_cast<int>(current->GetTimeLeft()));

	return got;
}

//
//DropTechs - drop ALL techs
//
void Player::DropTechs()
{
	while(m_techs.empty() == false)
		(*m_techs.begin())->DropTech(NULL);

	gi.setConfigstring(CS_TECHINFO, "clear");
	gi.SendServerCommand(edict-g_entities, "ctf_updatetechs");
}

//
//RemoveTech - removes tech with given id
//
void Player::RemoveTech(int id)
{
	for(TECH_LIST_ITERATOR it = m_techs.begin(); it != m_techs.end(); ++it)
	{
		if((*it)->GetID() == id)
		{
			m_techs.erase(it);
			return;
		}
	}

	gi.setConfigstring(CS_TECHINFO, va("tech\\%d\\time\\-1", id));
	gi.SendServerCommand(edict-g_entities, "ctf_updatetechs");
}

//
//SpawnBloodyGibbs!!!
//
void Player::CTF_SpawnBloodyGibs(Event* ev)
{
	if(!com_blood->integer)
		return;

	//scale - use event arg if given, otherwise just use 1
	float scale;

	if(ev->NumArgs() > 1)
		scale = ev->GetFloat(1);
	else
		scale = 1.0;
	
	//spawn the gibs
	Animate* gib = new Animate;
	gib->setModel("models/fx_playergib.tik");
	gib->setScale(scale);
	gib->setOrigin(centroid);
	gib->RandomAnimate("idle", EV_Remove);
	gib->PostEvent(EV_Remove, 1);
	
	Sound("sound/player/gib.wav", CHAN_AUTO, 1, 300);
}

//
//DamageEvent
//
void Player::CTF_DamageEvent(Event* ev)
{
	//if we are not in ctf mode, use normal damageevent func
		if(g_gametype->integer != GT_CTF)
		{
			Sentient::DamageEvent(ev);
			return;
		}

	//see if we can bail
		if((takedamage == DAMAGE_NO) || (movetype == MOVETYPE_NOCLIP))
			return;

	//extract stuff from event
		Entity	*inflictor	= ev->GetEntity(2);
		Entity	*attacker	= ev->GetEntity(3);
		int		damage		= ev->GetInteger(1);
		int		mod			= ev->GetInteger(9);
		Vector	pos			= ev->GetVector(4);
		Vector  damdir		= ev->GetVector(5);

		Player* player		= NULL;
		if(attacker->isSubclassOf(Player))
			player = static_cast<Player*>(attacker);

	//if we are being hit by a sword (ie the inflictor is a PLAYER), 
	//make the damage location be our centroid
		if(inflictor && inflictor->isSubclassOf(Player))
			pos = centroid;

	//tech damage
		if(player)
			damage *= player->TechDamage(false, pos);

	//do falling damage handling
		if(mod == MOD_FALLING)
		{
			//if we have acro, we do not take falling dmg
			if(TechAcro(false) != 1.0f)
			{
				return;
			}

			//cap falling damage
			else if(sv_maxFallingDamage->integer > 0)
			{
				if(damage > sv_maxFallingDamage->integer)
					damage = sv_maxFallingDamage->integer;
			}
		}

	//dont bother using protection if the attacker isnt a client
		if(attacker && attacker->isClient())
		{
			//tech protection
				damage *= TechProtection(true, pos);

			//tech empathy
				float factor = TechEmpathy(true, pos);
				if(factor > 0.001f)
				{
					if(mod != MOD_EMPATHY && attacker && attacker->isClient())
					{
						Event* damageEvent = new Event(EV_Damage);
						damageEvent->AddInteger(damage*factor);
						damageEvent->AddEntity(inflictor);
						damageEvent->AddEntity(this);
						damageEvent->AddInteger(0);
						damageEvent->AddInteger(0);
						damageEvent->AddInteger(0);
						damageEvent->AddInteger(0);
						damageEvent->AddInteger(0);
						damageEvent->AddInteger(MOD_EMPATHY);

						attacker->ProcessEvent(damageEvent);
					}

					damage -= damage*factor;
				}

			//shield damage reduction
				if(CTF_InShieldRange(ev->GetVector(5)))
				{
					if(CanBlock(mod, LargeShieldActive()))
					{
						Shield* shield = static_cast<Shield*>(GetActiveWeapon(WEAPON_LEFT));
						shield->Impact();							
						damage -= damage * shield->GetDamageReduction();
						
					}
				}				
		}

	//figure momentum add
		if(inflictor && inflictor != this)
		{
			//rocket jumping hack - you knock youself more than others
			const float factor = ((player == this) ? 80.0f : 40.0f);

			Vector vel;
			damdir.normalize();
			VectorScale(damdir, g_knockBack->value * damage * factor, vel);
			VectorAdd(velocity, vel, velocity);
		}

	//if attacking yourself dont take full brunt of the damage
		if(attacker == this)
			damage *= 0.5f;

	//attacker is NOT yourself
		else
		{
			//team play damage avoidance
			if(player && player->GetTeam() == GetTeam())
			{
				if(deadflag == DEAD_NO)
					CTF::LocalSound(player, "sound/ctf/feedback/hitfriend.wav");

				//telefrags still hurt fully
				if(mod != MOD_TELEFRAG)
					damage *= g_friendlyFire->value;
			}

			if(deadflag == DEAD_NO)
				CTF::LocalSound(player, "sound/ctf/feedback/hitenemy.wav");
		}

	//spurt some blood
		if(com_blood->integer && !m_gibbed && attacker && inflictor && !inflictor->isSubclassOf(Projectile))
		{
			static float last = 0;
			if(level.time - last >= 0.1f)
			{
				last = level.time;
				

				float scale = damage / 18.0f;
				if(scale > 1.0f) scale = 1.0f;
				if(scale > 0.1f)
				{
					Animate* bloodeffect = new Animate;
					bloodeffect->setModel("fx_painblood.tik");
					bloodeffect->setOrigin(pos);
					bloodeffect->setScale(scale);
					bloodeffect->RandomAnimate("idle", EV_Remove);
				}
			}
		}

	//check for godmode or invincibility
		if(flags & FL_GODMODE)
			return;

	//
	//do the damage, but dont let health go below -999 cos it looks daft on the hud
	//when things overlap...
	//
		int oldHealth = health;
		health -= damage;
		if(health < -999)
			health = -999;

	//
	//send death/kill/pain events
	//

		Event* event = NULL;

		if(health <= 0)
		{
		//spawn gibs
			CTF_Gib();

		//post a killed message to ourself - since we JUST died due to this damage event
			if(oldHealth > 0)
			{
				event = new Event(EV_Killed);
				event->AddEntity(attacker);
				event->AddInteger(damage);
				event->AddEntity(inflictor);
				event->AddInteger(mod);
				ProcessEvent(event);
			}

		//we dont want to post a pain message, so bail now
			return;
		}

	//give ourself a pain message
	if(!m_gibbed)
	{
		event = new Event(EV_Pain);
		event->AddFloat(damage);
		event->AddEntity(attacker);
		ProcessEvent(event);
	}
}

//
//DoRewards - give appropriate rewards for the death of this player
//
void Player::CTF_DoRewards(Player* attacker)
{
	if(!attacker || attacker == this)
		return;

	//if attacker is on our team, PUNISH!
		if(attacker->GetTeam() == GetTeam())
		{
			attacker->AddScore(CTF::BONUS_KILL_TEAMMATE);

			//TODO: play a sound
			return;
		}



		attacker->AddScore(1);

		//see if we are close to other team's base, if so
		//give the killer a defence bonus
			CTF_Flag* flag = CTF::FindFlag(CTF::OtherTeam(GetTeam()));
			if(flag)
			{
				Vector flagorigin = flag->GetSpawnPos();

				//if distance between dead guy and base is small enough (and they, 
				//are in the same PVS) give the bonus
					if(	gi.inPVS(flagorigin, origin) &&
						CTF::DistanceLess(flag->GetSpawnPos(), origin, CTF::MAX_DEFENCE_BONUS_DIST)
					)
					{
						CTF::HudPrintToAll(NULL, va("%s defends the base\n",
							CTF::GetName(attacker)));

						attacker->AddScore(CTF::BONUS_DEFEND);
						
						//send reward
						attacker->AddReward(CTF::REWARD_DEFENCE);
					}
			}

		//if dead guy had flag, award killer
			if(GetFlag())
			{				
				attacker->AddScore(CTF::BONUS_KILL_FC);

				//TODO: make sure the attacker gets an assist if needed...
			}
}

//
//print death messages
//
void Player::CTF_Obituary(Entity* attacker, Entity* inflictor, int meansofdeath)
{
	if(attacker)
	{
		assert(attacker->isSubclassOf(Player));
	}

	//send console msg to everyone
		meansOfDeath_t mod = static_cast<meansOfDeath_t>(meansofdeath);
		str obituaryMsg = CTF::Obituary().MODString(mod, this, attacker).c_str();

		if(obituaryMsg.length())
			CTF::HudPrintToAll(NULL, obituaryMsg.c_str());


	//print big messages on killer & victim's screens
	//dont bother if this is a self inflicted death (ie no attacker OR attacker is ourself)
	if(attacker && attacker != this)
	{
		CTF::ScoreboardPrint(this, va(S_COLOR_WHITE "Fragged by %s", 
			CTF::GetName(attacker)));
		CTF::ScoreboardPrint(attacker, va(S_COLOR_WHITE  "You fragged %s", 
			CTF::GetName(this)));
	}
}

//
//CTF Replacement for the Player::Killed func
//
void Player::CTF_Killed(Event* ev)
{
	//make sure we're in CTF mode
		if(g_gametype->integer != GT_CTF)
		{
			assert(false);
			return;
		}

	//TEMP:
		gi.SendServerCommand(edict-g_entities, "stufftext \"+cameralook\"");


	//extract info from event
		Player   *attacker		= ev->GetEntity(1)->isSubclassOf(Player) ? static_cast<Player*>(ev->GetEntity(1)) : NULL;
		Entity   *inflictor		= ev->GetEntity(3);
		int      meansofdeath	= ev->GetInteger(4);
		
		this->num_deaths++;

	//tell attacker they got a kill
		if(attacker != NULL && attacker->isClient())
		{
			Event* event = new Event(EV_GotKill);
			event->AddEntity(this);
			event->AddInteger(0);
			event->AddEntity(inflictor);
			event->AddInteger(ev->GetInteger(9));
			event->AddInteger(0);
			attacker->ProcessEvent(event);
		}

	//stop movin
		if(m_gibbed)
			PostEvent(EV_CTF_Player_DeadStopAnim, FRAMETIME);

	//remove crosshair
		CTF_RemoveCrosshair();

	//if we killed ourself, increment suicides
		if(attacker == this || !attacker)
			this->num_suicides++;

	//store killer
		m_lastKiller = attacker;

		pain_type = static_cast<meansOfDeath_t>(meansofdeath);
		
	//print death msg
		Obituary(attacker, inflictor, meansofdeath);
	
	//do general rewards
		CTF_DoRewards(attacker);

	//make sure we dont keep the flag after we die
		if(m_pFlag)
		{
			//check we arent on a nodrop area
			if(gi.pointcontents(origin, 0) & CONTENTS_NODROP)
				m_pFlag->ReturnFlag(NULL);
			else
				DropFlag();
		}

	//drop our tech
		DropTechs();

	//drop weapons
		Weapon* weap = GetActiveWeapon(WEAPON_DUAL);
		if(weap)
			weap->Drop();

		weap = GetActiveWeapon(WEAPON_LEFT);
		if(weap)
			weap->Drop();

		weap = GetActiveWeapon(WEAPON_RIGHT);
		if(weap)
			weap->Drop();

	//post a respawn timer event
		if(sv_forcerespawn->value)
			PostEvent(EV_CTF_Player_Respawn, sv_forcerespawn->value);
	
	//return spit module
		gi.SendServerCommand(edict-g_entities, "stufftext \"ctf_spit_return\"");

	//misc settings
		deadflag = m_gibbed ? DEAD_DEAD : DEAD_DYING;
		
		respawn_time = level.time + 0.5f;
		
		edict->clipmask = MASK_DEADSOLID;
		edict->svflags |= SVF_DEADMONSTER;
		
		setContents(CONTENTS_CORPSE);
		setMoveType(MOVETYPE_NONE);
		
		angles.x = 0;
		angles.z = 0;
		setAngles(angles);

		takedamage = DAMAGE_YES; //do not autoaim to dead bodies
		
	//change music
		ChangeMusic( "failure", "normal", true );
	
	//stop targeting monsters	
		if(left_arm_target)
		{
			left_arm_target->edict->s.eFlags &= ~EF_LEFT_TARGETED;
			left_arm_target = NULL;
		}
	
		if(right_arm_target)
		{
			right_arm_target->edict->s.eFlags &= ~EF_RIGHT_TARGETED;
			right_arm_target = NULL;
		}
	
	//post a dead event just in case
		PostEvent(EV_Player_Dead, 2);
}

//
//manual suicide via console
//
void Player::CTF_Kill(Event* ev)
{
	//if we are not alive, bail, and,
	//cant kill ourself until 3 seconds after respawn
		if(deadflag != DEAD_NO || level.time - respawn_time < 3)
			return;
	
	//turn off godmode
		flags &= ~FL_GODMODE;

	//set health to 1 and give 1000 damage
		health = 1;
	
		Event* damageEvent = new Event(EV_Damage);
		damageEvent->AddInteger(1000);
		damageEvent->AddEntity(this);
		damageEvent->AddEntity(this);
		damageEvent->AddInteger(0);
		damageEvent->AddInteger(0);
		damageEvent->AddInteger(0);
		damageEvent->AddInteger(0);
		damageEvent->AddInteger(0);
		damageEvent->AddInteger(MOD_SUICIDE);
		ProcessEvent(damageEvent);
}

//
//GotKill - called when we kill someone
//
void Player::CTF_GotKill(Event* ev)
{
	//dont award streaks for non player kills
		if(ev->GetEntity(1)->isSubclassOf(Player) == false)
			return;

	//if it is < x seconds since last kill
		if(m_lastKillTime >= 0 && level.time-m_lastKillTime <= CTF::MULTIKILL_AWARD_TIME)
		{
			++m_killStreak;
			
			for(int i=0; i<CTF::STREAK_NUM_MESSAGES; ++i)
			{
				if(m_killStreak >= CTF::STREAK_FRAGS_REQUIRED[i])
					break;
			}

			//if we are on a streak, do some stuff
				if(i < CTF::STREAK_NUM_MESSAGES)
				{
					//only print the message if we have ran out of messages or we have just 
					//qualified for one
						if(m_killStreak >= CTF::STREAK_FRAGS_REQUIRED[0] || m_killStreak == CTF::STREAK_FRAGS_REQUIRED[i])
						{
							CTF::CenterPrint(this, CTF::STREAK_MESSAGES[i].c_str());
							CTF::LocalSound(this, CTF::STREAK_SOUNDS[i].c_str());
						}
				}
		}
		else
		{
			m_killStreak = 1;
		}

		m_lastKillTime = level.time;
}

//
//Disconnect
//
void Player::CTF_Disconnect()
{
	//drop stuff
		DropTechs();
		DropFlag();

	//spawn some eye candy
		Animate* anim = new Animate;
		anim->setOrigin(origin);
		anim->setModel("fx_clientdc.tik");
		anim->RandomAnimate("idle", EV_Remove);

	//do leaving admin bits
		GetCTFManager().LeaveTeam(this);

	//kill our crosshair
		CTF_RemoveCrosshair();

		//tell clients about the disconnect & stop followers following
		TeamIterator iter(TEAM_NONE);
		TeamIterator end;

		for(; iter != end; ++iter)
		{
			gi.SendServerCommand(iter->edict-g_entities, "ctf_singlescore %s \n", 
			va("%i %s %i %i %i %i %i %i %i %i",
				entnum, "", 0, 0, 0, 0, 0, 0, 0, 0)
				);
			if(iter->IsFollowing(this))
				iter->StopFollowing();
		}
}

//
//Add Water - add n units of water to our arsenal
//
void Player::AddWater(float amount)
{
	water_power += amount;

	if(water_power < 0)
		water_power = 0;

	if(water_power > max_water_power)
		water_power = max_water_power;
}

//
//Reset - called when a player changes teams (or any time when frags etc need 
//to be reset)
//
void Player::CTF_Reset()
{
	DropFlag();
	DropTechs();

	num_kills = 0;
	num_deaths = 0;
	num_suicides = 0;
	num_captures = 0;
	m_lastKiller = NULL;
}

//
//TechDamage - returns cumulative damage factor of all techs
//
float Player::TechDamage(bool effect, Vector pos)
{
	float factor = 1.0f;

	for(TECH_LIST_ITERATOR it = m_techs.begin(); it != m_techs.end(); ++it)
	{
		if((*it)->GetDamage() != 1.0f)
		{
			factor *= (*it)->GetDamage();

			if(effect)
			{
				Sound((*it)->GetUseSnd(), CHAN_AUTO);
			}
		}
	}

	return factor;
}

//
//TechProtection - returns cumulative protection factor of all techs
//
float Player::TechProtection(bool effect, Vector pos)
{
	float factor = 1.0f;
	
	for(TECH_LIST_ITERATOR it = m_techs.begin(); it != m_techs.end(); ++it)
	{
		if((*it)->GetProtection() != 1.0f)
		{
			factor *= (*it)->GetProtection();
			
			if(effect)
				Sound((*it)->GetUseSnd(), CHAN_AUTO);
		}
	}

	return factor;
}

//
//TechEmpathy - returns cumulative protection factor of all techs
//
float Player::TechEmpathy(bool effect, Vector pos)
{
	float factor = 1.0f;
	bool gotFactor = false;

	for(TECH_LIST_ITERATOR it = m_techs.begin(); it != m_techs.end(); ++it)
	{
		if((*it)->GetEmpathy() > 0.001f)
		{
			factor *= (*it)->GetEmpathy();
			
			if(effect)
				Sound((*it)->GetUseSnd(), CHAN_AUTO);

			gotFactor = true;
		}
	}

	return gotFactor ? factor : 0.0f;
}

//
//TechHaste - returns cumulative haste factor of all techs
//
float Player::TechAcro(bool effect, Vector pos)
{
	float factor = 1.0f;
	bool gotFactor = false;

	for(TECH_LIST_ITERATOR it = m_techs.begin(); it != m_techs.end(); ++it)
	{
		if((*it)->GetAcro() != 1.0f)
		{
			factor *= (*it)->GetAcro();
			gotFactor = true;

			if(effect)
				Sound((*it)->GetUseSnd(), CHAN_AUTO);
		}
	}

	return gotFactor ? factor : 1.0f;
}

//
//Gib - checks health, and gibs if it's low enough
//
bool Player::CTF_Gib()
{
	//should we gib
	if(!m_gibbed && health < -40)
	{
		ProcessEvent(EV_Sentient_SpawnBloodyGibs);

		hideModel();
		setMoveType(MOVETYPE_NONE);
		setSolidType(SOLID_NOT);
		velocity = vec_zero;
		avelocity = vec_zero;

		deadflag = DEAD_DEAD;
		m_gibbed = true;

		return true;
	}

	return false;
}

//
//removes our crosshair
//
void Player::CTF_RemoveCrosshair()
{
	if(crosshair)
		crosshair->hideModel();
}

//
//Completely stop moving & animating
//
void Player::CTF_DeadStopAnim(Event* ev)
{
	//stop animating and zero out our speeds
		ProcessEvent(EV_StopAnimating);
		setMoveType(MOVETYPE_NONE);
		setSolidType(SOLID_NOT);

		velocity  = vec_zero;
		avelocity = vec_zero;
}

//
//Think
//
void Player::CTF_ClientThink()
{
	//Intermission 'READY' status
	if(GetCTFManager().Intermission() && CTF::OnTeam(this))
	{
		if(buttons & BUTTON_ATTACKLEFT || buttons & BUTTON_USE)
		{
			m_intermissionReady = true;
			gi.SendServerCommand(edict-g_entities, "intermissionready 1");
		}
		else if(buttons & BUTTON_ATTACKRIGHT)
		{
			m_intermissionReady = false;
			gi.SendServerCommand(edict-g_entities, "intermissionready 0");
		}
	}

	//follow ents
	if( (GetTeam() == TEAM_NONE) && (buttons & BUTTON_ATTACKLEFT) )
	{
		FollowNext();
	}

	//stop following	
	if( (GetTeam() == TEAM_NONE) && (buttons & BUTTON_ATTACKRIGHT) && (m_followEnt >= 0) )
	{
		StopFollowing();
	}

}

//
//Add a reward
//
void Player::AddReward(int rewardType, int count)
{
	if(rewardType < 0 || rewardType >= static_cast<int>(CTF::REWARD_MAX))
	{
		assert(false);
		return;
	}

	m_rewards[rewardType].first += count;
	m_rewards[rewardType].second = level.time;

	//tell client
	gi.SendServerCommand(edict-g_entities, "addreward %d %d", rewardType, count);
}

//
//follow given player (ie look out of their eyes)
//
void Player::Follow(Player* player)
{
	Camera* cam = new Camera;
					
	Event* ev = new Event("follow");
	ev->AddEntity(player);
	cam->ProcessEvent(ev);
	cam->Cut(NULL);
	SetCamera(cam, 0);	

	m_followEnt = player->entnum;
	gi.SendServerCommand(edict-g_entities, "followent %d", m_followEnt);
}

//
//return whether or not we are following given player
//
bool Player::IsFollowing(Player* player)
{
	return entnum == player->entnum;
}

//
//stop following, return to normal player cam
//
void Player::StopFollowing()
{
	SetCamera(NULL, 0);
	m_followEnt = -1;
	gi.SendServerCommand(edict-g_entities, "followent -1");
}

//
//Follows the next player, with optional team checking
//
void Player::FollowNext(teamtype_t team)
{
	int stop = m_followEnt < 0 ? MAX_CLIENTS : m_followEnt;
	int i = m_followEnt+1;

	while(i != stop)
	{
		if(g_entities[i].entity && g_entities[i].entity->isSubclassOf(Player))
		{
			Player* follow = static_cast<Player*>(g_entities[i].entity);

			if(team != TEAM_NONE && follow->GetTeam() != team)
				return;

			if(follow != this && CTF::OnTeam(follow))
			{
				Follow(follow);
				return;
			}
		}

		if(++i > MAX_CLIENTS)
			i = 0;
	}
}

//
//InShieldRange - return true if the given position vector falls in the blockable area
//of the current shield
//
bool Player::CTF_InShieldRange(Vector& pos)
{
	Weapon* weap = GetActiveWeapon(WEAPON_LEFT);
	float block_size = 0.0f;
	
	if(weap && weap->isSubclassOf(Shield))
	{

		if(LargeShieldActive())
			block_size = 90;
		else if(ShieldActive())
			block_size = 45;

		Vector player_angle = GetTorsoAngles();
		Vector attack_angle = pos.toAngles();
		float yaw_diff		= player_angle[YAW] - attack_angle[YAW] + 180.0f;
		
		if(AngleNormalize180(yaw_diff) < block_size)
			return true;
	}

	return false;
}
