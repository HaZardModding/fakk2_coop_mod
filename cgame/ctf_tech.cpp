/////////////////////////////////////////////////////////////////////
//
// client tech bits
//
// -Yorvik
//
/////////////////////////////////////////////////////////////////////

#pragma warning(disable:4786)

#pragma warning(push, 1)
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include "str.h"
#pragma warning(pop)

#include "cg_local.h"
#include "ctf_tech.h"
#include "ctf_cg_drawtools.h"

typedef std::pair<int, str>			StringKeyPair;
typedef std::vector<StringKeyPair>	StringMap;
typedef StringMap::iterator			StringMapIterator;

typedef std::pair<int, int>			TechInfoPair; //id, timeleft
typedef std::list<TechInfoPair>		TechList;
typedef TechList::iterator			TechListIterator;



namespace CTF{

const float	TECH_ICON_WIDTH	= 32.0f;
const float	TECH_ICON_HEIGHT= 32.0f;

struct IsKey
{
	IsKey(int nKey) : m_nKey(nKey)
	{}

	bool operator() (const StringKeyPair& pair) const
	{
		return pair.first == m_nKey;
	}

	
private:
	int m_nKey;
};



//
//store tech icon shader names in a map indexed by integers (ie the tech ID).
//the current tech stat (STAT_TECH) stores the id of the current tech.
//
//this map is populated automatically during TIKI file parsing
//
StringMap ctf_techIcons;

//
//list of all techs we have right now
//
TechList ctf_techs;


//
//return info for tech of type 'id'
//
TechListIterator GetTech(int id)
{
	for(TechListIterator it = ctf_techs.begin(); it != ctf_techs.end(); ++it)
	{
		if(it->first == id)
			return it;
	}

	return ctf_techs.end();
}

//
//UpdateTechs
//
void UpdateTechs()
{
	static int lastTime = 0;

	//
	//loop through each tech updating their timeleft values and removing depleted ones
	//
	for(TechListIterator techIT = ctf_techs.begin(); techIT != ctf_techs.end();)
	{
		if(techIT->second < 0)
		{
			++techIT;
			continue;
		}
		
		//if timeleft is < 0, remove this tech
		if(cg.snap->serverTime >= techIT->second)
		{
			TechListIterator tmp = techIT;
			++techIT;
			ctf_techs.erase(tmp);
		}
		else
		{
			++techIT;
		}
	}

	//DRAW
	DrawTechIcons();
}

//
//GetTechIcon - return shader name given a tech ID
//
const char* GetTechIcon(int id)
{
	StringMapIterator it = std::find_if(ctf_techIcons.begin(), ctf_techIcons.end(), IsKey(id));

	if(it == ctf_techIcons.end())
		return NULL;
	else
		return it->second.c_str();
}

//
//Draw tech icon and time left in the tech to the client's hud
//
void DrawTechIcons()
{
	int handle = 0;
	
	float x = 2;
	float y = SCREEN_H - HUD_BAR_HEIGHT - TECH_ICON_HEIGHT * 4.0f;

	for(TechListIterator techIT = ctf_techs.begin(); techIT != ctf_techs.end(); ++techIT)
	{
		//is this very wasteful - calling registershader once per frame?
		const char* techShader = GetTechIcon(techIT->first);
		
		if(!techShader)
			continue;

		//get shader
		handle = cgi.R_RegisterShaderNoMip(techShader);
		if(handle <= 0)
			continue;

		//set color and draw the pic
		cgi.R_SetColor(colorWhite);	
		DrawPic(x, y, TECH_ICON_WIDTH, TECH_ICON_HEIGHT, 0, 0, 1, 1, handle);

		//draw timeleft text
		if(techIT->second >= 0)
		{
			int time = static_cast<int>((techIT->second - cg.snap->serverTime) / 1000.0f);

			if(time < CTF_TECH_WARNING_TIME)
				CTF::SetColor(1.0f, 0.1f, 0.1f, 1.0f);
			else
				CTF::SetColor(0.7f, 1.0f, 0.7f, 1.0f);

			if(time > 99)
				time = 99;

			DrawString(
				va("%s%d", time < 10 ? "0" : "", time),
				x, y + TECH_ICON_HEIGHT + 8, 1.0f, true
			);
		}

		//update y coord for next tech icon
		y -= (TECH_ICON_HEIGHT + FONT_HEIGHT + 12);
	}
}

//
//set's a tech's shader name
//
void SetTechIconShader(int id, const char* iconshadername)
{
	StringMapIterator iter = std::find_if(ctf_techIcons.begin(), ctf_techIcons.end(), IsKey(id));

	if(iter != ctf_techIcons.end())
	{
		iter->second = iconshadername;
	}
	else
	{
		ctf_techIcons.push_back(StringKeyPair(id, iconshadername));
	}
}

} //~namespace CTF


//
//UpdateTechs - called when console command "UpdateTechs" is recieved.
//This is called when a tech event happens (ie pickup, run out, drop etc)
//
extern "C"{
void CTF_UpdateTechs(void)
{
	const char* tmp = CG_ConfigString(CS_TECHINFO);
	char infostring[256];
	strcpy(infostring, tmp);

	//clear all techs
	if(!strcmpi(infostring, "clear"))
	{
		CTF::ctf_techs.clear();
	}

	//we picked up one or more techs
	else
	{
		if(atoi(cgi.Argv(1)))
			CTF::ctf_techs.clear();

		while(1)
		{
			const char* techStr = Info_ValueForKey(infostring, "tech");
			const char* timeStr = Info_ValueForKey(infostring, "time");

			if(!strlen(techStr) || !strlen(timeStr))
				break;


			TechListIterator it = CTF::GetTech(atoi(techStr));

			//if we dont have it already, add it
			if(it == CTF::ctf_techs.end())
			{
				CTF::ctf_techs.push_back(std::make_pair(
					atoi(techStr), cg.snap->serverTime + atoi(timeStr) * 1000
				));
			}

			//we already have a tech of this type, 
			//so simply update the time of the one we already have
			else
			{
				it->second = cg.snap->serverTime + atoi(timeStr) * 1000;
			}

			Info_RemoveKey(infostring, "tech");
			Info_RemoveKey(infostring, "time");
		}
	}
}
}// ~extern "C"