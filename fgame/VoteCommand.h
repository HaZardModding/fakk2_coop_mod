#if !defined(AFX_VOTECOMMAND_H__AFB00E8A_EED1_4552_990C_861D954C71E4__INCLUDED_)
#define AFX_VOTECOMMAND_H__AFB00E8A_EED1_4552_990C_861D954C71E4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable : 4514)
#pragma warning(push, 1)
#include <string>
#pragma warning(pop)

namespace VoteCommand {

////////////////////////////////////////////////////////////////////
// Default vote command class.
class Default
{
public:

	Default(const std::string& cmd, const std::string& args);
	virtual ~Default();

	// True if it possible for this vote command to work
		virtual bool CanExecute() const;

	// Returns a user-friendly version of the vote string
	// e.g. a kick by player id will show the name of the player being kicked here
		virtual std::string GetVoteString() const;

	// Returns a string containing the reason that this command can't be executedon
		virtual std::string GetReason() const;

	// Runs whatever the stored command is
		virtual void Execute() const;

	// Creation function used by the vote factory
		static Default* Create(const std::string& cmd, const std::string& args);

protected:

	void RunCommand(const std::string& cmd) const;

	// the command that's being voted
		std::string m_cmd; 

	// args for the command
		std::string m_args; 
};


} //~namespace VoteCommand

#endif // !defined(AFX_VOTECOMMAND_H__AFB00E8A_EED1_4552_990C_861D954C71E4__INCLUDED_)
