//////////////////////////////////////////////////////////////////////
// A forward iterator for iterating over all the players in a team
// pass the team to the constructor, or TEAM_NONE for all clients.
//


#include "Player.h"

#pragma warning(push, 1)
#include <iterator>
#pragma warning(pop)

class TeamIterator : public std::iterator<std::forward_iterator_tag, Player*>
{
public:
	TeamIterator() : m_nPos(-1)
	{}

	TeamIterator(const TeamIterator& rhs) : m_Team(rhs.m_Team), m_nPos(rhs.m_nPos)
	{}

	explicit TeamIterator(teamtype_t team) : m_Team(team), m_nPos(-1)
	{
		// Our current pos is -1, so this will start looking from 0
			FindNextPlayer();
	}

	Player& operator*()
	{
		// Make sure pos is valid
			assert(m_nPos >= 0 && m_nPos < maxclients->value);

		// Then give them what they want
			return *static_cast<Player*>(g_entities[m_nPos].entity);
	}

	Player* operator->()
	{
		return &operator*();
	}

	TeamIterator& operator++()
	{
		// Make sure pos is valid
			assert(m_nPos >= 0 && m_nPos < maxclients->integer);

		// Move on to the next player
			FindNextPlayer();

		return *this;
	}

	bool operator == (const TeamIterator& rhs)
	{
		// We only need to check the pos - team can be anything cos we should only check
		// iters to the same thing (and it makes it easier for 'end' to work)
		return m_nPos == rhs.m_nPos;
	}

	bool operator != (const TeamIterator& rhs)
	{
		return !(*this == rhs);
	}


private:

	// Finds the next player, or sets the state to 'end' (i.e. m_nPos == -1) if none found
	void FindNextPlayer()
	{
		// Initial advance so we do start looking after the current player
			++m_nPos;

		// find the next player that matches the team we want
		for(;m_nPos < maxclients->integer; ++m_nPos)
		{
			if(!g_entities[m_nPos].inuse)
				continue;
			
			// if the team doens't matter, we can stop looking now
				if(m_Team == TEAM_NONE)
					break;

			// Check that it really is a player
				assert(dynamic_cast<Player*>(g_entities[m_nPos].entity));

			// If this player is on the team we want, we're done
				if(static_cast<Player*>(g_entities[m_nPos].entity)->GetTeam() == m_Team)
					break;
		}

		// If we didn't find anything, set our state to 'end'
			if(m_nPos > maxclients->integer-1)
				m_nPos = -1;
	}


/////////////////////////////////////////////////////////////////////////////
// Data
	teamtype_t m_Team; // The team we're looking for
	int m_nPos; // the current position in the edict array. -1 = end
};