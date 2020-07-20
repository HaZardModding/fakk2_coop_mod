///////////////////////////////////////////////////////////////////////////////
//
//
// CTF Techs
//
// -Yorvik
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CTF_TECH_H_DEFINED
#define CTF_TECH_H_DEFINED

#if _MSC_VER > 1000
#pragma once
#endif

#include "ctf_inventoryitem.h"

extern Event EV_CTF_Tech_Drop;
class Animate;

//
//CTF_Tech - BASE
//
class CTF_Tech : public CTF_InventoryItem  
{
	CLASS_PROTOTYPE(CTF_Tech);

	float	m_empathyFactor;		//how much an attacker get's hurt
	float	m_damageFactor;			//how much damage our weapons give
	float	m_protectionFactor;		//how much damage we take
	float	m_acroFactor;			//how much more acrobatic we are (running & jumping)
	float	m_healthRegenAmount;	//how much health we regenerate each shot
	float	m_waterRegenAmount;		//how much water we regenerate each shot
	float	m_delay;				//seconds between custom event trigger

	str		m_useSnd;				//sound played when the tech is 'used'
	str		m_pickupSnd;			//sound played when the tech is picked up
	str		m_dieSnd;				//sound played a few sec's before tech runs out
	str		m_constantEffect;		//effect that is constantly around player when we have this tech
	Animate* m_effect;

	str		m_shader;				//the shader used to draw the icon on the hud

	float	m_duration;				//number of seconds the tech lasts
	float	m_lifeleft;				//life left (seconds)

	int		m_id;					//unique id # of this tech

	float	m_scale;

public:
	CTF_Tech();
	virtual ~CTF_Tech();

	void CTF_DisplayPickup(Entity* ent = NULL);

	virtual void SetScale(Event* ev);
	virtual void Pickup(Event* ev);
	virtual void Respawn(Event* ev);
	virtual void DropTech(Event* ev);
	virtual int  TechDamage(Event* ev, int dmg);
	virtual void TechTrigger(Event* ev);
	virtual void TriggerSecond(Event* ev);
	virtual void Shrink(Event* ev);
	virtual void Grow(Event* ev);

	virtual void SetPickupSound(Event* ev);
	virtual void SetUseSound(Event* ev);
	virtual void SetDieSound(Event* ev);
	virtual void SetEffect(Event* ev);

	virtual void SetDuration(Event* ev);
	virtual void SetID(Event* ev);
	virtual void SetEmpathyFactor(Event* ev);
	virtual void SetDamageFactor(Event* ev);
	virtual void SetProtectionFactor(Event* ev);
	virtual void SetAcroFactor(Event* ev);
	virtual void SetHealthRegenAmount(Event* ev);
	virtual void SetWaterRegenAmount(Event* ev);
	virtual void SetDelay(Event* ev);

	int		GetID()			const {return m_id;}
	float	GetTimeLeft()	const {return m_lifeleft;}
	float	GetDuration()	const {return m_duration;}
	float	GetDamage()		const {return m_damageFactor;}
	float	GetProtection()	const {return m_protectionFactor;}
	float	GetEmpathy()	const {return m_empathyFactor;}
	float	GetAcro()		const {return m_acroFactor;}

	str		GetUseSnd()		const {return m_useSnd;}
	str		GetDieSnd()		const {return m_dieSnd;}
	str		GetShader()		const {return m_shader;}

	void	AddLife(float secs)	  {m_lifeleft += secs;}
};

#endif //CTF_TECH_H_DEFINED