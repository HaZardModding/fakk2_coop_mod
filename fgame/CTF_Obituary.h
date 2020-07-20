//
//Description
//

#ifndef CTF_OBITUARY_H_INCLUDED
#define CTF_OBITUARY_H_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif

#include "g_local.h"

#pragma warning(push, 1)
	#include <map>
	#include <string>

	typedef std::multimap<meansOfDeath_t, std::string>	MODStringMap;
	typedef MODStringMap::iterator						MODStringIterator;
#pragma warning(pop)




class CTF_Obituary
{
	MODStringMap m_MODStrings;

	CTF_Obituary();
	void LoadMODsFromFile(const char* filename);

public:
	~CTF_Obituary() {}

	std::string MODString(meansOfDeath_t mod, Entity* victim, Entity* attacker);

	static CTF_Obituary& Instance()
	{
		static CTF_Obituary obi;
		return obi;
	}
};


namespace CTF{

CTF_Obituary& Obituary();

} //~namespace CTF

#endif //CTF_OBITUARY_H_INCLUDED