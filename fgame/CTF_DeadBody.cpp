//
//Dead Body class implementation.
//
//These are spawned when a player dies - they take damage and spray blood/gib accordingly :)
//
//-Yorvik
//

#include "CTF_DeadBody.h"
#include "Player.h"

CLASS_DECLARATION(Actor, CTF_DeadBody, NULL)
{
	{&EV_Damage,			CTF_DamageEvent},
	{&EV_TakeDamage,		CTF_TakeDamageEvent},
	{&EV_FadeOut,			CTF_FadeOut},
	{NULL, NULL}
};

//
//Constructor
//
CTF_DeadBody::CTF_DeadBody() : Actor(), m_lastBloodSpray(0.0f)
{
	health = 0;
	takedamage = DAMAGE_NO;
	PostEvent(EV_FadeOut, 20);
	PostEvent(EV_TakeDamage, FRAMETIME);
}

//
//Constructor - calls InitFromPlayer
//
CTF_DeadBody::CTF_DeadBody(Player* src) : Actor(), m_lastBloodSpray(0.0f)
{
	if(!src)
	{
		ProcessEvent(EV_Remove);
		return;
	}

	edict->s.number = edict - g_entities;
	if(src->edict->s.groundEntityNum == ENTITYNUM_NONE)
	{
		edict->s.pos.trType = TR_GRAVITY;
		edict->s.pos.trTime = level.time;
		velocity = src->velocity;
	}


	flags &= ~FL_AUTOAIM; //dont aim at dead bodies
	health = src->health;
	takedamage = DAMAGE_NO;
	deadflag = DEAD_DEAD;
	PostEvent(EV_FadeOut, 20);
	PostEvent(EV_TakeDamage, FRAMETIME);

	InitFromPlayer(src);
}

//
//TakeDamageEvent - makes this ent take damage
//
void CTF_DeadBody::CTF_TakeDamageEvent(Event* ev)
{
	takedamage = DAMAGE_YES;
	flags &= ~FL_AUTOAIM; //dont aim at dead bodies
}

//
//DamageEvent - spew blood or gib, depending on how much health we have
//
void CTF_DeadBody::CTF_DamageEvent(Event* ev)
{
	int		damage		= ev->GetInteger(1);
	Entity	*inflictor	= ev->GetEntity(2);
	Vector	pos			= ev->GetVector(4);
	health  -= damage;

	//if we are being hit by a sword (ie the inflictor is a PLAYER), 
	//make the damage location be our centroid
		if(inflictor && inflictor->isSubclassOf(Player))
			pos = centroid;

	//
	//SPRAY BLOOD
	//

	//if blood is OFF, bail
	if(!com_blood->integer)
		return;

	if(health > -40)
	{
		if(level.time - m_lastBloodSpray < 0.1f)
			return;

		m_lastBloodSpray = level.time;
		
		float scale = damage / 15.0f;
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

	//
	//GIB
	//
	else
	{
		//spawn a bunch of gibs
			Animate* ev = new Animate;
			ev->setModel("models/fx_playergib.tik");
			ev->setScale(1.0f);
			ev->setOrigin(centroid);
			ev->RandomAnimate("idle", EV_Remove);
			ev->PostEvent(EV_Remove, 1);

		//sound
			Sound("sound/player/gib.wav", CHAN_AUTO, 1, 300);

		//remove the dead body from the world
			ProcessEvent(EV_Remove);
	}
}

//
//InitFromPlayer - set our data to mimic the given player
//
void CTF_DeadBody::InitFromPlayer(Player* srcPlayer)
{
	setOrigin(srcPlayer->origin);
	setModel(srcPlayer->model);
	setAngles(srcPlayer->angles);
	setContents(CONTENTS_CORPSE);
	SetOutfit(srcPlayer->GetOutfit());

	edict->s.anim = srcPlayer->edict->s.anim;
	edict->s.frame = srcPlayer->edict->s.frame;
}

//
//SetOutfit - shows / hides appropriate appendages (shoulderpad etc)
//
void CTF_DeadBody::SetOutfit(int stage)
{
	SurfaceCommand("armpad*",			"+nodraw");
	SurfaceCommand("holster*",			"+nodraw");
	SurfaceCommand("pouch",				"+nodraw");
	SurfaceCommand("kneepad*",			"+nodraw");
	SurfaceCommand("shoulderpad*",		"+nodraw");
	SurfaceCommand("glasses",			"+nodraw");

	if(stage >= 5)
		SurfaceCommand( "glasses",		"-nodraw" );
	if(stage >= 4)
		SurfaceCommand( "shoulderpad*", "-nodraw" );
	if(stage >= 3)
		SurfaceCommand( "kneepad*",		"-nodraw" );
	if(stage >= 2)
		SurfaceCommand( "armpad*",		"-nodraw" );
	if(stage >= 1)
	{
		SurfaceCommand( "holster*",		"-nodraw" );
		SurfaceCommand( "pouch",		"-nodraw" );
	}
}

//
//FadeOut - slowly remove the dead body from the world
//
void CTF_DeadBody::CTF_FadeOut(Event* ev)
{
	float myscale = edict->s.scale;
	myscale -= 0.003f;
	
	if(myscale < 0)
		myscale = 0;
	
	if(myscale <= 0)
	{
		ProcessEvent(EV_Remove);
		return;
	}
	
	PostEvent(*ev, FRAMETIME);
	setScale(myscale);
}