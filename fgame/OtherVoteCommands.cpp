#include "OtherVoteCommands.h"
#include "Player.h"
#include "ctf_global.h"


namespace VoteCommand {


/////////////////////////////////////////////////////////////////////////////
// Kick vote command
/////////////////////////////////////////////////////////////////////////////
Kick::Kick(const std::string& cmd, const std::string& args) 
				:	Default(cmd, args),
					m_clientNum(CTF::GetPlayerNum(args, true))
{
	// We're only for kick commands
	assert(CTF::ToUpper(cmd) == "KICK");
}

// True if it possible for this vote command to work
bool Kick::CanExecute() const
{
	// If this client is in the game, they can be kicked
	return m_clientNum >= 0 
		&& m_clientNum < maxclients->value 
		&& g_entities[m_clientNum].inuse;
}


// Returns a user-friendly version of the vote string
std::string Kick::GetVoteString() const
{
	if(!CanExecute())
		return "";

	// We always need to run off and get the name of the player - they may have changed it
	// since the vote started, we're kicking via player id so the name is only for other 
	// players to see
	if(Player* player = static_cast<Player*>(g_entities[m_clientNum].entity))
	{
		return std::string("Kick ") + std::string(player->edict->client->pers.netname);
	}
	else
	{
		assert(false); // NULL player!!!!!
		return "";
	}
	
}

// Runs whatever the stored command is
void Kick::Execute() const
{
	RunCommand(va("%s %d", m_cmd.c_str(), m_clientNum));
}


Default* Kick::Create(const std::string& cmd, const std::string& args)
{
	return new Kick(cmd, args);
}

// Returns a string containing the reason that this command can't be executedon
std::string Kick::GetReason() const
{
	return m_args + " is not on the server";
}

/////////////////////////////////////////////////////////////////////////////
// Map vote command
/////////////////////////////////////////////////////////////////////////////

Map::Map(const std::string& cmd, const std::string& args) : Default(cmd, args)
{
	assert(CTF::ToUpper(cmd) == "MAP");
}

// Runs whatever the stored command is
void Map::Execute() const
{
	// Make sure the map cycle does not get busted before we execute the command
		level.preserveCurrentMapCycle = true;

	Default::Execute();
}

// Creation function used by the vote factory
Default* Map::Create(const std::string& cmd, const std::string& args)
{
	return new Map(cmd, args);
}

/////////////////////////////////////////////////////////////////////////////
// Next map
/////////////////////////////////////////////////////////////////////////////
NextMap::NextMap(const std::string& cmd, const std::string& args) 
			: Default(cmd, "") // Next map has no args
{
	assert(CTF::ToUpper(cmd) == "NEXTMAP");
}

// Runs whatever the stored command is
void NextMap::Execute() const
{
	// Put a vstr before nextmap so the contents of nextmap cvar are executed
	RunCommand(va("vstr %s %s", m_cmd, m_args));
}


// Creation function used by the vote factory
Default* NextMap::Create(const std::string& cmd, const std::string& args)
{
	return new NextMap(cmd, args);
}

/////////////////////////////////////////////////////////////////////////////
// Teams
/////////////////////////////////////////////////////////////////////////////
Teams::Teams(const std::string& cmd, const std::string& args) 
			: Default("balance_teams", "") // Teams has no args
{
	assert(CTF::ToUpper(cmd) == "TEAMS" || CTF::ToUpper(cmd) == "BALANCE_TEAMS");
}

// Creation function used by the vote factory
Default* Teams::Create(const std::string& cmd, const std::string& args)
{
	return new Teams(cmd, args);
}

// True if it possible for this vote command to work
bool Teams::CanExecute() const
{
	// If teams are uneven, they can be balanced
	return CTF::TeamsAreUneven();
}

// Returns a user-friendly version of the vote string
std::string Teams::GetVoteString() const
{
	return "Balance teams";
}

// Returns a string containing the reason that this command can't be executedon
std::string Teams::GetReason() const
{
	return "The teams are fine";
}

} //~namespace VoteCommand