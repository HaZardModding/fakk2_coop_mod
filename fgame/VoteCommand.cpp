#include "VoteCommand.h"
#include "g_local.h"


namespace VoteCommand {

/////////////////////////////////////////////////////////////////////////////
// Default vote command
//
Default::Default(const std::string& cmd, const std::string& args)
				:	m_cmd(cmd),
					m_args(args)
{
}

Default::~Default()
{
	
}

// True if it possible for this vote command to work
bool Default::CanExecute() const
{
	return true; // always pass by default
}

// Returns a user-friendly version of the vote string
std::string Default::GetVoteString() const
{
	// Default just gives back the actaul string
	return m_cmd + " " + m_args;
}

// Runs whatever the stored command is
void Default::Execute() const
{
	RunCommand(m_cmd + " " + m_args);	
}


// Creation function used by the vote factory
Default* Default::Create(const std::string& cmd, const std::string& args)
{
	return new Default(cmd, args);
}

// Returns a string containing the reason that this command can't be executedon
std::string Default::GetReason() const
{
	return ""; // By default all commands can execute so we don't need to do anything here
}

void Default::RunCommand(const std::string& cmd) const
{
	gi.SendServerCommand(NULL, "stufftext \"%s\"", cmd.c_str());
}


} //~namespace VoteCommand


