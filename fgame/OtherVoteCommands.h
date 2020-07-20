#if !defined(AFX_OTHERVOTECOMMANDS_H__06077525_6106_4C23_9E99_1839AEFF6971__INCLUDED_)
#define AFX_OTHERVOTECOMMANDS_H__06077525_6106_4C23_9E99_1839AEFF6971__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VoteCommand.h"

namespace VoteCommand {

/////////////////////////////////////////////////////////////////////////////
// Vote class used when voting to kick a player
class Kick : public Default
{
public:
	Kick(const std::string& cmd, const std::string& args);

	// True if it possible for this vote command to work
		virtual bool CanExecute() const;

	// Returns a user-friendly version of the vote string
		virtual std::string GetVoteString() const;

	// Runs whatever the stored command is
		virtual void Execute() const;

	// Creation function used by the vote factory
		static Default* Create(const std::string& cmd, const std::string& args);

	// Returns a string containing the reason that this command can't be executedon
		virtual std::string GetReason() const;
private:

	// The id of the player that needs to be kicked
		int m_clientNum; 
};

/////////////////////////////////////////////////////////////////////////////
// Map
class Map : public Default
{
public:
	Map(const std::string& cmd, const std::string& args);

	// Runs whatever the stored command is
		virtual void Execute() const;

	// Creation function used by the vote factory
		static Default* Create(const std::string& cmd, const std::string& args);
};


/////////////////////////////////////////////////////////////////////////////
// Next map
class NextMap : public Default
{
public:
	NextMap(const std::string& cmd, const std::string& args);

	// Runs whatever the stored command is
		virtual void Execute() const;

	// Creation function used by the vote factory
		static Default* Create(const std::string& cmd, const std::string& args);
};


/////////////////////////////////////////////////////////////////////////////
// Balance teams
class Teams : public Default
{
	Teams(const std::string& cmd, const std::string& args);

	// Creation function used by the vote factory
		static Default* Create(const std::string& cmd, const std::string& args);

	// True if it possible for this vote command to work
		virtual bool CanExecute() const;

	// Returns a user-friendly version of the vote string
		virtual std::string GetVoteString() const;

	// Returns a string containing the reason that this command can't be executedon
		virtual std::string GetReason() const;
};

} //~namespace VoteCommand

#endif // !defined(AFX_OTHERVOTECOMMANDS_H__06077525_6106_4C23_9E99_1839AEFF6971__INCLUDED_)
