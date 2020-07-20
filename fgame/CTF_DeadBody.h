#if !defined(CTF_DEADBODY_H_INCLUDED)
#define CTF_DEADBODY_H_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//
//Dead Body interface.
//
//These are spawned when a player dies - they take damage and spray blood/gib accordingly :)
//
//-Yorvik
//

#include "Actor.h"
class Player;

class CTF_DeadBody : public Actor
{
	float	m_lastBloodSpray;
	CLASS_PROTOTYPE(CTF_DeadBody);

	void SetOutfit(int stage);

public:
	CTF_DeadBody();
	CTF_DeadBody(Player* src);
	~CTF_DeadBody()	{}

	void InitFromPlayer(Player* srcPlayer);
	virtual void CTF_DamageEvent(Event* ev);
	virtual void CTF_TakeDamageEvent(Event* ev);
	virtual void CTF_FadeOut(Event* ev);
};

#endif // ~CTF_DEADBODY_H_INCLUDED
