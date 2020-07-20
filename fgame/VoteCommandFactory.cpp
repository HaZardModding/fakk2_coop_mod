#include "VoteCommandFactory.h"
#include "VoteCommand.h"
#include "OtherVoteCommands.h"
#include "ctf_global.h"

#pragma warning(push, 1)
#include <algorithm>
#pragma warning(pop)

namespace {
		
	// The private section of this class. It lives here so we don't get a load of crapy warnings in
	// every file that uses this
	typedef VoteCommand::Default* (*CreationFunc)(const std::string& cmd, const std::string& args);
	typedef std::map<std::string, CreationFunc> FactMap;

	FactMap m_factMap;
} //~namespace


/////////////////////////////////////////////////////////////////////////////
// VoteCommandFactory class
/////////////////////////////////////////////////////////////////////////////
VoteCommandFactory::VoteCommandFactory()
{
	// All vote command strings need to be uppercase inside the mae
	m_factMap.insert(FactMap::value_type("KICK", VoteCommand::Kick::Create));
	m_factMap.insert(FactMap::value_type("MAP", VoteCommand::Map::Create));
	m_factMap.insert(FactMap::value_type("KEXTMAP", VoteCommand::NextMap::Create));
	m_factMap.insert(FactMap::value_type("TEAMS", VoteCommand::Teams::Create));
	m_factMap.insert(FactMap::value_type("BALANCE_TEAMS", VoteCommand::Teams::Create));

}

VoteCommand::Default* VoteCommandFactory::Create(const std::string& command)
{
	// Convert the single string into two separate strings then call the overloaded version
	std::string::const_iterator endCmd = std::find_if(command.begin(), command.end(), isspace);
	std::string::const_iterator beginArgs = std::find_if(endCmd, command.end(), std::not1(std::ptr_fun(isspace)));
	std::string::const_iterator endArgs = std::find_if(beginArgs, command.end(), isspace);

	std::string cmd(command.begin(), endCmd);
	std::string args(beginArgs, endArgs);

	return Create(cmd, args);
}


VoteCommand::Default* VoteCommandFactory::Create(const std::string& cmd, const std::string& args)
{
	static VoteCommandFactory instance;
	return instance.CreateVoteCommand(cmd, args);
}

VoteCommand::Default* VoteCommandFactory::CreateVoteCommand(const std::string& cmd, const std::string& args) const
{
	// Use the correct creation function if this command is in the map, otherwise
	// just use a default vote command
	FactMap::const_iterator iter = m_factMap.find(CTF::ToUpper(cmd));

	if(iter != m_factMap.end())
	{
		return iter->second(cmd, args);
	}
	else
	{
		return VoteCommand::Default::Create(cmd, args);
	}
}