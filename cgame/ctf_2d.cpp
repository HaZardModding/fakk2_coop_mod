/////////////////////////////////////////////////////////////////////
//
// 2d hud bits
//
// -Yorvik
//
/////////////////////////////////////////////////////////////////////

#include "cg_local.h"
#include "ctf_2d.h"
#include "ctf_tech.h"
#include "ctf_msg.h"
#include "ctf_cg_drawtools.h"
#include "ctf_reward.h"
#include "ctf_itempickup.h"

#pragma warning(push, 1)
#include <string>
#include <vector>
#include <algorithm>
#pragma warning(pop)

namespace CTF{

	std::vector<str> ctf_locations;
	int ctf_maxLocationLength;

	extern bool intermissionReady;
	
	extern CTF_ScoreboardMsgSystem msg_sb;
	extern CTF_HudMsgSystem msg_normal;
	extern CTF_TeamMsgSystem msg_team;
	extern CTF_ClientText msg_free;

	vec3_t vec_zero = {0,0,0};

}//~namespace CTF

//prototypes
float CG_DrawLagometer();

extern "C"{
//
//Calculates modification values for virtual screen & moves weapon huds 
//into alignment with hud ammo readouts
//also does other misc initialisation
//
void CTF_InitClient(void)
{

	//init screen
	CTF::screen_xmod = cgs.glconfig.vidWidth / SCREEN_W;
	CTF::screen_ymod = cgs.glconfig.vidHeight / SCREEN_H;

	CTF_InitMsgSystem();

	//ask the server for some data about us
	cgi.Cmd_Execute(EXEC_NOW, "ctf_initclientdata");
	CTF_InitLocations();

	//
	//scale the weapon display huds to the right sizes
	//

	//dual handed weapon hud
	cgi.Cmd_Execute(EXEC_NOW, va("globalwidgetcommand dualweaphud rect %f %f %f %f", 
		(HUD_ICON_WIDTH*CTF::screen_xmod)*11.0f, cgs.glconfig.vidHeight-(HUD_BAR_HEIGHT*CTF::screen_ymod),
		(HUD_ICON_WIDTH*CTF::screen_xmod)*2.0f, (HUD_BAR_HEIGHT*CTF::screen_ymod))
	);
	cgi.Cmd_Execute(EXEC_NOW, va("globalwidgetcommand dual_model rect %f %f %f %f", 
		0.0f, 0.0f,
		(HUD_ICON_WIDTH*CTF::screen_xmod)*2.0f, (HUD_BAR_HEIGHT*CTF::screen_ymod))
	);

	//one handed weapon hud
	cgi.Cmd_Execute(EXEC_NOW, va("globalwidgetcommand weaphud rect %f %f %f %f", 
		(HUD_ICON_WIDTH*CTF::screen_xmod)*11.0f, cgs.glconfig.vidHeight-(HUD_BAR_HEIGHT*CTF::screen_ymod),
		(HUD_ICON_WIDTH*CTF::screen_xmod)*2.0f, (HUD_BAR_HEIGHT*CTF::screen_ymod))
	);
	cgi.Cmd_Execute(EXEC_NOW, va("globalwidgetcommand left_model rect %f %f %f %f", 
		0.0f, 0.0f,
		(HUD_ICON_WIDTH*CTF::screen_xmod), (HUD_BAR_HEIGHT*CTF::screen_ymod))
	);
	cgi.Cmd_Execute(EXEC_NOW, va("globalwidgetcommand right_model rect %f %f %f %f", 
		(HUD_ICON_WIDTH*CTF::screen_xmod), 0.0f,
		(HUD_ICON_WIDTH*CTF::screen_xmod), (HUD_BAR_HEIGHT*CTF::screen_ymod))
	);
}

//
//InitLocations
//
void CTF_InitLocations()
{
	CTF::ctf_locations.clear();

	str cs = CG_ConfigString(CS_MAPLOCATIONS);
	int num = atoi(Info_ValueForKey(cs.c_str(), "count"));

	for(int i=0; i<num; ++i)
	{
		str location = CTF::TrimColor(Info_ValueForKey(cs.c_str(), va("loc%d", i)));


		//update max
		if(location.length() > CTF::ctf_maxLocationLength)
			CTF::ctf_maxLocationLength = location.length();

		CTF::ctf_locations.push_back(location.c_str());
	}
}

} //~extern "C"


namespace CTF{

//
//returns whether we are on a team or not
//
inline bool OnTeam()
{
	if	(	
		cg.snap->ps.stats[STAT_TEAM] != TEAM_RED
		&&
		cg.snap->ps.stats[STAT_TEAM] != TEAM_BLUE
		)
	{
		return false;
	}

	return true;
}

//
//DrawMOTD - draw message of the day to the screen
//
void DrawMOTD()
{
	const float scale = 1.0f;
	float y = 50.0f;

	cvar_t* sv_motd = cgi.Cvar_Get("sv_motd", "", CVAR_SERVERINFO);
	if(!sv_motd)
		return;

	std::string motd = sv_motd->string;
	if(motd.empty())
		return;

	SetColor(colorWhite);
	std::string line = "";
	int pos = 0;

	while(!motd.empty())
	{
		pos = motd.find("\n");
		if(pos < 0) pos = motd.size()+1;

		line = motd.substr(0, pos-1);

		if(line == motd)
			motd = "";
		else
			motd = motd.substr(pos+1);

		msg_free.AddItem(line,
			SCREEN_W/2 - (TrimColor(line.c_str()).length()*FONT_WIDTH*scale)/2.0f, y,
			scale, 10.0f
		);

		y += FONT_HEIGHT*scale;
	}
}

//
//Draw flag status icons beside the hud's team score display.
//Draw OUR flag on top
//
void DrawFlags()
{
	int team = cg.snap->ps.stats[STAT_TEAM];
	int width = FLAG_ICON_WIDTH;
	
	//
	//widen the flag status icons & score displays to fit widest score string
	//

	//get highest team score
	int score = cg.snap->ps.stats[STAT_REDSCORE];
	if(cg.snap->ps.stats[STAT_BLUESCORE] > score)
		score = cg.snap->ps.stats[STAT_BLUESCORE];

	if(score > 99) //ie if score is 3 digits or more
	{
		char buf[16];
		itoa(score, buf, 10);
		width = (FONT_WIDTH*1.1f) * strlen(buf);
	}

		
	//blue status
	int			x		= SCREEN_W - width * (team == TEAM_BLUE ? 2 : 1);
	int			y		= SCREEN_H - FLAG_ICON_HEIGHT*2;
	int			status	= cg.snap->ps.stats[STAT_BLUEFLAGSTATUS];
	qhandle_t	handle	= cgs.media.CTFFlagShaders[status]+3;
	
	DrawPic(x, y, width, FLAG_ICON_HEIGHT, 0, 0, 1, 1, handle);

	//blue score
	vec4_t col = {0.0f, 0.0f, 1.0f, 0.7f};
	SetColor(col);
	DrawBox(x, y+FLAG_ICON_HEIGHT, width, FLAG_ICON_HEIGHT);

	char num[16];
	itoa(cg.snap->ps.stats[STAT_BLUESCORE], num, 10);
	SetColor(colorWhite);

	DrawString(num,
		x + width/2.0f - strlen(num)*FONT_WIDTH/2.0f,
		y + FLAG_ICON_HEIGHT + FLAG_ICON_HEIGHT/2.0f - FONT_HEIGHT/2.0f,
		1.0f);

	//red status
	x		=	SCREEN_W - width * (team == TEAM_BLUE ? 1 : 2);
	status	=	cg.snap->ps.stats[STAT_REDFLAGSTATUS];
	handle	=	cgs.media.CTFFlagShaders[status];	

	DrawPic(x, y, width, FLAG_ICON_HEIGHT, 0, 0, 1, 1, handle);
	
	//red score
	col[0] = 1.0f;
	col[2] = 0.0f;
	SetColor(col);
	DrawBox(x, y+FLAG_ICON_HEIGHT, width, FLAG_ICON_HEIGHT);

	SetColor(colorWhite);
	itoa(cg.snap->ps.stats[STAT_REDSCORE], num, 10);
	DrawString(num,
		x + width/2.0f - strlen(num)*FONT_WIDTH/2.0f,
		y + FLAG_ICON_HEIGHT + FLAG_ICON_HEIGHT/2.0f - FONT_HEIGHT/2.0f,
		1.0f);

	//
	//capturelimit
	//
	SetColor(0.5, 0.5, 0.5, 1.0f);
	x = SCREEN_W - width*2.2f;
	itoa(cgs.capturelimit, num, 10);

	// Don't show the cap limit unless its been set
	if(cgs.capturelimit > 0)
	{
		DrawString(num, 
			x - strlen(num)*FONT_WIDTH+1,
			y + FLAG_ICON_HEIGHT + FLAG_ICON_HEIGHT/2.0f - FONT_HEIGHT/2.0f,
			1.0f);
	}

	//if we have the flag, draw a little icon telling us
	SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	if(cg.snap->ps.stats[STAT_GOTFLAG])
	{
		DrawPic(
			x - FLAG_ICON_WIDTH*2, SCREEN_H-HUD_BAR_HEIGHT+5, 
			HUD_BAR_HEIGHT-10, HUD_BAR_HEIGHT-10, 0, 0, 1, 1, 
			cgs.media.CTFFlagShaders[0 + ((team == TEAM_BLUE) ? 0 : 3)]
		);
	}
}

//
//Draws a large translucent box taking up (about) the bottom eighth of the screen
//in the color of the client's team
//
void DrawTeamLine()
{
	int team = 0;

	if(cg.snap->ps.stats[STAT_TEAM] == TEAM_BLUE)
		team = 1;

	SetColor(TEAM_COLOR[team]);
	DrawBox(0, SCREEN_H-HUD_BAR_HEIGHT, SCREEN_W, HUD_BAR_HEIGHT);
	//DrawPic(0, SCREEN_H-HUD_BAR_HEIGHT, SCREEN_W, HUD_BAR_HEIGHT, 0, 0, 1, 1, cgs.media.hudbar);

}

//
//DrawTeamScores - draw's a team's scores to the screen.
//Called from the scoreboard draw func
//
float DrawTeamScores(int team, float x, float y, float scale)
{
	float w = SCREEN_W*0.5f;

	//build a list of the players on our team
	std::vector<int> teamclients;

	for(int i=0; i<MAX_CLIENTS; ++i)
	{
		if(cgs.clientinfo[i].team == team)
			teamclients.push_back(i);
	}

	//sort list by score
	std::sort(teamclients.begin(), teamclients.end(), FragCompare);
	

	float lineheight = scale*FONT_HEIGHT+10.0f;

	//
	//draw info for each client on current team
	//
	int ping	= 0;
	bool ready	= false;

	for(std::vector<int>::iterator it = teamclients.begin(); it != teamclients.end(); ++it)
	{
		ping  = cgs.clientinfo[*it].ping;
		ready = cgs.clientinfo[*it].intermissionReady != 0;

		//if we are drawing OUR details...
		if(*it == cg.snap->ps.clientNum)
		{
			//draw a colored box around the item
			SetColor(0.4f, 0.4f, 0.4f, 0.7f);
			DrawBox(x+5, y+2, w-10.0f, lineheight-4);

			//get a more recent ping & intermission_ready value locally
			ping  = cg.snap->ping;
			ready = intermissionReady;

		}

		//we are drawing our last killer's info
		else if(*it == cgs.clientinfo[cg.snap->ps.clientNum].lastkiller)
		{
			//draw a grey box around the item
			SetColor(0.4f, 0.4f, 0.1f, 0.7f);
			DrawBox(x+5, y+2, w-10.0f, lineheight-4);
		}

		//draw score, ping, time & name
		if(ping > 999)
			ping = 999;
		else if(ping < 0)
			ping = 0;

		SetColor(colorWhite);
		DrawString(va("%d", cgs.clientinfo[*it].frags), x+SB_COLUMNS[0]+2, y+4, scale);
		DrawString(va("%d", cgs.clientinfo[*it].caps),  x+SB_COLUMNS[1]+2, y+4, scale);
		DrawString(va("%d", ping),						x+SB_COLUMNS[2]+2, y+4, scale);
		DrawString(va("%d", cgs.clientinfo[*it].time),	x+SB_COLUMNS[3]+2, y+4, scale);
		DrawString(CapString(cgs.clientinfo[*it].name, 16).c_str(),
														x+SB_COLUMNS[4]+2, y+4, scale);
		if(ready)
		{
			DrawString(S_COLOR_WHITE "READY", x + SCREEN_W/2.0f - FONT_WIDTH*scale*6, y+4, 
					scale, true);
		}

		//if this client has the flag, draw it!
		if(cgs.clientinfo[*it].flag != 0)
		{
			qhandle_t handle = cgs.clientinfo[*it].team == TEAM_BLUE ? 
										cgs.media.CTFFlagShaders[0] : cgs.media.CTFFlagShaders[3];

			DrawPic(x+SB_COLUMNS[4]-lineheight, y+1, lineheight, lineheight-2, 0, 0, 1, 1, handle);
		}

		//move down a line
		y += lineheight;
	}

	return y;
}

//
//DrawSpectators - on the scoreboard
//
const float SB_SPEC_COLS[] = {
	200.0f, 300.0f, 400.0f
};

void DrawSpectators(float y)
{
	const float headingScale = 0.8f;
	const float nameScale = 0.9f;
	static float xOffset = SCREEN_W/2;

	//make a string containing all spectators names separated by comma's
	std::string specString = "";
	for(int i=0; i<MAX_CLIENTS; ++i)
	{
		if(cgs.clientinfo[i].team == TEAM_NONE && strlen(cgs.clientinfo[i].name) > 0)
		{
			if(!specString.empty())
				specString += S_COLOR_WHITE ", ";

			specString += cgs.clientinfo[i].name;			
		}
	}

	//dont draw anything if there arent any spectators
	if(specString.empty())
		return;

	//draw a 'SPECTATING:' heading bar
	SetColor(0.7f, 0.7f, 0.7f, 1.0f);
	DrawPic(0, y, SCREEN_W/2, FONT_HEIGHT*headingScale+4, 0, 0, 1, 1, cgs.media.sb_lfade);
	DrawPic(SCREEN_W/2, y, SCREEN_W/2, FONT_HEIGHT*headingScale+4, 0, 0, 1, 1, cgs.media.sb_rfade);

	SetColor(0.1f, 0.1f, 0.1f, 1.0f);
	DrawAlignedString("SPECTATING:", 0, y+2, headingScale, false, 1.0f, ALIGN_CENTER, ALIGN_NONE);
	SetColor(colorWhite);

	//make names scroll a sixth of the screen every second (so it takes 6 seconds for
	//a name top completely traverse the screen)
	xOffset -= (SCREEN_WIDTH/6.0f) * (cg.frametime/1000.0f);

	//if all names have scrolled off the screen, start them off at the far right again
	if(xOffset < -(TrimColor(specString.c_str()).length()*FONT_WIDTH*nameScale))
		xOffset = SCREEN_W;

	DrawString(specString.c_str(), xOffset, y+FONT_HEIGHT*headingScale+8,
		nameScale, true, 1);
}

//
//DrawScoreBoard
//
bool DrawScoreboard(bool forceDraw = false)
{
	if(!forceDraw && !cg.showScores)
		return false;

	//request an update from the server every 2 seconds
	static float lastUpdate = 0.0f;
	if(cg.time - lastUpdate >= 2000.0f)
	{
		lastUpdate = cg.time;
		cgi.Cmd_Execute(EXEC_NOW, "updatescores");
	}



	float scale = 1.0f;

	cgi.Cmd_Execute(EXEC_NOW, "ui_hud 0");

	//draw reward icons
	float y = GetRewardSystem().DrawRewardsSB(5);

	//draw mesages
	CTF::msg_sb.SetY(y);
	CTF::msg_normal.Update();
	CTF::msg_team.Update();
	y = CTF::msg_sb.Draw();

	//
	//draw red & blue scores
	//
	CTF::SetColor(colorRed);
	DrawAlignedString(va("Score: %d", cg.snap->ps.stats[STAT_REDSCORE]), 0, y, 
		0.8f, true, 1.0f, ALIGN_NONE, ALIGN_NONE);

	CTF::SetColor(0.3f, 0.3f, 1.0f, 1.0f);
	DrawAlignedString(va("Score: %d", cg.snap->ps.stats[STAT_BLUESCORE]), 0, y, 
		0.8f, true, 1.0f, ALIGN_RIGHT, ALIGN_NONE);
	
	//
	//draw the "Blue team leads 5 to 2" message
	//
	char teamstatus[128];

	if(cg.snap->ps.stats[STAT_BLUESCORE] == cg.snap->ps.stats[STAT_REDSCORE])
	{
		sprintf(teamstatus, "Teams are Tied at %d", 
			cg.snap->ps.stats[STAT_BLUESCORE]);

		SetColor(colorLtGrey);
	}
	else if(cg.snap->ps.stats[STAT_BLUESCORE] > cg.snap->ps.stats[STAT_REDSCORE])
	{
		sprintf(teamstatus, "Blue leads %d to %d", cg.snap->ps.stats[STAT_BLUESCORE], cg.snap->ps.stats[STAT_REDSCORE]);
		SetColor(0.3f, 0.3f, 1.0f, 1.0f); //dont use colorBlue, it's too dark
	}
	else
	{
		sprintf(teamstatus, "Red leads %d to %d", cg.snap->ps.stats[STAT_REDSCORE], cg.snap->ps.stats[STAT_BLUESCORE]);
		SetColor(colorRed);
	}

	DrawAlignedString(teamstatus, 0, y, 1.0f, true, 1.0f, ALIGN_CENTER, ALIGN_NONE);
	y += 30.0f;

	//
	//draw headings
	//
	float x = 0;
	scale = 0.5f;

	SetColor(0.4f, 0.4f, 0.4f, 1.0f);
	DrawPic(x, y-2, SCREEN_W/2, FONT_HEIGHT*scale+4, 0, 0, 1, 1, cgs.media.sb_lfade);
	DrawPic(SCREEN_W/2, y-2, SCREEN_W/2, FONT_HEIGHT*scale+4, 0, 0, 1, 1, cgs.media.sb_rfade);

	SetColor(colorWhite);

	for(int i=0; i<2; ++i)
	{
		DrawString("SCORE", x+SB_HCOLUMNS[0], y, scale, true);
		DrawString("CAPS",  x+SB_HCOLUMNS[1], y, scale, true);
		DrawString("PING",  x+SB_HCOLUMNS[2], y, scale, true);
		DrawString("TIME",  x+SB_HCOLUMNS[3], y, scale, true);
		DrawString("NAME",  x+SB_HCOLUMNS[4], y, scale, true);

		x += SCREEN_W/2.0f;
	}

	y += FONT_HEIGHT*scale;

	//underline heading row
	SetColor(colorYellow);
	DrawBox(0, y+2.0f, SCREEN_W, 2);

	y += 3.0f;

	//
	//make sure we don't draw off the bottom of the screen by choosing a good
	//scale value for the text
	//

	//get count of players from team with the most players
	int num = 0;
	int red = 0;
	int blue = 0;
	for(i=0; i<MAX_CLIENTS; ++i)
	{
		if(cgs.clientinfo[i].team == TEAM_RED)
			++red;
		else if(cgs.clientinfo[i].team == TEAM_BLUE)
			++blue;
	}

	num = red > blue ? red : blue;

	//0.65 is default scale (ie the scale that will be used if everything fits on screen ok)
	scale = 0.65f;

	//there are 5 pixels of padding above & below each item
	//therefore the total height of the scoreboard is (HeightOfFont+10) * NumberOfPlayers
	float total = (10.0f + (FONT_HEIGHT*scale)) * num;

	//if we are gonna spill over the bottom, modify the scale so we don't!
	if(total + y > SCREEN_H)
	{
		scale = ((SCREEN_H-y) / total) * 0.75f;
		total *= scale;
	}

	//
	//draw team boxes
	//
	SetColor(colorWhite);
	++y;
	
	//red
	SetColor(0.8f, 0.0f, 0.0f, 1.0f);
	DrawPic(0, y, SCREEN_W/2.0f, total, 0, 0, 1, 1, cgs.media.sb_lfade);

	//blue
	SetColor(0.0f, 0.0f, 0.8f, 1.0f);
	DrawPic(SCREEN_W/2.0f, y, SCREEN_W/2.0f, total, 0, 0, 1, 1, cgs.media.sb_rfade);

	//seperator
	SetColor(colorYellow);
	DrawBox(SCREEN_W/2.0f-1.0f, y, 2.0f, total);

	//draw client info for each team
	float ySpec = SCREEN_H*0.7f;
	
	float yTeam = DrawTeamScores(TEAM_RED,	0.0f,			y, scale);
	if(yTeam > ySpec)
		ySpec = yTeam;

	yTeam = DrawTeamScores(TEAM_BLUE,	SCREEN_W/2.0f,	y, scale);
	if(yTeam > ySpec)
		ySpec = yTeam + 20;

	DrawSpectators(ySpec);

	return true;
}

//
//return entnum of ent in front of us
//
void ScanForCrosshairEntity(void)
{
	vec3_t		start, end;

	vec3_t min = {-15, -15, -15};
	vec3_t max = { 15,  15,  15};

	VectorCopy( cg.playerHeadPos, start );
	VectorMA( start, 4096, cg.refdef.viewaxis[0], end );

	trace_t trace;
	CG_Trace(&trace, start, min, max, end, 
		cg.snap->ps.clientNum, CONTENTS_SOLID|CONTENTS_BODY, qtrue, qtrue, "");

	//if the ent is invalid, dont use it
	//that includes if the ent is in fog, ent is not on a team, entnum is >max
	if(
		trace.entityNum >= MAX_CLIENTS ||
		cgi.CM_PointContents(trace.endpos, 0) & CONTENTS_FOG ||
		(
		cgs.clientinfo[trace.entityNum].team != TEAM_RED && 
		cgs.clientinfo[trace.entityNum].team != TEAM_BLUE
		)
	  )
	{
		return;
	}

	cg.crosshairClientNum = trace.entityNum;
	cg.crosshairClientTime= cg.time;
}

//
//DrawCrosshairName - print name of person in front of us
//
void DrawCrosshairName()
{
	const float namestay	= 1200.0f;
	ScanForCrosshairEntity();

	if(!cg_drawCrosshairNames->integer)
		return;

	//if invalid client, bail
	if(cg.crosshairClientNum < 0 || cg.crosshairClientNum >= MAX_CLIENTS)
		return;
	
	//calculate how much we should fade
	float fade = 1.0f - ((cg.time - cg.crosshairClientTime) / namestay);

	//if we faded out completely, invalidate the client num and bail
	if(fade <= 0.0f)
	{
		cg.crosshairClientNum = -1;
		return;
	}

	//draw ent's name centered close to the bottom of the screen
	DrawAlignedString(va("%s%s", S_COLOR_WHITE, cgs.clientinfo[cg.crosshairClientNum].name), 
		0, SCREEN_H - HUD_BAR_HEIGHT - FONT_HEIGHT*4.0f, 1.0f, true, fade,
		ALIGN_CENTER, ALIGN_NONE);
}

//
//draw framerate
//
float DrawFPS(float y)
{
	if(!cg_drawFPS->integer)
		return y;

	static int lastFPScheck = 0;
	static int fpsChecks	= 0;
	static int fpsTotal		= 0;
	static char fps[16];

	const int	UPDATE_DELAY	= 150;
	const float scale			= 0.8f;

	//only update FPS x times a second for readability,
	//otherwise it flashes around so much it is impossible to read
	//note: the displayed fps value is the average FPS since last update
	
	if(cg.time-lastFPScheck > UPDATE_DELAY && fpsChecks)
	{
		lastFPScheck = cg.time;
		sprintf(fps, "%dFPS", fpsTotal / fpsChecks);

		fpsChecks = 0;
		fpsTotal  = 0;
	}
	else //update average
	{
		++fpsChecks;
		fpsTotal +=  1000 / (cg.frametime + 1);
	}

	y -= FONT_HEIGHT*scale;
	SetColor(colorWhite);
	DrawString(fps, SCREEN_W - strlen(fps)*FONT_WIDTH*scale, y, scale);

	return y;
}

//
//drawping
//
float DrawPing(float y)
{
	if(!cg_drawPing->integer)
		return y;

	const float scale = 0.8f;
	const int	UPDATE_DELAY	= 250;
	static int	lastCheck		= 0;
	static int	ping			= 0;

	if(cg.time - lastCheck >= UPDATE_DELAY)
	{
		ping = cg.snap->ping;

		if(ping < 0)	ping = 0;
		if(ping > 999)	ping = 999;

		lastCheck = cg.time;
	}

	const char* pingString = va("Ping: %d", ping);
	DrawString(pingString, SCREEN_W - strlen(pingString)*FONT_WIDTH*scale, y, scale);

	return y + FONT_HEIGHT*scale;
}

//
//DrawTime
//
float DrawTime(float y)
{
	static cvar_t* timelimit = cgi.Cvar_Get("timelimit", "0", 0);

	if(!cg_drawTime->integer)
		return y;

	float scale = 0.75f;

	//IF less than 10 seconds left OR in sudden death, make time string be red
	if(	timelimit->integer > 0 &&
		(cg.time > timelimit->integer*60000 ||
		(cg.snap->ps.stats[STAT_TIMELEFT_MINUTES] == 0 &&
		cg.snap->ps.stats[STAT_TIMELEFT_SECONDS] <= 10))
	)
	{
		SetColor(1,0,0,1);
	}
	else
	{
		SetColor(1,1,1,1);
	}
	

	const char* timestring = va("%s%d:%s%d",
		cg.snap->ps.stats[STAT_TIMELEFT_MINUTES] < 10 ? "0" : "", 
		cg.snap->ps.stats[STAT_TIMELEFT_MINUTES],
		cg.snap->ps.stats[STAT_TIMELEFT_SECONDS] < 10 ? "0" : "",
		cg.snap->ps.stats[STAT_TIMELEFT_SECONDS]
		);

	DrawString(
		timestring,		
		SCREEN_W - FONT_WIDTH*strlen(timestring)*scale, y,
		scale
		);

	return y + FONT_HEIGHT*scale;
}

//
//DrawTeamOverlay
//
//format:
//FLAG | POWERUPS | NAME | LOCATION | HEALTH
//
float DrawTeamOverlay()
{
	if(!cg_drawTeamOverlay->integer)
		return 0.0f;

	//request update every 2 seconds
		static float lastUpdate = 0.0f;

		if(cg.time - lastUpdate >= 2000.0f)
		{
			lastUpdate = cg.time;
			cgi.Cmd_Execute(EXEC_NOW, "updatelocations");
		}

	//more constants
		const float scale			= 0.7f;
		const float ITEM_SEPERATION = 2.0f;
		const float ICON_SIZE		= FONT_HEIGHT * scale;

		const float COLS[] = {
			FONT_HEIGHT*scale + ITEM_SEPERATION,			//flag icon
			MaxTechs()*ICON_SIZE + ITEM_SEPERATION,			//techs
			MaxNameWidth()*FONT_WIDTH*scale + 10.0f,		//name
			ctf_maxLocationLength*FONT_WIDTH*scale + 10.0f,	//location
			ICON_SIZE+ITEM_SEPERATION,						//health icon
			FONT_WIDTH*scale * 3.0f + 5.0f,					//health
			ICON_SIZE+ITEM_SEPERATION,						//water icon
			FONT_WIDTH*scale * 3.0f + 5.0f					//water
		};


	//total width of all columns
		const float OVERLAY_WIDTH = 
			COLS[0] + COLS[1] + COLS[2] + COLS[3] + COLS[4] + COLS[5] + COLS[6] + COLS[7];

	//our team: 0 = red; 1 = blue
		int team = 0;
		if(cg.snap->ps.stats[STAT_TEAM] == TEAM_BLUE)
			team = 1;

	//colors for the background
		const vec4_t bg_lookat	= {0.5f, 0.5f, 0.5f, 0.6f};
		const vec4_t bg_norm	= {
			TEAM_COLOR[team][0], 
			TEAM_COLOR[team][1], 
			TEAM_COLOR[team][2], 
			TEAM_COLOR[team][3]
		};

	//draw info for each client on your team
		clientInfo_t* ci = NULL;
		float x = 0.0f;
		float y = 0.0f;

		for(int i=0; i<MAX_CLIENTS; ++i)
		{
			ci = &cgs.clientinfo[i];

			//if client is not on our team, ignore
				if(ci->team != cg.snap->ps.stats[STAT_TEAM])
					continue;

			//move to far left bit of overlay box
				x = SCREEN_W-OVERLAY_WIDTH;

			//highlight person we are looking at
				if(i == cg.crosshairClientNum)
					SetColor(bg_lookat);
				else
					SetColor(bg_norm);

				DrawBox(x, y, OVERLAY_WIDTH, FONT_HEIGHT*scale + ITEM_SEPERATION);

			//use the calculated color based on how far from bases the client is
				SetColor(ci->teamOverlayColor);

			//flag
				if(ci->flag)
				{
					//draw a line above and below the players info to highlight them
					SetColor(1, 1, 0, 0.5f);
					DrawBox(x, y, OVERLAY_WIDTH, 1);
					DrawBox(x, y+FONT_HEIGHT*scale-1, OVERLAY_WIDTH, 1);
					
					//draw the flag icon next to their name
					SetColor(1,1,1,1);
					DrawPic(x, y, COLS[0], COLS[0], 0, 0, 1, 1, 
							cgs.media.CTFFlagShaders[team == 1 ? 0 : 3]);
				}

			//techs
				x += COLS[0];			
				int handle = 0;

				for(int j=0; j<MAX_TECHS; ++j)
				{
					if(ci->techs[j] <= 0)
						continue;

					const char* techShader = GetTechIcon(ci->techs[j]);
					if(!techShader)
						continue;

					handle = cgi.R_RegisterShader(techShader);

					if(handle <= 0)
						continue;

					DrawPic(x, y, ICON_SIZE, ICON_SIZE, 0, 0, 1, 1, handle);
					x += ICON_SIZE;
				}

			//name
				x = (SCREEN_W - OVERLAY_WIDTH) + (COLS[0] + COLS[1]);
				DrawString(ci->name, x, y, scale);
				
			//location
				x += COLS[2];

				if(ci->location >= 0 && ci->location < ctf_locations.size())
				{
					SetColor(1,1,1,1);
					DrawString(ctf_locations[ci->location].c_str(), x+1, y+1, scale, false, 1-ci->teamOverlayColor[1]);

					SetColor(ci->teamOverlayColor);
					DrawString(ctf_locations[ci->location].c_str(), x, y, scale);
				}

			//health icon
				x += COLS[3];
				SetColor(1,1,1,1);

				if(ci->health <= 0)
					handle = cgs.media.ui_healthicon_tiny_dead;
				else if(ci->health <= 20)
					handle = cgs.media.ui_healthicon_tiny_low;
				else
					handle = cgs.media.ui_healthicon_tiny;

				DrawPic(x, y, COLS[4], COLS[4], 0, 0, 1, 1, handle);
			
			//health
				x += COLS[4];
				SetColor(PercentColor(100, ci->health));
				DrawString(va("%d", ci->health < 0 ? 0 : ci->health), x, y, scale);
				SetColor(ci->teamOverlayColor);

			//water icon
				x += COLS[5];
				DrawPic(x, y, COLS[4], COLS[4], 0, 0, 1, 1, cgs.media.ui_watericon_tiny);

			//water
				x += COLS[6];
				SetColor(PercentColor(100, ci->water));
				DrawString(va("%d", ci->water), x, y, scale);
				SetColor(ci->teamOverlayColor);

			//move down for next player
				y += FONT_HEIGHT*scale + ITEM_SEPERATION;
		}

	return y;
}

//
//DrawUnevenTeamWarning - if the difference in players on each team is >= 2, show a small
//warning icon on the hud
//
const float TEAM_WARNING_W = 64.0f;
const float TEAM_WARNING_H = 16.0f;
void DrawUnevenTeamWarning()
{
	int red = 0;
	int blue = 0;

	for(int i=0; i<MAX_CLIENTS; ++i)
	{
		if(cgs.clientinfo[i].team == TEAM_RED)
			++red;
		else if(cgs.clientinfo[i].team == TEAM_BLUE)
			++blue;
	}

	//only warn if player counds have a >=2 difference
	if(abs(red-blue) >= 2)
	{
		DrawPic(SCREEN_W - FLAG_ICON_WIDTH*2 - TEAM_WARNING_W, SCREEN_H - HUD_BAR_HEIGHT, 
			TEAM_WARNING_W, TEAM_WARNING_H, 0, 0, 1, 1, cgs.media.unEvenTeamWarning);
	}
}

//
//DrawVote
//
void DrawVote()
{
	if(strlen(ctf_voteString) <= 0)
		return;

	float scale = 0.6f;
	DrawString(va("%s%s", ctf_voteString, S_COLOR_WHITE), 
		10, SCREEN_H - HUD_BAR_HEIGHT*2, 1.0f, true, scale);
}

//
//DrawHud - draws hud bits
//
void DrawHud()
{
	float x					= 0.0f;
	float scale				= 1.0f;
	static bool dead		= false;

	if(!cg.snap)
		return;

	SetColor(colorWhite);

	//if we are dead, force scoreboard draw
		if(cg.snap->ps.stats[STAT_HEALTH] <= 0)
		{
			if(!dead)
			{
				CTF_ClearScoreboardMessages();
				cg.showScores = qtrue;				
				cgi.Cvar_Set("ui_hud", "0");
				dead = true;
			}

			DrawScoreboard(true);
			return;
		}

	//if we are alive, bail IF we draw the scoreboard
		else
		{
			if(dead)
			{
				cg.showScores = qfalse;
				dead = false;
			}
		}

	//if we draw the scoreboard normally (ie we are alive) bail
		if(DrawScoreboard())
			return;

	//check cvar...
		if(!cg_draw2D->integer)
			return;

	//
	//spectator bits
	//
		if(!OnTeam())
		{
			//if we are sollowing someone, draw a "Following: BILL" message,
			//else sinply draw "SPECTATOR"
				if(followEntnum >= 0 && followEntnum < MAX_CLIENTS && followEntnum != cg.snap->ps.clientNum)
				{
					DrawAlignedString(va(S_COLOR_WHITE "Following: %s", cgs.clientinfo[followEntnum].name),
						0, SCREEN_H - FONT_HEIGHT*3.0f, 1.5f, true, 1.0f,
						ALIGN_CENTER, ALIGN_NONE);
				}
				else
				{
					DrawAlignedString(S_COLOR_WHITE "SPECTATOR", 0, SCREEN_H - FONT_HEIGHT*3.0f, 1.5f, true, 1.0f,
						ALIGN_CENTER, ALIGN_NONE);
				}

			//tell people how to join a team
				DrawAlignedString("Use Team Menu (\"ctf_teamJoin\") to join a team.", 
					0, SCREEN_H-FONT_HEIGHT, 0.75f, true, 1.0f,
					ALIGN_CENTER, ALIGN_NONE);

			//draw SOME 2d stuff (only that which a spectator needs)
				CTF_UpdateMsgSystem();
				DrawFPS(CG_DrawLagometer());
				DrawPing(DrawTime(0.0f));
				cgi.Cmd_Execute(EXEC_NOW, "ui_hud 0");

			//bail before we draw anything else
				return;
		}

	//
	//draw normal player bits
	//
		DrawTeamLine();							//big red/blue line at bottom of screen
		CTF_UpdateMsgSystem();					//ALL messages
		DrawFlags();							//flag status displays & capture scores/limit
		UpdateTechs();							//techs & time left on them
		DrawCrosshairName();					//name of person we are looking at
		DrawPing(DrawTime(DrawTeamOverlay()));	//ping & team overlay box
		DrawUnevenTeamWarning();				//TEAMS warning
		DrawFPS(CG_DrawLagometer());			//FPS display & lagometer
		DrawVote();								//current vote msg
		CTF::GetRewardSystem().DrawRewards();	//reward icons
		CTF::GetPickupManager().DrawItems();	//pickup icons

	//draw Following: Bill message
		if(followEntnum >= 0 && followEntnum < MAX_CLIENTS && followEntnum != cg.snap->ps.clientNum)
		{
			DrawAlignedString(va(S_COLOR_WHITE "Following: %s", cgs.clientinfo[followEntnum].name),
				0, SCREEN_H - FONT_HEIGHT*4.0f, 1.5f, true, 1.0f,
				ALIGN_CENTER, ALIGN_NONE);
		}

	//force ui_hud to 1 - TODO: make ui_hud read only so user cannot change this 
	//(which will make this bit redundant)
		cgi.Cmd_Execute(EXEC_NOW, "ui_hud 1");

	//temp store for itoa's
		char buf[16];
	
	//health
		SetColor(PercentColor(100, cg.snap->ps.stats[STAT_HEALTH]));

		scale = 0.65f;
		x = 5.0f;
		DrawPic(x, SCREEN_H-HUD_ICON_HEIGHT, HUD_ICON_WIDTH, HUD_ICON_HEIGHT, 0, 0, 1, 1, 
			cgs.media.ui_healthicon);
		DrawNumber(cg.snap->ps.stats[STAT_HEALTH], 
			x+HUD_ICON_WIDTH+2, SCREEN_H - HUD_ICON_HEIGHT/2 - (NUM_FONT_HEIGHT*scale*0.5f), scale);

	//water
		SetColor(PercentColor(100, cg.snap->ps.stats[STAT_WATER_LEVEL]));
		
		x = HUD_ICON_WIDTH*4.0f;
		DrawPic(x, SCREEN_H-HUD_ICON_HEIGHT, HUD_ICON_WIDTH, HUD_ICON_HEIGHT, 0, 0, 1, 1, 
			cgs.media.ui_watericon);

		DrawNumber(cg.snap->ps.stats[STAT_WATER_LEVEL], 
			x+HUD_ICON_WIDTH + 2, SCREEN_H - HUD_ICON_HEIGHT/2 - (NUM_FONT_HEIGHT*scale*0.5f), scale);

	//=============
	//AMMO
	//=============
			
	//primary
		float numscale = 0.65f;
		int ammo = cg.snap->ps.stats[STAT_AMMO_LEFT];

		if(cg.snap->ps.stats[STAT_AMMO_LEFT] || cg.snap->ps.stats[STAT_CLIPAMMO_LEFT])
		{
			x = HUD_ICON_WIDTH*11.0f;
			

			SetColor(PercentColor(cg.snap->ps.stats[STAT_MAXAMMO_LEFT], ammo, colorWhite, colorWhite));
			
			itoa(ammo, buf, 10);
			x -= strlen(buf)*NUM_FONT_WIDTH*numscale;

			DrawNumber(
				ammo, 
				x,
				SCREEN_H - HUD_BAR_HEIGHT*0.5f - NUM_FONT_HEIGHT*numscale*0.5f,
				numscale
			);

		//primary, clip
			scale = 0.9f;
			ammo = cg.snap->ps.stats[STAT_CLIPAMMO_LEFT];

			if(ammo)
			{
				SetColor(colorWhite);
				DrawPic(x-20, SCREEN_H-16,
					16, 16, 0, 0, 1, 1, cgs.media.ui_gunclip);

				SetColor(PercentColor(cg.snap->ps.stats[STAT_MAXCLIPAMMO_LEFT], ammo, colorWhite, colorWhite));

				itoa(ammo, buf, 10);
				DrawString(buf, 
					x-strlen(buf)*FONT_WIDTH*scale,
					SCREEN_H - HUD_BAR_HEIGHT*0.5f - NUM_FONT_HEIGHT*numscale*0.5f,
					scale
				);
			}
		}
			
	//secondary		
		ammo = cg.snap->ps.stats[STAT_AMMO_RIGHT];
		
		if(cg.snap->ps.stats[STAT_AMMO_RIGHT] || cg.snap->ps.stats[STAT_CLIPAMMO_RIGHT])
		{
			x = HUD_ICON_WIDTH*13;
			itoa(ammo, buf, 10);

			SetColor(PercentColor(cg.snap->ps.stats[STAT_MAXAMMO_RIGHT], ammo, colorWhite, colorWhite));
			
			DrawNumber(
				ammo, 
				x,
				SCREEN_H - HUD_BAR_HEIGHT*0.5f - NUM_FONT_HEIGHT*numscale*0.5f,
				numscale
			);

		//secondary, clip
			ammo = cg.snap->ps.stats[STAT_CLIPAMMO_RIGHT];

			if(ammo)
			{
				SetColor(colorWhite);
				DrawPic(x+strlen(buf)*NUM_FONT_WIDTH*numscale+4, SCREEN_H-16,
					16, 16, 0, 0, 1, 1, cgs.media.ui_gunclip);

				SetColor(PercentColor(cg.snap->ps.stats[STAT_MAXCLIPAMMO_RIGHT], ammo, colorWhite, colorWhite));

				DrawString(va("%d", ammo), 
					x+strlen(buf)*NUM_FONT_WIDTH*numscale,
					SCREEN_H - HUD_BAR_HEIGHT*0.5f - NUM_FONT_HEIGHT*numscale*0.5f,
					scale
				);
			}
		}
}

} //~namespace CTF