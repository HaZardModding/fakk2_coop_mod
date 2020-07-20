#include "g_local.h"
#include "entity.h"

#pragma warning(push, 1)
#include <string>
#include <vector>
#include <functional>
#pragma warning(pop)

extern Event EV_CTF_Spawner_SpawnItem;




class CTF_ItemSpawner : public Item
{
	CLASS_PROTOTYPE(CTF_ItemSpawner);	
	typedef std::vector<std::string> StringList;
	
	StringList		m_items;
	StringList		m_commands;
	float			m_random;
	float			m_delay;

	Vector GetRandomOrigin();
	void PopulateStringList(StringList& list, Event* ev);

public:
	CTF_ItemSpawner();
	~CTF_ItemSpawner();

	virtual void AddSpawnItems(Event* ev);
	virtual void SpawnRandomItem(Event* ev);
	virtual void SetDelay(Event* ev);
	virtual void SetRandom(Event* ev);
	virtual void SetCommands(Event* ev);

	void ItemPickedUp();
};