#if !defined(CTF_TYPES_H_INCLUDED)
#define CTF_TYPES_H_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(push, 1)
#include <vector>
#pragma warning(pop)

class CTF_PlayerStart;
class CTF_TargetLocation;

typedef std::vector<CTF_PlayerStart*>		SpawnPoints;
typedef std::vector<CTF_TargetLocation*>	MapLocations;


#endif // ~CTF_TYPES_H_INCLUDED
