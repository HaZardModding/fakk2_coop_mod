#include "cg_local.h"
#include "CTF_ItemPickup.h"
#include "CTF_CG_DrawTools.h"
#include <algorithm>

extern "C"{
	
	void CTF_AddItemInfo(const char* className, qhandle_t iconHandle)
	{
		CTF::GetPickupManager().AddItemInfo(className, iconHandle);
	}

	void CTF_ItemPickup(const char* className, const char* idName)
	{
		CTF::GetPickupManager().ItemPickedUp(className, idName);
	}

}//~extern "C"

namespace CTF
{
	const float ICON_SIZE		= 32.0f;
	const int	ICON_LIFE		= 3000;		//in msecs
	const int   ICON_FADEDELAY	= 2000;		//in msecs
	const float ICON_X			= 40.0f;	//x position of pickup icon
	const float ICON_NAME_X		= ICON_X + ICON_SIZE + 5.0f;

//singleton
CTF_PickupManager& GetPickupManager()
{
	return CTF_PickupManager::Instance();
}


//add some item info to our map
void CTF_PickupManager::AddItemInfo(const char* className, qhandle_t iconHandle)
{
	ItemInfoMapIterator it = m_items.find(className);
	if(it == m_items.end())
	{
		m_items.insert(std::make_pair(std::string(className), qhandle_t(iconHandle)));
		return;
	}

	//we already have info for this item
	it->second = iconHandle;
}

//add a item that we picked up to our pickup list
void CTF_PickupManager::ItemPickedUp(const char* className, const char* idName)
{
	if(!className)
		return;

	ItemInfoMapIterator it = m_items.find(className);
	if(it == m_items.end())
	{
		AddItemInfo(className, 0);
		it = m_items.find(className);
	}

	pickup_t pickup;
	pickup.birthTime = cg.time;
	pickup.className = className;
	pickup.iconHandle = it->second;
	pickup.idName = idName;

	m_pickups.push_front(pickup);
}

//update, remove old pickups etc
void CTF_PickupManager::Update()
{
	for(PickupListIterator it = m_pickups.begin(); it != m_pickups.end();)
	{
		if(cg.time - it->birthTime > ICON_LIFE)
		{
			PickupListIterator tmp = it;
			++it;
			m_pickups.erase(tmp);
		}
		else
		{
			++it;
		}
	}
}

//draw items
void CTF_PickupManager::DrawItems()
{
	Update();

	float alpha = 0;
	float scale = 1.0f;
	float y = SCREEN_H - HUD_BAR_HEIGHT - ICON_SIZE - 10.0f;

	for(PickupListConstIterator it = m_pickups.begin(); it != m_pickups.end(); ++it)
	{
		//only fade out after a certain amount of time
		alpha = 1.0f - (static_cast<float>((cg.time - it->birthTime) - ICON_FADEDELAY) / static_cast<float>(ICON_LIFE-ICON_FADEDELAY));
		
		if(alpha > 1.0f)
			alpha = 1.0f;

		SetColor(1,1,1,alpha);

		//draw the icon
		if(it->iconHandle != 0)
			DrawPic(ICON_X, y, ICON_SIZE, ICON_SIZE, 0, 0, 1, 1, it->iconHandle);

		//draw the name
		DrawString(
			it->idName.c_str(), 
			ICON_NAME_X, y + ICON_SIZE/2 - FONT_HEIGHT*scale/2, 
			scale, true, alpha
		);

		y -= (ICON_SIZE + 2.0f);
	}
}

}//~namespace CTF