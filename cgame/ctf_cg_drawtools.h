//
//
//Common drawing releated routines & data go here.
//
//- Yorvik
//

#ifndef CTF_CG_DRAWTOOLS_H_DEFINED
#define CTF_CG_DRAWTOOLS_H_DEFINED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef __cplusplus
#include "str.h"

extern float SCREEN_W;
extern float SCREEN_H;

//
//Constants
//
extern const int	TEAM_NONE;
extern const int	TEAM_RED;
extern const int	TEAM_BLUE;
extern const float	TEAM_COLOR[2][4];
extern const float	FLAG_ICON_WIDTH;
extern const float	FLAG_ICON_HEIGHT;
extern const float	HUD_BAR_HEIGHT;

extern const float	HEAD_ICON_SIZE;

extern const float	VIRTUAL_SCREEN[][2];
extern float		SCREEN_W;
extern float		SCREEN_H;

extern const float	FONT_WIDTH;
extern const float	FONT_HEIGHT;
extern const float	FONT_IMG_WIDTH;
extern const float	FONT_IMG_HEIGHT;
extern const float	FONT_TEX_WIDTH;
extern const float	FONT_TEX_HEIGHT;

extern const float	NUM_FONT_WIDTH;
extern const float	NUM_FONT_HEIGHT;

extern const float	HUD_ICON_WIDTH;
extern const float	HUD_ICON_HEIGHT;

extern const float	LAGOMETER_WIDTH;
extern const float	LAGOMETER_HEIGHT;

extern const float  SB_HCOLUMNS[];
extern const float  SB_COLUMNS[];

namespace CTF{
	//
	//Alignment enumeration
	//
	enum ctf_fontalign_t{
		ALIGN_NONE,
		ALIGN_CENTER,
		ALIGN_LEFT,
		ALIGN_RIGHT,
		ALIGN_TOP,
		ALIGN_BOTTOM
	};

	//
	//data
	//
	extern float	screen_xmod;
	extern float	screen_ymod;
	extern vec4_t	currentColor;
	extern int		followEntnum;

	//
	//std::sort comaprison functions
	//all take 2 entnum's
	//
	bool FragCompare(int left, int right);	//compare frags
	bool CapsComapre(int left, int right);	//compare captures
	bool NameCompare(int left, int right);	//compare names

	//
	//drawing to virtual screen
	//
	void HeadSprite(centity_t *player, qhandle_t shader, float offset);
	int  Align(ctf_fontalign_t align, int size, int upper);
	void DrawBox(float x, float y, float w, float h);
	void DrawNumber(int num, float x, float y, float scale = 1.0f);
	int  DrawString(const char* s, float x, float y, float scale = 1.0f, bool shadow = false, 
				float alpha = 1.0f);
	int  DrawAlignedString(const char* s, float x, float y, float scale = 1.0f, 
				bool shadow = false, float alpha = 1.0f,
				ctf_fontalign_t horiz = ALIGN_NONE, ctf_fontalign_t vert = ALIGN_NONE);
	void DrawPic(float x, float y, float w, float h, float texx, float texy,
				float texw, float texh, qhandle_t handle);

	//
	//Color management
	//
	void	SetColor(const vec4_t color);
	void	SetColor(float r, float g, float b, float a);
	str		TrimColor(const char* s);
	str		CapString(const char* s, int max);
	float*	PercentColor(float max, float val, vec4_t full = colorWhite, vec4_t half = colorYellow, vec4_t qtr = colorRed);

	//
	//Misc
	//
	int MaxNameWidth(bool ourTeamOnly = true);
	int MaxTechs(bool ourTeamOnly = true);

}//~namespace CTF

extern "C"{
	extern char ctf_voteString[1024];
};

#endif //ifdef C++


//
//C bits
//
extern char ctf_voteString[1024];


#endif //CTF_CG_DRAWTOOLS_H_DEFINED
