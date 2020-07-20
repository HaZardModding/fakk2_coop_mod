#if !defined(AFX_VOTECOMMANDFACTORY_H__351212A5_E2D3_4235_881A_DE755C2093FC__INCLUDED_)
#define AFX_VOTECOMMANDFACTORY_H__351212A5_E2D3_4235_881A_DE755C2093FC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(push, 1)
#include <string>
#include <map>
#pragma warning(pop)

namespace VoteCommand {
	class Default;
}

/////////////////////////////////////////////////////////////////////////////
//
// Factory class for creating VoteCommand objects for a specific vote command string
//
class VoteCommandFactory  
{
	// No direction creation
	VoteCommandFactory();
	VoteCommandFactory(const VoteCommandFactory&);
public:
	
	// Call this to create a vote object
		static VoteCommand::Default* Create(const std::string& cmd_with_args);
		static VoteCommand::Default* Create(const std::string& cmd, const std::string& args);

private:
	VoteCommand::Default* CreateVoteCommand(const std::string& cmd, const std::string& args) const;
};

#endif // !defined(AFX_VOTECOMMANDFACTORY_H__351212A5_E2D3_4235_881A_DE755C2093FC__INCLUDED_)
