//
//Description
//
#include "CTF_Obituary.h"
#include "CTF_Global.h"
#pragma warning(push, 1)
#include <vector>
#pragma warning(pop)

//singleton implementation
namespace CTF{
CTF_Obituary& Obituary()
{
	return CTF_Obituary::Instance();
}
} //~namespace CTF


//
//Construction / Destruction
//
CTF_Obituary::CTF_Obituary()
{
	LoadMODsFromFile("global/meansofdeath.scr");
}

//
//Read mod messages from given file
//
//Messages in format:
//"mod type string"		"message for this death type"
//
void CTF_Obituary::LoadMODsFromFile(const char* filename)
{
	Script file;
	file.LoadFile(filename);

	str modString = "";
	str obituary = "";
	int i = 0;

	while(file.TokenAvailable(qtrue))
	{
		//get next MOD
			modString = file.GetString(qtrue);

		//find the int representation of this MOD
			for(i = 0; i<MOD_TOTAL_NUMBER; ++i)
			{
				if(strcmpi(means_of_death_strings[i], modString.c_str()) == 0)
					break;
			}

			if(i >= MOD_TOTAL_NUMBER)
				continue;

		//read rest of line into 'obituary'
			if(!file.TokenAvailable(qtrue))
				break;

			obituary = file.GetString(qtrue);			

			if(obituary.length() <= 0)
				continue;

		//insert this data into our map
			m_MODStrings.insert(std::make_pair(static_cast<meansOfDeath_t>(i), obituary));
	}
}

//
//Scan our list of mod messages, and randomly pick one of the matches for the given 'mod'
//
std::string CTF_Obituary::MODString(meansOfDeath_t mod, Entity* victim, Entity* attacker)
{
	//we must always have a victim
		if(!victim)
		{
			assert(victim);
			return str("");
		}

	//find first match for 'mod' in our multimap
		MODStringIterator it = m_MODStrings.find(mod);
		
	//if we have no death msg's for this 'mod', spew a generic crappy one
		if(it == m_MODStrings.end())
		{
			if(attacker)
			{
				if(attacker == victim)
				{
					// suicide - can this ever happen? or does 'attacker' get droped
					// when they are the same?

					return va("%s committed suicide", CTF::GetName(victim));
				}
				else
				{
					// normal kill
					return str(va("%s killed %s", 
								CTF::GetName(attacker),
								CTF::GetName(victim)));
				}
			}
			else
			{
				// dunno wtf happend
				return str(va("%s died", CTF::GetName(victim)));
			}
		}

	//
	//replace $victim$ and $attacker$ with attacker and victims real names
	//

	//choose a random obituary for this MOD
		std::advance(it, rand() % m_MODStrings.count(mod));
		
	//make a temp copy of the MOD string since we will be replacing bits of it in a moment
		std::string buf = it->second;

	//insert attackers name in place of any instances of string "$attacker$"
		if(attacker)
		{
			const std::string attackerName = CTF::GetName(attacker);
			buf = CTF::Replace(buf, "$attacker$", attackerName);
		}

	//insert victims name in place of any instances of string "$victim$"
		buf = CTF::Replace(buf, "$victim$", CTF::GetName(victim));
	
	//return the ammended string and make sure we revert color back to white
		return buf + S_COLOR_WHITE;
}
