//
//CTF_TargetGive - link player starts to this and any player that spawns from
//that spot will be 'given' everything targetted by this ent
//
//-Yorvik
//

#ifndef TARGETGIVE_H_INCLUDED
#define TARGETGIVE_H_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif

#include "g_local.h"
#include "entity.h"
class Player;

class CTF_TargetGive : public Entity
{
	CLASS_PROTOTYPE(CTF_TargetGive);

public:
	void GiveItems(Player* player);
};

#endif //TARGETGIVE_H_INCLUDED