///////////////////////////////////////////////////////////////////////////////
//
//
// CTF_Spit Weapon class interface
//
// -Yorvik
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CTF_Spit_H_DEFINED
#define CTF_Spit_H_DEFINED

#include "weapon.h"
#include "weaputils.h"

//
//CLASS: CTF_SpitModule - The disc that is fired by the CTF_Spit
//
class CTF_SpitModule : public Projectile
{
	CLASS_PROTOTYPE(CTF_SpitModule);

	bool		m_dead;			//has the module been disabled?
	Entity*		m_killer;		//ent that killed the module
	str			m_returnModel;	//filename of tiki used for return effect

	bool		CheckPlayerFit(Player* player);

public:
	CTF_SpitModule();
	virtual ~CTF_SpitModule();

	
	virtual void DamageEvent(Event* ev);
	//virtual void Touch(Event *ev);
	virtual void CTF_TakeDamage(Event* ev);
	virtual void DummyTakeDamage(Event* ev);
			void Return(Event* ev);
			void SetReturnModel(Event* ev);

	bool	Dead()				const	{return m_dead;}		//is the module dead?
	Entity* GetKiller()			const	{return m_killer;}		//get pointer to the modules murderer!
	str		GetReturnModel()	const	{return m_returnModel;}	//return tiki filename of return anim
};



//
//CLASS: CTF_Spit - The CTF_Spit weapon
//
class CTF_Spit : public Weapon
{
	CLASS_PROTOTYPE(CTF_Spit);

	//pointer to the module we previously fired
	//...and is set to NULL when we teleport to it
	CTF_SpitModule*	m_firedModule;

	//filename of tiki used for teleport effect
	str			m_teleportModel;

public:
	CTF_Spit();
	virtual ~CTF_Spit();

	void ReturnModule	(bool spawnEffect = false);
	void Teleport		();
	virtual void Shoot	(Event* ev);

	void SetTeleportModel(Event* ev);
	bool GotModule() const {return m_firedModule == NULL;}
};

#endif //CTF_Spit_H_DEFINED