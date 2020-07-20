// VoteSystem - singleton class for handling voting
//
// Voting is done from the console using the commands :
//		CallVote <command> [command args]	= start a vote, e.g CallVote map f2-ctf1. Only one vote at a time
//		vote yes							= vote for for the current vote
//		vote no								=  vote against the current vote
//
//	For a vote to pass >50% must vote yes.
//
//
//
//
//
// Stuff to todo:
//	* Reset the vote when the map changes
//
//  * Make sure there is only one command in the vote string (i.e. no ;)
//		cos ppl will try stuff like "callvote map ctf_void2;quit" to break 
//		the server
//	
//	* A cvar to enable/disable voting
//	
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VOTE_H__27A9AC84_2836_4449_8801_AC0A4E68CEBD__INCLUDED_)
#define AFX_VOTE_H__27A9AC84_2836_4449_8801_AC0A4E68CEBD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable : 4786)
#pragma warning(push, 1)
#include <set>
#include <string>
#include <utility>
#pragma warning(pop)

namespace VoteCommand{

	class Default;
}

namespace CTF {

class VoteSystem  
{
	VoteSystem(const VoteSystem&); //prevent copy
	operator=(const VoteSystem&); //prevent assignment

	VoteSystem(); // Need to access via 'Instance()'

public:
	~VoteSystem();

	// Singleton implementation
		static VoteSystem& Instance(); 

	// calls a vote for the player represented by pEnt
	// strVoteCommand is the command line being voted for. e.g. "map ctf_ewey"
		void CallVote(const gentity_t* pEnt, const std::string& strVoteCommand);

	// Makes the player 'pEnt' vote yes or no
		void Vote(const gentity_t* pEnt, bool bYes);

	// Checks if the vote has passed, and if it has it executes the voted command
		void CheckVote();

	// Sets the commands that can be voted on. The commands should be separated by a semicolon
		void SetVoteCommands(const std::string& strCommands);

	// Lets the vote system know that a frame has been run
		void Ping();
	
	// Display the commands allowed for voting to pEnt
		void ShowVoteCommands(const gentity_t* pEnt) const;

	// Sets all the voting stuff to their pre-vote state
		void Reset();

private:

	//Read voteable commands from a file
		void LoadVoteCommands(const std::string& filename);

	// Returns the number of players in the game (not included spectators)
		int GetPlayerCount() const;

	// Returns true if strVoteCommand is allowed to be vote on
		bool IsVallidVote(const std::string& strVoteCommand);

	// Returns true if we've had enough yes votes to pass the current vote
		bool HasVotePassed() const;

	// Returns true if it's possible for the vote to pass (i.e. not more than 50% no votes)
		bool CanVotePass() const;

	//Execute tha command that has been voted for
		void ExecuteVoteCommand() const;

	// Returns the number of 'yes' votes needed for this vote to pass
		int GetYesTarget() const;

//////////////////////////////////////////////////////////////////////
//data

	// Commands that are voteable
		std::set<std::string> m_voteCommands; 

	// Each of the players that has already voted
		std::set<const gentity_t*> m_votedPlayers;  

	// The player that called the vote
		const gentity_t* m_pPlayer; 

	// The current vote command, or NULL for none
		VoteCommand::Default* m_pVoteCommand; 
		
	int m_nYesCount;//the number of yes votes
	int m_nNoCount; //the number of no votes

	float m_fVoteStartTime; // The time the vote was called
	float m_lastPingTime;	//time the vote system was last pinged
	static const int m_nVoteTimeout; // The time (seconds) allowed for everyone to vote
};



qboolean CallVote(gentity_t* pEnt);
qboolean Vote(gentity_t* pEnt);

} //~namespace CTF

#endif // !defined(AFX_VOTE_H__27A9AC84_2836_4449_8801_AC0A4E68CEBD__INCLUDED_)
