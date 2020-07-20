/////////////////////////////////////////////////////////////////////////////
//
// An STL style forward iterator for iterating over entity 'classes' (i.e. 
// those that you use G_FindClass for). The default constructor creates
// an 'end' iterator pass the string name of the class you want to iterator
// over to the ctor to create a 'begin' iterator.
//
//
/////////////////////////////////////////////////////////////////////////////

#pragma warning(push, 1)
#include <string>
#include <iterator>
#pragma warning(pop)


template<class TClassType>
class EntityClassIterator : public std::iterator<std::forward_iterator_tag, TClassType*>
{
public:
	// Default ctor - end iterator (no need to anything with the string)
	EntityClassIterator() : m_pCurrent(NULL)
	{}

	// Copy ctor
	EntityClassIterator(const EntityClassIterator& other) 
							:	m_pCurrent(other.m_pCurrent), 
								m_strClassname(other.m_strClassname)
	{}

	// construct with a string to make a 'begin' iterator
	explicit EntityClassIterator(const std::string& strClassname) : m_strClassname(strClassname)
	{
		// Setup current with the first match for this class name
		m_pCurrent = static_cast<TClassType*>(G_FindClass(NULL, m_strClassname.c_str()));
	}

	// pre-increment. Moves on to the next class
	EntityClassIterator& operator++()
	{
		// Warn if current is null i.e. we've gone passed the end - this could cause infinite loop
		assert(m_pCurrent);

		// Then find the next class
		m_pCurrent = static_cast<TClassType*>(G_FindClass(m_pCurrent , m_strClassname.c_str()));

		return *this;
	}

	// post-increment. Moves on to the next class
	EntityClassIterator& operator++(int)
	{
		EntityClassIterator temp(*this);

		++*this;
		
		return temp;
	}

	// Get the current class type
	TClassType* operator*()
	{
		return m_pCurrent;
	}

	bool operator == (const EntityClassIterator& rhs)
	{
		// We are the same if we point the same class
		// we can't check the string as well cos this could be an end iterator
		return m_pCurrent == rhs.m_pCurrent;
	}

	bool operator != (const EntityClassIterator& rhs)
	{
		// Use the == to check, so we don't need to change both of these if == changes
		return !operator==(rhs);
	}


private:
/////////////////////////////////////////////////////////////////////////////
// Data
	std::string m_strClassname; // the name of the class we're looking for
	TClassType* m_pCurrent; // the current class
};
