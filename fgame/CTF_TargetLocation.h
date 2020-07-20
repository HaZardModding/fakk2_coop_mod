///////////////////////////////////////////////////////////////////////////////
//
//
// target_location entity
// Used in ctf games to evaluate 'where' a player is
//
// -Yorvik
//
///////////////////////////////////////////////////////////////////////////////

#include "Entity.h"

class CTF_TargetLocation : public Entity
{
	CLASS_PROTOTYPE(CTF_TargetLocation);

	str m_location;
	int m_locationNum;
	
public:
	CTF_TargetLocation();
	~CTF_TargetLocation();

	str  GetLocationString();
	void SetLocationString(Event* ev);

	void SetLocationNum(int n)	{m_locationNum = n;}
	int  GetLocationNum()		{return m_locationNum;}
};