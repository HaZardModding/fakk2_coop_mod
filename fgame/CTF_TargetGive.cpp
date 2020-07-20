//
//CTF_TargetGive - link player starts to this and any player that spawns from
//that spot will be 'given' everything targetted by this ent
//
//-Yorvik
//

#include "CTF_TargetGive.h"
#include "Player.h"

CLASS_DECLARATION(Entity, CTF_TargetGive, "ctf_target_give")
{
	{NULL, NULL}
};

//
//Gives the player that spawned at the linked player start all the stuff this give is linked to
//
void CTF_TargetGive::GiveItems(Player* player)
{
	//if we are given a bong player OR we are not targetting any other ents, bail
		if(!player || target.length() <= 0)
			return;

	//find first targetted ent
		Entity* ent = G_FindTarget(NULL, target.c_str());


	//find the rest and give each one to the player
		while(ent)
		{
			//give the ent to the player
				Event* ev = new Event(EV_Item_Pickup);
				ev->AddEntity(player);
				ent->ProcessEvent(ev);

			//get next targetted ent
				ent = G_FindTarget(ent, target.c_str());
		}
}