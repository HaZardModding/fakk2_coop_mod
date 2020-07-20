#include "g_local.h"
#include "Vote.h"

#pragma warning(push, 1)
#include <algorithm>
#include "TeamIterator.h"
#pragma warning(pop)

#include "VoteCommandFactory.h"
#include "VoteCommand.h"
#include "ctf_global.h"
#include "script.h"

namespace CTF {

qboolean CallVote(gentity_t* pEnt)
{
	//make sure voting is allowed on this server
		if(!g_allowVote->integer)
		{
			CTF::HudPrint(pEnt->entity, "Voting not allowed");
			return qtrue;
		}

	//dont let spectators start a vote
		if(!pEnt->entity || !pEnt->entity->isSubclassOf(Player) || !CTF::OnTeam(static_cast<Player*>(pEnt->entity)))
		{
			CTF::HudPrint(pEnt->entity, "Spectators cannot vote");
			return qtrue;
		}

	// Just pass this on to the vote system
		VoteSystem::Instance().CallVote(pEnt, gi.args());

	return qtrue;

}

qboolean Vote(gentity_t* pEnt)
{
	//make sure voting is allowed on this server
		if(!g_allowVote->integer)
		{
			CTF::HudPrint(pEnt->entity, "Voting not allowed");
			return qtrue;
		}

	//dont let spectators vote
		if(!pEnt->entity || !pEnt->entity->isSubclassOf(Player) || !CTF::OnTeam(static_cast<Player*>(pEnt->entity)))
		{
			CTF::HudPrint(pEnt->entity, "Spectators cannot vote");
			return qtrue;
		}

	// Make sure we've got a valid vote
		if(gi.argc() < 1)
		{
			CTF::HudPrint(pEnt->entity, "Only 'Yes' and 'No' are valid votes");
			return qtrue;
		}

	// Find out if they voted yes
		bool bYes = toupper(gi.argv(1)[0]) == 'Y';
	
	// Then pass this on to the vote system
		VoteSystem::Instance().Vote(pEnt, bYes);
		
	return qtrue;
}

const int VoteSystem::m_nVoteTimeout = 20; //the time (seconds) allowed for everyone to vote

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VoteSystem::VoteSystem() : m_pVoteCommand(NULL)
{
	Reset();

	LoadVoteCommands("global/callvote.scr");
}

VoteSystem::~VoteSystem()
{
	
}

VoteSystem& VoteSystem::Instance() // Singleton implementation
{
	static VoteSystem vote;
	return vote;
}



//////////////////////////////////////////////////////////////////////


//calls a vote for the player represented by pEnt
//strVoteCommand is the command line being voted for. e.g. "map f2ctf1"
void VoteSystem::CallVote(const gentity_t* pEnt, const std::string& strVoteCommand)
{
	//if we're already in a vote, we cant have another one
	if(m_pVoteCommand)
	{
		CTF::HudPrint(pEnt->entity,"Vote in progress\n");
		return;
	}

	//make sure the vote is allowed
	if(!IsVallidVote(strVoteCommand))
	{
		CTF::HudPrint(pEnt->entity,"You can't vote on that command\n");
		
		//tell them what commands they can vote on
		ShowVoteCommands(pEnt);
		return;
	}

	m_pPlayer = pEnt; // The person that voted

	m_pVoteCommand = VoteCommandFactory::Create(strVoteCommand);

	//store the time of the vote start
	m_fVoteStartTime = level.time;

	
	if(m_pVoteCommand->CanExecute())
	{
		Vote(pEnt, true); //the person that called the vote automatically votes yes
	}
	else
	{
		CTF::HudPrint(pEnt->entity, va("%s\n", m_pVoteCommand->GetReason().c_str()));
		Reset();
	}

}

//makes the player pEnt vote yes or no
void VoteSystem::Vote(const gentity_t* pEnt, bool bYes)
{
	if(!m_pVoteCommand)
	{
		CTF::HudPrint(pEnt->entity, "No vote in progress\n");
		return;
	}

	//make sure this player isn't already in the set
	if(m_votedPlayers.find(pEnt) == m_votedPlayers.end())
	{
		//add the player to the set
		m_votedPlayers.insert(pEnt);

		//update the vote counts
		if(bYes)
			++m_nYesCount;
		else
			++m_nNoCount;

		CTF::HudPrint(pEnt->entity, "Vote Cast.");
		CheckVote();
	}
	else
	{
		CTF::HudPrint(pEnt->entity, "You've already voted\n");
	}
}

//sets all the voting things to their pre-vote state
void VoteSystem::Reset()
{
	m_nYesCount = 0;
	m_nNoCount = 0;

	delete m_pVoteCommand;
	m_pVoteCommand = NULL;

	m_lastPingTime = 0.0f;

	// Make sure we empty the set of players that have voted - otherwise only one 
	// vote per game
	m_votedPlayers.clear();

	//tell client about vote status
	for(int i=0; i<MAX_CLIENTS; ++i)
		gi.SendServerCommand(i, "votestring");
}

void VoteSystem::CheckVote()
{
	//if we're not in a vote, we dont need to check anything
	if(!m_pVoteCommand)
		return;

	//have we had enough yes votes to pass this vote?
	if(HasVotePassed())
	{
		//yep, so tell eveyone
		CTF::HudPrintToAll(NULL, va(S_COLOR_GREEN "Vote passed" S_COLOR_WHITE " - Yes(%d) No(%d)\n",
			m_nYesCount, m_nNoCount));

		// Then call the command
		ExecuteVoteCommand();		
	}
	else //not enough 'yes' votes
	{
		//if time is up.....................................or the vote can't pass
		if(level.time - m_fVoteStartTime > m_nVoteTimeout || !CanVotePass())
		{
			//...fail the vote
			CTF::HudPrintToAll(NULL, va(S_COLOR_RED "Vote Failed" S_COLOR_WHITE " : Yes(%d) No(%d)\n", 
				m_nYesCount, m_nNoCount));
			
		}
		else
		{
			//time not up, so return now - if we don't we'll hit the call to Reset, and we don't want to do that
			return;
		}
		
	}

	//Get ready for next vote
	Reset();
}

// Returns the number of players in the game (not included spectators)
int VoteSystem::GetPlayerCount() const
{
	TeamIterator red(TEAM_RED);
	TeamIterator blue(TEAM_BLUE);
	TeamIterator end;
	
	return std::distance(red, end) + std::distance(blue, end);
}

//returns true if strVoteCommand is allowed to be vote on
bool VoteSystem::IsVallidVote(const std::string& strVoteCommand)
{
	// Get the name of the command (e.g. "map")
	std::string strCmd(strVoteCommand.begin(), std::find_if(strVoteCommand.begin(), strVoteCommand.end(), isspace));

	strCmd = CTF::ToUpper(strCmd);

	// Check to see if this vote command in in the set
	if( m_voteCommands.find(strCmd) != m_voteCommands.end() )
		return true;
	else
		return false;
}

//sets the commands that can be voted on. The commands should be separated by a semicolon
void VoteSystem::SetVoteCommands(const std::string& strCommands)
{
	std::string str = strCommands;

	//make sure we don't keep any old commands in the list
	m_voteCommands.clear();

	//convert the string to uppercase
	std::transform(str.begin(), str.end(), str.begin(), toupper);

	//remove all spaces. commands must be one word, and we don't want gaps before or after the semicolon
	while(str.find(' ') != std::string::npos)
		str.replace(str.find(' '), 1, "");

	//find the first semicolon in the string
	std::string::size_type pos = str.find(";");

	while(pos != std::string::npos)
	{
		//add the text before the ; to the set
		m_voteCommands.insert(str.substr(0, pos));

		//then set the string to the text after the semicolon
		str = str.substr(pos+1, std::string::npos);

		//and find the position of the next semicolon
		pos = str.find(";");
	}

	//add the last item to the set
	if(str.length() > 0)
		m_voteCommands.insert(str);
}

// Returns true if enough 'yes' votes have been passed to win the vote
bool VoteSystem::HasVotePassed() const
{
	//have we had enough yes votes to pass this vote?
	return (m_nYesCount >= GetYesTarget());
}

// Returns true if it's possible for the vote to pass (i.e. not more than 50% no votes)
bool VoteSystem::CanVotePass() const
{
	return m_nNoCount < GetYesTarget();
}

// Returns the number of 'yes' votes needed for this vote to pass
int VoteSystem::GetYesTarget() const
{
	int target = GetPlayerCount() / 2;

	if(target < 1)
		target = 1;
	else
		++target;

	return target;
}

//displays the commands allowed for voting to pEnt
void VoteSystem::ShowVoteCommands(const gentity_t* pEnt) const
{
	gi.SendServerCommand( pEnt-g_entities, "print \"Commands allowed :\n\"");
	
	std::set<std::string>::const_iterator it;

	for(it = m_voteCommands.begin(); it != m_voteCommands.end(); ++it)
	{
		gi.SendServerCommand( pEnt-g_entities, "print \"   %s\n\"", it->c_str());
	}
}

void VoteSystem::Ping()
{
	if(!m_pVoteCommand || level.time - m_lastPingTime < 1.0f)
		return;

	m_lastPingTime = level.time;
	bool voted = false;

	//tell client about vote status
	for(int i=0; i<MAX_CLIENTS; ++i)
	{
		voted = false;

		if(!g_entities[i].entity || !g_entities[i].entity->isSubclassOf(Player))
			continue;

		if(m_votedPlayers.find(g_entities[i].entity->edict) != m_votedPlayers.end())
			voted = true;

		//update the vote string (ie the big text that is drawn to the screen)
		gi.SendServerCommand(i, "votestring %s",
			va("%sVOTE (%d): %s   - Yes(%d) No(%d)", 
				(voted ? "^7" : "^1"),
				m_nVoteTimeout-static_cast<int>(level.time-m_fVoteStartTime),
				m_pVoteCommand->GetVoteString().c_str(),
				m_nYesCount, m_nNoCount
			)
		);
	}

	CheckVote();
}

//
//Parse a script file and any valid token is assumed a valid voteable command and as such,
//is added to our vote commands list
//
void VoteSystem::LoadVoteCommands(const std::string& filename)
{
	Script file;
	file.LoadFile(filename.c_str());

	while(file.TokenAvailable(qtrue))
		m_voteCommands.insert(ToUpper(file.GetToken(qtrue)));
}

//
//Executes the command that has been voted for
//there is some special handling for certain commands...
//
void VoteSystem::ExecuteVoteCommand() const
{
	assert(m_pVoteCommand);
	if(!m_pVoteCommand)
		return;

	if(m_pVoteCommand->CanExecute())
		m_pVoteCommand->Execute();
}

}
 //~namespace CTF