#ifndef ITEMPICKUP_H
#define ITEMPICKUP_H

#pragma warning(disable: 4786)
#pragma warning(push, 1)
	#include <map>
	#include <list>
	#include <string>
#pragma warning(pop)

namespace CTF{

class CTF_PickupManager
{
	CTF_PickupManager() {}

	//map that holds a collection of handles to pics indexed by the classname of the item
	//it is a picture of.
	typedef std::map<std::string, qhandle_t>	ItemInfoMap;
	typedef ItemInfoMap::iterator				ItemInfoMapIterator;


	struct pickup_t
	{
		std::string className;		//name of item we picked up
		std::string idName;			//descriptive name of the item
		qhandle_t	iconHandle;		//handle to the icon
		int			birthTime;		//cg.time when we picked up the item
	};

	//list of currently visible items (ie ones we recently picked up)
	typedef std::list<pickup_t>					PickupList;
	typedef PickupList::iterator				PickupListIterator;
	typedef PickupList::const_iterator			PickupListConstIterator;

	ItemInfoMap m_items;
	PickupList	m_pickups;

	void Update();

public:
	~CTF_PickupManager() {}

	void AddItemInfo(const char* className, qhandle_t iconHandle);
	void ItemPickedUp(const char* className, const char* idName);
	void DrawItems();

	static CTF_PickupManager& Instance()
	{
		static CTF_PickupManager man;
		return man;
	}
};

//singleton
CTF_PickupManager& GetPickupManager();


} //~namespace CTF

#endif