///////////////////////////////////////////////////////////////////////////////
//
//
// CTF General Bits
//
// -Yorvik
//
///////////////////////////////////////////////////////////////////////////////

#include "ctf_global.h"
#include "ctf_manager.h"
#include "ctf_tech.h"
#include "ctf_flag.h"
#include "TeamIterator.h"

#pragma warning(push, 1)
#include <algorithm>
#include <functional>
#pragma warning(pop)

namespace CTF {

//
//Bonuses
//
const char*	HUD_MSG_COLOR			= S_COLOR_GREEN;
const char* HUD_TEAMMSG_COLOR		= S_COLOR_CYAN;

//
//SayTeam - called when console command say_team is recieved
//sends typed message (prefixed with senders name) to all other players
//
qboolean SayTeam(gentity_t* ent)
{
	if(!ent->entity || !ent->entity->isSubclassOf(Player))
	{
		assert(false);
		return qtrue;
	}

	str msg;
	Player* player = static_cast<Player*>(ent->entity);

	for(int i=1; i<gi.argc(); ++i)
	{
		msg += gi.argv(i);

		if(i < gi.argc()-1)
			msg += " ";
	}

	msg = ConvertSymbolString(msg.c_str(), player);

	TeamHudPrint(player, msg.c_str());
	LocalSoundToAll(NULL, "sound/misc/menu2.wav");
	return qtrue;
}

//
//Say - called when console command say is recieved
//sends the typed message (prefixed with senders name) to all players
//
qboolean Say(gentity_t* ent)
{
	if(!ent->entity || !ent->entity->isSubclassOf(Player))
	{
		assert(false);
		return qtrue;
	}

	str msg = CTF::GetName(ent);
	msg += ": ";
	msg += HUD_MSG_COLOR;

	int start = 1;
	if(stricmp(gi.argv(0), "say"))
		start = 0;

	for(int i=start; i<gi.argc(); ++i)
	{
		msg += gi.argv(i);

		if(i < gi.argc()-1)
			msg += " ";
	}

	msg = ConvertSymbolString(msg.c_str(), static_cast<Player*>(ent->entity));
	HudPrintToAll(NULL, msg.c_str());
	LocalSoundToAll(NULL, "sound/misc/menu2.wav");

	return qtrue;
}

//
//UpdateSingleScore
//
//args in format: entnum name frags team health ping time flag[1/0] lastkiller_entnum
//
void UpdateSingleScore(Player* from, gentity_t* to)
{
	gi.SendServerCommand(to->entity->edict-g_entities, "ctf_singlescore %s \n", 
		va("%i %s %i %i %i %i %i %i %i %i %i",
			from->entnum,
			CTF::GetName(from), 
			from->GetScore() - from->GetSuicides(),
			from->GetCaps(),
			static_cast<int>(from->GetTeam()),
			static_cast<int>(from->health),
			from->client->ping, 
			static_cast<int>(level.time - from->edict->spawntime) / 60,
			(from->GetFlag() == NULL) ? 0 : 1,
			from->LastKillerNum(),
			(from->CTF_IntermissionReady() == true ? 1 : 0)
			)
		);
}

//
//UpdateSingleLocation
//
//args in format: entnum name team health flag[1/0] num_techs <tech_id's> location_string
//
void UpdateSingleLocation(Player* from, gentity_t* to)
{
	const Player::TECH_LIST& techlist = from->GetTechList();

	int flag = 0;
	if(from->GetFlag())
		flag = 1;

	//distance from flag based coloring
	//TODO: move this to its own function someplace...
	float col[] = {1,1,1};

	CTF_Flag* blueflag = FindFlag(TEAM_BLUE);
	CTF_Flag* redflag  = FindFlag(TEAM_RED);
	if(redflag && blueflag)
	{
		float flagdist	= DistanceTo(redflag->GetSpawnPos(), blueflag->GetSpawnPos());
		float rdist		= DistanceTo(redflag->GetSpawnPos(), from->origin);
		float bdist		= DistanceTo(blueflag->GetSpawnPos(), from->origin);
				
		if(rdist < bdist)
			col[1] = col[2] = rdist / (flagdist * 0.5f);
		else
			col[0] = col[1] = bdist / (flagdist * 0.5f);
	}

	//entnum name team health flag
	std::string buf = va("%d %s %d %d %d %d %d %f %f %f %d ", 
			from->entnum,
			CTF::GetName(from),
			static_cast<int>(from->GetTeam()),
			static_cast<int>(from->health),
			flag,
			static_cast<int>(from->GetWaterPower()),
			GetCTFManager().GetPlayerLocation(from),
			col[0], col[1], col[2],
			techlist.size()
		);

	//techs			
		for(Player::TECH_LIST_CONSTITERATOR it = techlist.begin(); it != techlist.end(); ++it)
			buf += va("%d ", (*it)->GetID());

	//send update command with our args
		gi.SendServerCommand(to->entity->edict-g_entities, "ctf_singlelocation %s", buf.c_str());
}

//
//UpdateScores - send info on all players in our team to given client by 
//passing args to "singlescore" command
//
qboolean UpdateScores(gentity_t* ent)
{
	TeamIterator it(TEAM_NONE);
	TeamIterator end;

	for(; it != end; ++it)
		UpdateSingleScore(&*it, ent);

	return qtrue;
}

//
//Send info needed for team overlay from all players on our team to given client
//by passing args to "singlelocation" command
//
qboolean UpdateLocations(gentity_t* ent)
{
	//send our info to everyone on our team
	TeamIterator iter(static_cast<Player*>(ent->entity)->GetTeam());
	TeamIterator end;

	for(; iter != end; ++iter)
		UpdateSingleLocation(&*iter, ent);

	return qtrue;
}

//
//Typically called just after a vid restart to reinform the client about our current data
//(like what techs we have, rewards etc)
//
qboolean InitClientData(gentity_t* ent)
{
	if(!ent || !ent->entity || !ent->entity->isSubclassOf(Player))
		return qtrue;

	Player* player = static_cast<Player*>(ent->entity);


	//
	//TECHS
	//
	std::string techString = "";

	//build a configstring holding all techs
	const Player::TECH_LIST& techs = player->GetTechList();

	for(Player::TECH_LIST_CONSTITERATOR techIt = techs.begin(); techIt != techs.end(); ++techIt)
	{
		techString +=	va("tech\\%d\\time\\%d\\", 
							(*techIt)->GetID(), 
							static_cast<int>((*techIt)->GetTimeLeft())
						);
	}

	//set the configstring and tell client to update
	gi.setConfigstring(CS_TECHINFO, techString.c_str());
	gi.SendServerCommand(player->edict-g_entities, "ctf_updatetechs 1");

	//
	//REWARDS
	//
	const Player::REWARD_LIST rewards = player->GetRewardList();

	for(int n = 0; n < rewards.size(); ++n)
		gi.SendServerCommand(player->edict-g_entities, "addreward_nosound %d %d", n, rewards[n]);

	//
	//map locations
	//


	return qtrue;
}

//
//Send score info about ent to EVERYONE
//
void UpdateScoresToAll(gentity_t* ent)
{
	TeamIterator it(TEAM_NONE);
	TeamIterator end;

	Player* me = static_cast<Player*>(ent->entity);

	for(; it != end; ++it)
		UpdateSingleScore(me, it->edict);
}

//
//Send location info about ent to EVERYONE
//
void UpdateLocationsToAll(gentity_t* ent)
{
	//send our info to everyone on our team
	TeamIterator iter(TEAM_NONE);
	TeamIterator end;

	Player* me = static_cast<Player*>(ent->entity);

	for(; iter != end; ++iter)
		UpdateSingleLocation(me, iter->edict);
}

//
//ScaledKillBox
//
//Kills all entities that would touch the proposed new positioning
//of ent if it were scaled to 'scale'.
//
void ScaledKillBox(Entity* ent, float scale)
{
	int			touch[MAX_GENTITIES];
	gentity_t*	hit = NULL;
	Entity*		hitEnt = NULL;
	
	//get number of entities in the area, and store those ent's in touch array
	int num = gi.AreaEntities(ent->origin + ent->mins*scale, ent->origin + ent->maxs*scale, 
		touch, MAX_GENTITIES);

	//loop through ents, giving them all some damage
	for(int i=0; i<num; ++i)
	{
		hit = &g_entities[touch[i]];
		hitEnt = hit->entity;

		if(!hit->inuse || !hitEnt || hitEnt == world || hitEnt->edict->solid == SOLID_NOT)
			continue;

		//store origin for a hack later
			Vector oldOrigin = hitEnt->origin;

		//post damage event
			Event* dmgEv = new Event(EV_Damage);
			dmgEv->AddInteger(hitEnt->health+500);		//damage
			dmgEv->AddEntity(ent);						//inflictor
			dmgEv->AddEntity(ent);						//attacker
			dmgEv->AddVector(vec_zero);					//pos
			dmgEv->AddVector(vec_zero);					//dir
			dmgEv->AddVector(vec_zero);					//normal
			dmgEv->AddInteger(0);						//knockback
			dmgEv->AddInteger(DAMAGE_NO_PROTECTION);	//flags
			dmgEv->AddInteger(MOD_TELEFRAG);			//means of death

			hitEnt->ProcessEvent(dmgEv);

		//hack time
			hitEnt->velocity	= vec_zero;
			hitEnt->avelocity	= vec_zero;
			hitEnt->origin		= oldOrigin;
	}
}

//
//ScaledKillboxHurtsTeammate
//Returns whether a scaled killbox around player will hurt any of his teammates
//
bool ScaledKillBoxHurtsTeammate(Vector& origin, Player* player, float scale)
{
	int			touch[MAX_GENTITIES];
	gentity_t*	hit = NULL;
	Entity*		hitEnt = NULL;
	
	//get number of entities in the area, and store those ent's in touch array
	int num = gi.AreaEntities(origin + player->mins*scale, 
						origin + player->maxs*scale, touch, MAX_GENTITIES);

	//loop through ents, if any one of then is on same team as player, return true
	for(int i=0; i<num; ++i)
	{
		hit = &g_entities[touch[i]];
		hitEnt = hit->entity;

		if(hitEnt->isSubclassOf(Player))
		{
			//if the ent is on our team and is not us, return true
			if(static_cast<Player*>(hitEnt)->GetTeam() == player->GetTeam() && static_cast<Player*>(hitEnt) != player)
				return true;
		}
	}

	//the box contains no teammates
	return false;
}

//
//TeamName - returns string representation of the given team type
//
str TeamName(teamtype_t team, bool colored)
{
	switch(team)
	{
	case TEAM_RED:
		return colored ? S_COLOR_RED "RED" S_COLOR_WHITE : "RED";
		
	case TEAM_BLUE:
		return colored ? S_COLOR_BLUE "BLUE" S_COLOR_WHITE : "BLUE";

	case TEAM_NONE:
		return "none";

	default:
		assert(false);
		return "none";
	}
}

//
//OtherTeamName - returns string representation of the 'other' team
//
str OtherTeamName(teamtype_t team, bool colored)
{
	switch(team)
	{
	case TEAM_BLUE:
		return colored ? S_COLOR_RED "RED" S_COLOR_WHITE : "RED";

	case TEAM_RED:
		return colored ? S_COLOR_BLUE "BLUE" S_COLOR_WHITE : "BLUE";

	case TEAM_NONE:
		return "none";

	default:
		assert(false);
		return "none";
	}
}


// Returns the team in 'pszTeam'
teamtype_t GetTeam(const char* pszTeam)
{
	if(!pszTeam)
		return TEAM_NONE;

	// We only need to test the first char to see what team it is
	switch(pszTeam[0])
	{
	case 'r':
	case 'R':
		return TEAM_RED;

	case 'b':
	case 'B':
		return TEAM_BLUE;

	default:
		return TEAM_NONE;
	}
}

//
//OnTeam - returns true if we are on a proper team (ie red or blue)
//
bool OnTeam(Player* player)
{
	return player->GetTeam() == TEAM_RED || player->GetTeam() == TEAM_BLUE;
}

//
//CenterPrint
//
void CenterPrint(Entity* ent, const char* text)
{
	gi.SendServerCommand(ent->edict-g_entities, "stufftext \"hudcenterprint %s%s\"", S_COLOR_WHITE, text);
}

//
//CenterPrintToAll - prints the message to all players except 'exclude'
//
void CenterPrintToAll(Entity* exclude, const char* text)
{
	TeamIterator end;
	TeamIterator iter(TEAM_NONE);

	for(; iter != end; ++iter)
	{
		if(&*iter != exclude)
			CenterPrint(&*iter, text);
	}
}

//
//ScoreboardPrint
//
void ScoreboardPrint(Entity* ent, const char* text, bool centerPrint)
{
	gi.SendServerCommand(ent->edict-g_entities, "stufftext \"scoreboardprint %s%s\"", S_COLOR_WHITE, text);
	
	//send this text to the centerprint message system aswel
	if(centerPrint)
		CenterPrint(ent, text);
}

//
//OtherTeam - returns the 'opposite' team to the team passed in
//
teamtype_t OtherTeam(const teamtype_t team)
{
	if(team == TEAM_RED)
		return TEAM_BLUE;
	else if(team == TEAM_BLUE)
		return TEAM_RED;
	else
		return TEAM_NONE;
}

// Overload to take a CTF_Team
CTF_Team& OtherTeam(const CTF_Team& team)
{
	teamtype_t other = OtherTeam(team.GetTeamType());

	assert(other != TEAM_NONE);

	return *GetCTFManager().GetTeam(other);
}

//
//HudPrintToAll - prints a message to everyones hud message window
//
void HudPrintToAll(Entity* exclude, const char* text)
{
	TeamIterator end;
	TeamIterator iter(TEAM_NONE);

	for(; iter != end; ++iter)
	{
		if(&*iter != exclude)
			HudPrint(&*iter, text);
	}
}

//
//HudPrint - prints a message to the hud's message window
//
void HudPrint(Entity* ent, const char* text)
{
	//if we don't have a ent to talk to or any text to send - take a dive
	if(!ent || !text)
		return;

	gi.SendServerCommand(ent->edict-g_entities, "stufftext \"hudprint %s%s\"", S_COLOR_WHITE, text);
}

//
//DistanceLess - returns whether distance between 2 vectors is LESS THAN OR EQUAL TO
//given distance 
//
bool DistanceLess(const Vector& vec1, const Vector& vec2, float distance)
{
	float actualDist =	(vec1[0] - vec2[0]) * (vec1[0] - vec2[0])
						+
						(vec1[1] - vec2[1]) * (vec1[1] - vec2[1])
						+
						(vec1[2] - vec2[2]) * (vec1[2] - vec2[2]);

	return (distance*distance) > actualDist;
}

//
//DistanceTo - returns distance between 2 vectors
//
float DistanceTo(const Vector& vec1, const Vector& vec2)
{
	return	sqrt(
				(vec1[0] - vec2[0]) * (vec1[0] - vec2[0])
				+
				(vec1[1] - vec2[1]) * (vec1[1] - vec2[1])
				+
				(vec1[2] - vec2[2]) * (vec1[2] - vec2[2])
			);
}

//
//ClientPrint - draws some text to ent's screen
//
void ClientPrint(Entity* ent, const char* msg, const char* x, const char* y, float scale, float life)
{
	gi.SendServerCommand(ent->edict-g_entities, 
		"stufftext \"clientdrawtext %s %s %f %f ^7%s\"", 
		x, y, scale, life, msg
	);
}

//
//ClientPrintToAll - draws some text to all ent's screens except "exclude"'s
//
void ClientPrintToAll(Entity* exclude, const char* msg, const char* x, const char* y, float scale, float life)
{
	TeamIterator end;
	TeamIterator iter(TEAM_NONE);

	for(; iter != end; ++iter)
	{
		if(&*iter != exclude)
			ClientPrint(&*iter, msg, x, y, scale, life);
	}
}

//
//TeamPrint - prints a message to the hud's message window for all members of our team
//
//if sender is NULL, msg is send to everyone on 'backupteam' (which can be TEAM_NONE
//to send to all players)
//
void TeamHudPrint(Player* sender, const char* msg, teamtype_t backupteam)
{
	//only people on a proper team can send team messages
	if(sender)
	{
		if(sender->GetTeam() != TEAM_RED && sender->GetTeam() != TEAM_BLUE)
		{
			assert(false);
			return;
		}
	}


	TeamIterator end;
	TeamIterator iter(sender == NULL ? backupteam : sender->GetTeam());

	//paste the players name and map location to start of msg
	std::string message;
	if(sender != NULL)
	{
		str location = GetCTFManager().GetLocationString(
			GetCTFManager().GetPlayerLocation(sender)
		);

		message = S_COLOR_WHITE;
		message += CTF::GetName(sender);
		message += S_COLOR_WHITE;
		
		if(location.length())
		{
			message += " [";
			message += location;
			message += S_COLOR_WHITE;
			message += "]";
		}

		message += ": ";
		message += HUD_TEAMMSG_COLOR;
	}

	message += msg;
	
	//replace common symbols with correct data
	message = ConvertSymbolString(message.c_str(), sender).c_str();

	for(; iter != end; ++iter)
	{
		gi.SendServerCommand(iter->edict-g_entities, "stufftext \"team_hudprint %s\"", message.c_str());
	}
}

//returns the given strng + a white color flag
const char* WhiteEnd(const char* s)
{
	static char buf[1024];

	sprintf(buf, "%s%s", s, S_COLOR_WHITE);
	return buf;
}

//return the given string WITHOUT any color chars
const char* TrimColor(const char* s)
{
	static char buf[1024];
	str tmp = "";	

	int n = strlen(s);
	for(int i=0; i<n; ++i)
	{
		if(s[i] == Q_COLOR_ESCAPE && i < n-1)
		{
			++i;
			continue;
		}

		tmp += s[i];
	}

	strcpy(buf, tmp.c_str());
	return buf;
}

//
//find the blue or red flag
//
CTF_Flag* FindFlag(teamtype_t team)
{
	Entity* flag = G_FindClass(NULL, ""); //flags have a null class name
	while(flag)
	{
		if(flag && flag->isSubclassOf(CTF_Flag) && static_cast<CTF_Flag*>(flag)->GetTeam() == team)
			break;

		flag = G_FindClass(flag, "");
	}

	return static_cast<CTF_Flag*>(flag);
}

//
//see if 50% or more of the players in the game are 'ready'
//this is for intermission...
//
void CheckIntermissionProgress()
{
	if(!GetCTFManager().Intermission())
		return;

	int numClients = 0;
	int readyClients = 0;

	//go through all players and increment readyClients for each client that is 'ready'
	//also, keep a count of the total # of players
	for(int i=0; i<MAX_CLIENTS; ++i)
	{
		if	(
			g_entities[i].inuse && g_entities[i].entity && 
			g_entities[i].entity->isSubclassOf(Player) && 
			OnTeam(static_cast<Player*>(g_entities[i].entity))
			)
		{
			++numClients;

			if(static_cast<Player*>(g_entities[i].entity)->CTF_IntermissionReady())
				++readyClients;
		}
	}

	//if no clients, stay at intermission...
	if(numClients <= 0)
		return;

	//dont stay in intermission for more than 2 mins
	if(level.time - level.intermissiontime >= 120.0f)
		GetCTFManager().ExitIntermission();

	//quick hack to compensate for crappy integer division
	//reason: if only one client, intermission immediately ends
	if(numClients == 1)
		numClients = 2;

	//if the number of ready clients is >= half the total number of clients, 
	//go to next level
	if(readyClients >= numClients/2)
		GetCTFManager().ExitIntermission();
}

//
//ConvertSymbolString - replaces common symbols in given string with current data
//eg - $L is replaced with given players location...
//
str ConvertSymbolString(const char* text, Player* player)
{
	if(!text || !player)
	{
		assert(false);
		return "";
	}

	std::string buf = text;
	std::string replace = "";

	//LOCATION - $L
		const int nLocation = GetCTFManager().GetPlayerLocation(player);

		if(nLocation >= 0)
			replace = (GetCTFManager().GetLocationString(nLocation) + S_COLOR_GREEN).c_str();
		else
			replace = "Unknown Location";

		buf = Replace(buf, "$L", replace);
		buf = Replace(buf, "$l", replace);		

	//TEAM - $T
		replace = (TeamName(player->GetTeam(), true) + S_COLOR_GREEN).c_str();
		buf = Replace(buf, "$T", replace);
		buf = Replace(buf, "$t", replace);
		
	//OTHER TEAM - $O
		replace = (OtherTeamName(player->GetTeam(), true) + S_COLOR_GREEN).c_str();
		buf = Replace(buf, "$O", replace);
		buf = Replace(buf, "$o", replace);
	
	//OUR NAME - $N
		replace = va("%s%s", CTF::GetName(player), S_COLOR_GREEN);
		buf = Replace(buf, "$N", replace);
		buf = Replace(buf, "$n", replace);

	return str(buf.c_str());
}

//Remove all instances of given char from a string
void RemChar(std::string& s, char ch)
{
	std::string::iterator iter = std::remove(s.begin(), s.end(), ch);

	s.erase(iter, s.end());
}

//Remove all whitespace from a string
void RemWhite(std::string& s)
{
	std::string::iterator iter = std::remove_if(s.begin(), s.end(), isspace);

	s.erase(iter, s.end());
}

//
//replace bit of a string with another
//
std::string Replace(std::string source, const std::string& oldStr, const std::string& newStr)
{
	if(oldStr == newStr.substr(0, oldStr.size()))
		return source;

	int pos = 0;

	while((pos = source.find(oldStr)) >= 0)
	{
		source.replace(pos, oldStr.size(), newStr);
	}

	return source;
}


//
//Return uppercase version of given string
//
std::string ToUpper(std::string string)
{
	std::transform(string.begin(), string.end(), string.begin(), toupper);

	return string;
}

//
//LocalSound - plays a sound that only 'player' can hear
//
void LocalSound(Player* player, const char* sound)
{
	if(player)
		gi.SendServerCommand(player->edict-g_entities, "stufftext \"localsound %s\"", sound);
}

void LocalSoundToAll(Player* exception, const char* sound)
{
	TeamIterator it(TEAM_NONE);
	TeamIterator end;

	for(; it != end; ++it)
		if(&(*it) != exception)
			LocalSound(&*it, sound);
}

//
//gets player entnum for given name (case insensitive)
//
int GetPlayerNum(const std::string& name, bool ignoreColor)
{
	TeamIterator it(TEAM_NONE);
	TeamIterator end;

	for(; it != end; ++it)
	{
		std::string curname = CTF::GetName(it);

		if(ignoreColor)
			curname = TrimColor(curname.c_str());

		if(!strcmpi(curname.c_str(), name.c_str()))
			return it->entnum;
	}

	return -1;
}

//
//Load message of the day from file
//
void LoadMOTD()
{
	std::string motd = "";

	Script file;
	file.LoadFile("motd.txt");

	while(file.TokenAvailable(qtrue))
	{
		//get a line at a time
		motd += file.GetLine(qtrue);

		if(file.TokenAvailable(qtrue))
			motd += "\n";
	}

	if(motd.length())
		gi.cvar_set("sv_motd", motd.c_str());
}

//
//follow next teammate (look out of their eyes)
//
qboolean FollowNextTeammate(gentity_t* ent)
{
	if(!ent || !ent->entity || !ent->entity->isSubclassOf(Player))
		return qtrue;

	Player* player = static_cast<Player*>(ent->entity);	
	player->FollowNext(player->GetTeam());
	return qtrue;
}

//
//stop following
//
qboolean FollowOff(gentity_t* ent)
{
	if(!ent || !ent->entity || !ent->entity->isSubclassOf(Player))
		return qtrue;

	static_cast<Player*>(ent->entity)->StopFollowing();
	return qtrue;
}

// Returns a pointer to the team with the most players
// or NULL if they both have the same. minDiffrence allows
// you to say how big the diffrence must be before they have 'more players'
CTF_Team* GetTeamWithMostPlayers(const int minDifference = 1)
{
	CTF_Team& redTeam = *GetCTFManager().GetTeam(TEAM_RED);
	CTF_Team& blueTeam = *GetCTFManager().GetTeam(TEAM_BLUE);

	const int redSize  = redTeam.m_players.NumObjects();
	const int blueSize = blueTeam.m_players.NumObjects();

	const int difference = redSize - blueSize;

	if(abs(difference) < minDifference) // same
	{
		return NULL;
	}
	else if(difference >= minDifference) // more red
	{
		return &redTeam;
	}
	else // more blue
	{
		assert(difference <= -minDifference);
		return &blueTeam;
	}
}

CTF_Team& GetTeamMostNeedingANewPlayer()
{
	if(CTF_Team* team = GetTeamWithMostPlayers())
	{
		return *GetCTFManager().GetTeam(OtherTeam(team->GetTeamType()));
	}
	else 
	{
		// both teams have the same number of players, so we'll
		// pick the one with the lowest score

		CTF_Team& redTeam = *GetCTFManager().GetTeam(TEAM_RED);
		CTF_Team& blueTeam = *GetCTFManager().GetTeam(TEAM_BLUE);

		if(redTeam.GetCaptureCount() < blueTeam.GetCaptureCount())
		{
			return redTeam;
		}
		else
		{
			// Either teams have the same score, or blue has less
			return blueTeam;
		}
	}
}

// true if the teams are uneven
bool TeamsAreUneven()
{
	return GetTeamWithMostPlayers(2) != NULL;
}


// Forces the teams to be balanced
// The players that have been connected for the sortest time
// in the team with the most players will be moved to the other 
// team to make the number of players about equal
qboolean BalanceTeams(gentity_t* ent)
{
	// Get the team with the most players, by at least 2
	CTF_Team* team = GetTeamWithMostPlayers(2);

	if(!team)
	{
		//TODO: tell ent the teams are fine?
		return qtrue;
	}

	// If get here, we know 'mostPlayers' has more players
	CTF_Team& mostPlayers = *team;
	CTF_Team& leastPlayers = OtherTeam(mostPlayers);

	const int difference = mostPlayers.GetPlayerCount() - leastPlayers.GetPlayerCount();

	// We to take half the diffrence from mostPlayers and put them on leastPlayers
	// using a normal int devision will take care of difference being odd. So if 
	// mostPlayers have 3 more than leastPlayers, we'll only move 1 player
	const int playersToChange = difference/2;

	for(int i=0; i<playersToChange; ++i)
	{
		GetCTFManager().JoinTeam(mostPlayers.GetNewestPlayer(), leastPlayers.GetTeamType());
	}
	
	return qtrue;
}

//
//GETNAME FUNCTIONS
//

const char* GetName(const Entity* ent)
{
	assert(ent);

	return va("%s" S_COLOR_WHITE, ent->client->pers.netname);
}

const char* GetName(Entity* ent)
{
	assert(ent);

	return va("%s" S_COLOR_WHITE, ent->client->pers.netname);
}

const char* GetName(gentity_t* ent)
{
	assert(ent);

	return va("%s" S_COLOR_WHITE, ent->client->pers.netname);
}

const char* GetName(TeamIterator& entIT)
{
	assert(&*entIT);

	return va("%s" S_COLOR_WHITE, entIT->client->pers.netname);
}

} //~namespace CTF