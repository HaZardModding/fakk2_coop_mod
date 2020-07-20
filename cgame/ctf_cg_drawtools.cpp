//
//
//Common drawing releated routines & data go here.
//
//- Yorvik
//

// Initial Includes
#include "cg_local.h"
#include "ctf_cg_drawtools.h"
#include "ctf_2d.h"
#include <string>

extern "C"{
	char ctf_voteString[1024];
}

//
//virtual screensize
//
//everything is draw to a virtual screen then scaled to the actual resolution
//
float SCREEN_W		= 800.0f;
float SCREEN_H		= 600.0f;

//standard font size info
const float	FONT_WIDTH			= 16;
const float	FONT_HEIGHT			= 20;
const float	FONT_IMG_WIDTH		= 256;
const float	FONT_IMG_HEIGHT		= 256;
const float	FONT_TEX_WIDTH		= FONT_WIDTH/FONT_IMG_WIDTH;
const float	FONT_TEX_HEIGHT		= FONT_WIDTH/FONT_IMG_HEIGHT;

//head icon size
const float HEAD_ICON_SIZE		= 48.0f;

//number font size info
const float NUM_FONT_WIDTH		= 32;
const float NUM_FONT_HEIGHT		= 48;

//teams
const int	TEAM_NONE			= 0;
const int	TEAM_RED			= 1;
const int	TEAM_BLUE			= 2;

//misc hud icon sizes
const float FLAG_ICON_WIDTH		= 32.0f;
const float FLAG_ICON_HEIGHT	= 20.0f;
const float	HUD_BAR_HEIGHT		= 40.0f;
const float HUD_ICON_WIDTH		= 32.0f;
const float HUD_ICON_HEIGHT		= 32.0f;
const float LAGOMETER_WIDTH		= 64.0f;
const float LAGOMETER_HEIGHT	= 64.0f;

//team colors
const vec4_t TEAM_COLOR[2]=
{
	{1.0f, 0.0f, 0.0f, 0.2f},	//RED
	{0.0f, 0.0f, 1.0f, 0.2f}	//BLUE
};

//x coords of scoreboard headings and data - they are not equal so that the data appears
//centered in it's 'cell' (not aligned to the left)
const float SB_HCOLUMNS[] = {
	10.0f, 60.0f, 100.0f, 140.0f, 200.0f
};
const float SB_COLUMNS[] = {
	25.0f, 65.0f, 105.0f, 145.0f, 200.0f
};




namespace CTF{

//these are the values by which all coords are eventually multiplied.
//this implements the 'virtual' screensize
//(note: they are actually set in CTF_InitClient; once the screen resolution is known)
	float screen_xmod = 1.0f;
	float screen_ymod = 1.0f;

//the entnum of the player we are following (looking out of eyes...)
	int followEntnum = -1;

//Current Color
	vec4_t currentColor = {1,1,1,1};

//when we are in intermission, are we ready?
	bool intermissionReady = false;

//
//SetColor - set current color, but also store it for future reference
//
void SetColor(float r, float g, float b, float a)
{
	currentColor[0] = r;
	currentColor[1] = g;
	currentColor[2] = b;
	currentColor[3] = a;

	cgi.R_SetColor(currentColor);
}
void SetColor(const vec4_t color)
{
	if(!color)
	{
		currentColor[0] = 1;
		currentColor[1] = 1;
		currentColor[2] = 1;
		currentColor[3] = 1;
	}
	else
	{
		currentColor[0] = color[0];
		currentColor[1] = color[1];
		currentColor[2] = color[2];
		currentColor[3] = color[3];
	}

	cgi.R_SetColor(color);
}

//
//PercentColor - returns a color depending on how 'val' compares to 'max'
//ie, if val is less than a quarter of max, 'qtr' is returned...
//
float* PercentColor(float max, float val, vec4_t full, vec4_t half, vec4_t qtr)
{
	if(max == 0)
		return full;

	if(val < max * 0.25f)
		return qtr;
	else if(val < max* 0.5f)
		return half;
	else
		return full;
}

//
//TrimColor - remove color characters from a string
//
str TrimColor(const char* s)
{
	str tmp;

	int n = strlen(s);
	for(int i=0; i<n; ++i)
	{
		if(s[i] == Q_COLOR_ESCAPE && i < n-1)
		{
			++i;
			continue;
		}

		tmp += s[i];
	}

	return tmp;
}

//
//CapString - trim given string to max size, maintaining color
//only visible chars are counted
//
str CapString(const char* s, int max)
{
	if(max <= 0)
		return "";

	str tmp;

	int n = strlen(s);
	int numChars = 0;

	for(int i=0; i<n; ++i)
	{
		//add color chars, but do not update the total length
		if(s[i] == Q_COLOR_ESCAPE)
		{
			tmp += s[i];
			
			if(i < n-1)
			{
				++i;
				tmp += s[i];
			}
		}

		//add the char and update the length of our composite string
		else
		{
			tmp += s[i];
			
			//if we hit the max, bail 
			if(++numChars > max)
				break;
		}
	}

	return tmp;
}

//
//NameCompare
//
//compararison func for std::sort
//looks up the corresponding entries in cgs.clientinfo for the names to compare
//
bool NameCompare(int left, int right)
{
	return strcmpi(
		TrimColor(cgs.clientinfo[left].name), 
		TrimColor(cgs.clientinfo[right].name)) < 0;
}

//
//CapsComapre
//
//compararison func for std::sort
//looks up the corresponding entries in cgs.clientinfo for the captures to compare.
//if the scores are equal, NameCompare is used.
//
bool CapsComapre(int left, int right)
{
	int score_l = cgs.clientinfo[left].caps;
	int score_r = cgs.clientinfo[right].caps;

	if(score_l == score_r)
		return NameCompare(left, right);
	

	return score_l > score_r;
}

//
//FragComapre
//
//compararison func for std::sort
//looks up the corresponding entries in cgs.clientinfo for the scores to compare.
//if the scores are equal, CapsComapre is used.
//
bool FragCompare(int left, int right)
{
	int score_l = cgs.clientinfo[left].frags;
	int score_r = cgs.clientinfo[right].frags;

	if(score_l == score_r)
		return CapsComapre(left, right);
	

	return score_l > score_r;
}

//
//DrawPic - draws a picture to the client screen. Compensates for virtual screensize
//
void DrawPic(float x, float y, float w, float h, float texx, float texy, float texw, float texh, qhandle_t handle)
{
	cgi.R_DrawStretchPic(
		x*screen_xmod,
		y*screen_ymod,
		w*screen_xmod,
		h*screen_ymod, 
		texx, texy, texw, texh, handle
	);
}

//
//DrawBox - draws a box in current color to client screen. Compensates for virtual screensize
//
void DrawBox(float x, float y, float w, float h)
{
	cgi.R_DrawBox(
		x*screen_xmod,
		y*screen_ymod,
		w*screen_xmod,
		h*screen_ymod
	);
}

//
//Align
//
int Align(ctf_fontalign_t align, float size, float upper)
{
	switch(align)
	{
	case ALIGN_LEFT:
	case ALIGN_TOP:
		return 0;
		break;

	case ALIGN_RIGHT:
	case ALIGN_BOTTOM:
		return upper-size;
		break;

	case ALIGN_CENTER:
	default:	
		return upper/2 - size/2;
		break;
	};

	//should never get here...
	return -1;
}

//
//return float array with color of given team
//
float* TeamColor(const int team, const float alpha)
{
	static float color[4] = {0,0,0,0};

	if(team == TEAM_RED)
	{
		color[0] = 1;
		color[1] = color[2] = 0;
		color[3] = alpha;
	}
	else if(team == TEAM_BLUE)
	{
		color[0] = color[1] = 0;
		color[2] = 1;
		color[3] = alpha;
	}
	else
	{
		color[0] = color[1] = color[2] = 1;
		color[3] = alpha;
	}

	return color;
}

//
//return color of team opposite to given team
//
float* OtherTeamColor(const int team, const float alpha)
{
	if(team == TEAM_RED)
		return TeamColor(TEAM_BLUE, alpha);
	else if(team == TEAM_BLUE)
		return TeamColor(TEAM_RED, alpha);
	else
		return TeamColor(TEAM_NONE, alpha);
}

//
//DrawString - draws a string to the client screen at given coords and in the
//CURRENT color (ie this func does not touch the color setting)
//
//returns last color changed to (ie the current color)
//
int DrawString(const char* s, float x, float y, float scale, bool shadow, float alpha)
{
	if(cgs.media.ui_charset == 0)
		return -1;

	if(shadow)
	{
		//store the current color
		float colBack[] = {currentColor[0], currentColor[1], currentColor[2], currentColor[3]};

		DrawString(va("%s%s%s", 
			S_COLOR_BLACK, TrimColor(s).c_str(), S_COLOR_WHITE), 
			x+2.0f*scale, y+2.0f*scale, scale, false, alpha);

		//restore old color
		currentColor[0] = colBack[0];
		currentColor[1] = colBack[1];
		currentColor[2] = colBack[2];
		currentColor[3] = colBack[3];
	}

	SetColor(currentColor[0], currentColor[1], currentColor[2], alpha);

	int lastColor = -1;				//set 'last' to white...
	int len = strlen(s);			//length of string
	float sx = 0;					//x position on source bitmap
	float sy = 0;					//y position on source bitmap

	float w = FONT_WIDTH*scale;		//width of outputted char
	float h = FONT_HEIGHT*scale;	//height of outputted char

	float dx = x;

	for(int i=0; i<len; ++i)
	{
		if(s[i] == '\n' || s[i] == '\t' || s[i] == '\r')
			continue;

		//if this char is a color modifier, change color to the color denoted by the next char
		if(s[i] == Q_COLOR_ESCAPE && i < (len-1))
		{
			lastColor = static_cast<int>(s[i+1]) - 48;

			if(lastColor <= 7 && lastColor >= 0)
				SetColor(g_color_table[lastColor][0], g_color_table[lastColor][1], g_color_table[lastColor][2], alpha);			
			
			++i;
			continue;
		}

		sx = ( static_cast<int>(s[i]) % 16) * FONT_TEX_WIDTH;
		sy = ( static_cast<int>(s[i]) / 16) * FONT_TEX_HEIGHT;

		DrawPic(dx, y, w, h, sx, sy, sx+FONT_TEX_WIDTH, sy+FONT_TEX_HEIGHT, cgs.media.ui_charset);
		dx += w;
	}

	return lastColor;
}

//
//DrawNumbers - same as drawstring, but takes an int and uses the number font
//
void DrawNumber(int num, float x, float y, float scale)
{
	if(cgs.media.ui_numset == 0)
		return;

	const float tex_width	= NUM_FONT_WIDTH / 256.0f;
	const float tex_height	= NUM_FONT_WIDTH / 64.0f;

	char szNum[16];
	_itoa(num, szNum, 10);
	
	int len = strlen(szNum);

	float sx = 0;					//x position on source bitmap
	float sy = 0;					//y position on source bitmap

	float w = NUM_FONT_WIDTH*scale;		//width of outputted char
	float h = NUM_FONT_HEIGHT*scale;	//height of outputted char

	float dx = x;

	for(int i=0; i<len; ++i)
	{
		if(szNum[i] == '-')
		{
			sx = tex_width*2;
			sy = tex_height;
		}
		else
		{
			sx = ( (static_cast<int>(szNum[i])-48) % 8) * tex_width;
			sy = ( (static_cast<int>(szNum[i])-48 > 7) ? 1 : 0) * tex_height;
		}

		DrawPic(dx, y, w, h, sx, sy, sx+tex_width, sy+tex_height, cgs.media.ui_numset);
		dx += w;
	}
}

//
//DrawAlignedString - Draws the given string in the desired place on the screen.
//
//Note - if horiz is anything but ALIGN_NONE, x is ignored (same for vert and y...)
//
int DrawAlignedString(const char* s, float x, float y, float scale, bool shadow, float alpha,
									ctf_fontalign_t horiz, ctf_fontalign_t vert)
{
	float w = FONT_WIDTH*scale * TrimColor(s).length();	//total width of outputted string
	float h = FONT_HEIGHT*scale;						//total height of outputted string

	//align horizontally
	if(horiz != ALIGN_NONE)
		x = Align(horiz, w, SCREEN_W);

	//align vertically
	if(vert != ALIGN_NONE)
		y = Align(vert, h, SCREEN_H);

	//draw the string
	return DrawString(s, x, y, scale, shadow, alpha);
}

//
//return width of longest name on our team
//
int MaxNameWidth(bool ourTeamOnly)
{
	int maxlen = 10;
	int curlen = 0;

	for(int i=0; i<MAX_CLIENTS; ++i)
	{
		if(ourTeamOnly && cgs.clientinfo[i].team != cg.snap->ps.stats[STAT_TEAM])
			continue;

		curlen = TrimColor(cgs.clientinfo[i].name).length();
		if(curlen > maxlen)
			maxlen = curlen;
	}

	return maxlen;
}

//
//return how many techs the client with the most techs has
//
int MaxTechs(bool ourTeamOnly)
{
	int max = 0;
	int cur = 0;
	int i, j;

	for(i=0; i<MAX_CLIENTS; ++i)
	{
		if(ourTeamOnly && cgs.clientinfo[i].team != cg.snap->ps.stats[STAT_TEAM])
			continue;
		
		cur = 0;
		for(j=0; j<MAX_TECHS; ++j)
		{
			if(cgs.clientinfo[i].techs[j] > 0)
				++cur;
		}

		if(cur > max)
			max = cur;
	}

	return max;
}

//
//float a sprite over a players head
//
void HeadSprite(centity_t *player, qhandle_t shader, float offset)
{
	if(shader <= 0)
		return;

	refEntity_t sprite;

	memset(&sprite, 0, sizeof(sprite));
	VectorCopy(player->lerpOrigin, sprite.origin);

	sprite.reType			= RT_SPRITE;
	sprite.hModel			= shader;
	sprite.origin[2]		+= offset;
	sprite.shaderRGBA[0]	= 255;
	sprite.shaderRGBA[1]	= 255;
	sprite.shaderRGBA[2]	= 255;
	sprite.shaderRGBA[3]	= 255;
	sprite.renderfx			= 0;
	sprite.scale			= 0.5f; //FIXME - mak this indpendant of shader size
	
	cgi.R_AddRefSpriteToScene(&sprite);
}

} //~namespace CTF

//
//set the intermissionReady bit of the clientinfo struct
//
extern "C" {

void CTF_IntermissionReady()
{
	CTF::intermissionReady = atoi(cgi.Argv(1)) != 0;
}

void CTF_DrawMOTD()
{
	CTF::DrawMOTD();
}

void CTF_FollowEnt(int entnum)
{
	CTF::followEntnum = entnum;
}

//
//colorise the given name string with appropriate team colors
//ie if the ent is on the red team, all ^8's are replaced with ^1's
//
const char* CTF_ColoriseName(int entnum, char* name)
{
	std::string s = name;
	std::string teamcol = "^7";
	std::string otherteamcol = "^7";

	if(cgs.clientinfo[entnum].team == TEAM_RED)
	{
		teamcol[1] = '1';
		otherteamcol[1] = '4';
	}
	else if(cgs.clientinfo[entnum].team == TEAM_BLUE)
	{
		teamcol[1] = '4';
		otherteamcol[1] = '1';
	}

	if(*s.rbegin() == '^')
		s.erase(s.end()-1);

	while(s.find("^8") != std::string::npos)
		s.replace(s.find("^8"), 2, teamcol);

	while(s.find("^9") != std::string::npos)
		s.replace(s.find("^9"), 2, otherteamcol);

	strcpy(name, s.c_str());
	return name;
}

} //~extern "C"