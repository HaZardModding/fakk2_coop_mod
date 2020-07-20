/////////////////////////////////////////////////////////////////////
//
// 2d hud bits
//
// -Yorvik
//
/////////////////////////////////////////////////////////////////////

#ifndef CTF_2D_DEFINED
#define CTF_2D_DEFINED

//use c linkage (no need to specify if already in c mode, ugh)
#ifdef __cplusplus
	extern "C"{
#endif

	extern cvar_t* ui_hud;
	extern cvar_t* cg_draw2D;
	extern cvar_t* cg_drawCrosshairNames;
	extern cvar_t* cg_drawTeamOverlay;

	//
	//functions that are called from C code go here
	//
	void CTF_InitClient(void);
	void CTF_InitLocations(void);

#ifdef __cplusplus
	}
#endif

//
//only use these bits in c++
//
#ifdef __cplusplus
	#include "str.h"

	namespace CTF{
		void DrawHud();
		void DrawFlags();
		void DrawTeamLine();
		void DrawMOTD();
	} //~namespace CTF
	

#endif //endif c++

#endif //CTF_2D_DEFINED