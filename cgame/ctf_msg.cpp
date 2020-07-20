////////////////////////////////////////////////////////////////////
//
// Hud Messaging System Implementation
//
// -Yorvik
//
////////////////////////////////////////////////////////////////////

#include "ctf_msg.h"
#include "cg_local.h"
#include "ctf_cg_drawtools.h"
#include "str.h"

#pragma warning(push, 1)
#include <list>
#pragma warning(pop)

const float MSG_TIME		= 4000.0f;

namespace CTF{

//Message Systems
CTF_HudMsgSystem		msg_normal;	//normal hud messages (top left)
CTF_TeamMsgSystem		msg_team;	//team hud messages (bottom left)
CTF_CenterMsgSystem		msg_center;	//centerprinting
CTF_ClientText			msg_free;	//print anywhere
CTF_ScoreboardMsgSystem	msg_sb;		//scoreboard messages (only drawn when scoreboard is shown)

};

//
//CG_HudPrint_UI_f - called when console command "hudprint" is recieved
//
void CG_HudPrint_UI_f()
{
	std::string msg;
	
	for(int i=1; i<cgi.Argc(); ++i)
	{
		msg += cgi.Argv(i);

		//dont add a space if this is the last arg
		if(i < cgi.Argc()-1)
			msg += " ";
	}

	CTF::msg_normal.AddMessage(msg, cg.time + MSG_TIME);
	cgi.Printf("%s\n", CTF::TrimColor(msg.c_str()).c_str());
}

//
//CG_TeamHudPrint_UI_f - called when console command team_hudprint is recieved
//
void CG_TeamHudPrint_UI_f()
{
	std::string msg;
	
	for(int i=1; i<cgi.Argc(); ++i)
	{
		msg += cgi.Argv(i);
		
		//dont add a space if this is the last arg
		if(i < cgi.Argc()-1)
			msg += " ";
	}

	CTF::msg_team.AddMessage(msg.c_str(), cg.time + MSG_TIME);
	cgi.Printf("%s\n", CTF::TrimColor(msg.c_str()).c_str());
}

//
//CG_CenterPrint_UI_f - called when console command centerprint is recieved
//
void CG_CenterPrint_UI_f()
{
	std::string msg;
	
	for(int i=1; i<cgi.Argc(); ++i)
	{
		msg += cgi.Argv(i);
		
		//dont add a space if this is the last arg
		if(i < cgi.Argc()-1)
			msg += " ";
	}

	CTF::msg_center.AddMessage(msg.c_str(), cg.time + MSG_TIME);
	cgi.Printf("%s\n", CTF::TrimColor(msg.c_str()).c_str());
}

//
//CG_ScoreboardPrint_UI_f - called when console command scoreboardprint is recieved
//
void CG_ScoreboardPrint_UI_f()
{
	std::string msg;
	for(int i=1; i<cgi.Argc(); ++i)
	{
		msg += cgi.Argv(i);
		
		//dont add a space if this is the last arg
		if(i < cgi.Argc()-1)
			msg += " ";
	}

	CTF::msg_sb.AddMessage(msg.c_str(), cg.time + 100);
}

extern "C"{
	//
	//Initialisation - called once per map/vid restart
	//
	void CTF_InitMsgSystem()
	{
		CTF::msg_center.Init();
		CTF::msg_normal.Init();
		CTF::msg_team.Init();
		CTF::msg_sb.Init();
	}

	//
	//Updateing - called once per frame
	//
	void CTF_UpdateMsgSystem()
	{
		CTF::msg_center.Update();
		CTF::msg_free.Update();
		CTF::msg_normal.Update();
		CTF::msg_team.Update();
	}

	
	//Clear scoreboard messages - this should only be called on "-scores"	
	void CTF_ClearScoreboardMessages()
	{
		CTF::msg_sb.Clear();
	}
	//Update scoreboard messages - this should only be called on "+scores"
	void CTF_UpdateScoreboardMessages()
	{
		CTF::msg_sb.Update();
	}
}

///////////////////////////////////////////////////////////////////////////////
//
// CTF_HudMsgSystem (messages in top left corner)
//
///////////////////////////////////////////////////////////////////////////////

//
//constructor
//
CTF_HudMsgSystem::CTF_HudMsgSystem() :
	m_x(0.0f),
	m_y(0.0f),
	m_scale(1.0f)
{}

CTF_HudMsgSystem::~CTF_HudMsgSystem()
{}

//
//Set upper left corner of message window
//
void CTF_HudMsgSystem::SetWindowPos(float x, float y)
{
	m_x = x;
	m_y = y;
}

//
//Init - set's window pos to preset location
//
void CTF_HudMsgSystem::Init()
{
	m_scale = 0.7f;
	m_x = 2;
	m_y = 2;
	m_maxMessages = 3;
	m_maxlen = (SCREEN_W-m_x) / (FONT_WIDTH*m_scale);
}

//
//Add a message to the list
//
void CTF_HudMsgSystem::AddMessage(const std::string& msg, const float birthTime)
{
	m_messages.push_back(CTF_SingleMessage(msg, birthTime));

	//make sure we dont have too many messages
	while(m_messages.size() > m_maxMessages)
		m_messages.pop_front();
}

//
//Clear list
//
void CTF_HudMsgSystem::Clear()
{
	m_messages.clear();
}

//
//Update - kill old messages
//
void CTF_HudMsgSystem::Update()
{
	MessageListIterator end = m_messages.end();
	MessageListIterator it;

	while( (it = m_messages.begin()) != m_messages.end() )
	{
		if(it->deathTime <= cg.time)
		{
			m_messages.pop_front();
			
			//skip past the following break
			continue;
		}

		//if we reach here - none of the other messages will be too old anyway, so skip them
		break;
	}



	Draw();
}

//
//Draw all messages to client screen
//
float CTF_HudMsgSystem::Draw()
{
	if(m_messages.empty())
		return 0.0f;

	MessageListIterator end = m_messages.end();
	MessageListIterator it  = m_messages.begin();
	int y = 0;

	while(it != end)
	{
		CTF::SetColor(NULL);
		int lines = CTF::TrimColor(it->msg.c_str()).length() / m_maxlen + 1;

		for(int i=0; i<lines; ++i)
		{
			CTF::DrawString(it->msg.substr(i*m_maxlen, m_maxlen).c_str(),
				m_x, m_y + y*FONT_HEIGHT*m_scale, m_scale, false);

			++y;
		}

		++it;
	}

	return 0.0f;
}

///////////////////////////////////////////////////////////////////////////////
//
//CTF_TeamMsgSystem (messages in bottom left corner)
//
///////////////////////////////////////////////////////////////////////////////

CTF_TeamMsgSystem::CTF_TeamMsgSystem() : CTF_HudMsgSystem()
{}

CTF_TeamMsgSystem::~CTF_TeamMsgSystem()
{}

//
//Init - set's window pos to preset location
//
void CTF_TeamMsgSystem::Init()
{
	m_maxMessages = 5;
	m_scale = 0.6f;
	m_x = 2;
	m_y = (SCREEN_H - HUD_BAR_HEIGHT) - FONT_HEIGHT*m_scale;
	m_maxlen = (SCREEN_W-m_x) / (FONT_WIDTH*m_scale);
}

float CTF_TeamMsgSystem::Draw()
{
	if(m_messages.empty())
		return 0.0f;

	MessageListIterator end = m_messages.end();
	MessageListIterator it  = m_messages.begin();
	int y = 0;
	int team = 0;

	if(cg.snap->ps.stats[STAT_TEAM] == TEAM_BLUE)
		team = 1;

	while(it != end)
	{
		int lines = CTF::TrimColor(it->msg.c_str()).length() / m_maxlen + 1;
		
		CTF::SetColor(TEAM_COLOR[team]);
		CTF::DrawBox(0,	m_y - (y+(lines-1))*FONT_HEIGHT*m_scale, 
								SCREEN_W, FONT_HEIGHT*m_scale*lines);

		for(int i=0; i<lines; ++i)
		{
			CTF::DrawString(it->msg.substr(i*m_maxlen, m_maxlen).c_str(), 
				m_x, m_y - (y+((lines-1)-i))*FONT_HEIGHT*m_scale, m_scale);

		}

		y += lines;
		++it;
	}

	return 0.0f;
}

///////////////////////////////////////////////////////////////////////////////
//
//CTF_CenterMsgSystem (messages in center of screen)
//
///////////////////////////////////////////////////////////////////////////////

void CTF_CenterMsgSystem::Init()
{
	m_maxMessages = 5;
	m_scale = 1.0f;
	m_y = SCREEN_H / 4.0f;
	m_maxlen = SCREEN_W / (FONT_WIDTH*m_scale);
}

float CTF_CenterMsgSystem::Draw()
{
	if(m_messages.empty())
		return 0.0f;

	MessageListIterator end = m_messages.end();
	MessageListIterator it  = m_messages.begin();
	int y = 0;
	float alpha = 1.0f;

	while(it != end)
	{
		alpha = (it->deathTime - cg.time) / 1000.0f;
		if(alpha < 0.0f) alpha = 0.0f;
		if(alpha > 1.0f) alpha = 1.0f;
		
		CTF::DrawAlignedString(it->msg.c_str(), 
			m_x, m_y + y*FONT_HEIGHT*m_scale,
			m_scale, true, alpha,
			CTF::ALIGN_CENTER, CTF::ALIGN_NONE);

		++y;
		++it;
	}

	return 0.0f;
}

///////////////////////////////////////////////////////////////////////////////
//
//ScoreboardMsg Implementation (messages drawn only when scoreboard is shown)
//
///////////////////////////////////////////////////////////////////////////////

void CTF_ScoreboardMsgSystem::Init()
{
	m_maxMessages = 5;
	m_scale = 1.0f;
	m_y = 50.0f;
	m_maxlen = SCREEN_W / (FONT_WIDTH*m_scale);
}

//
//Draw
//
float CTF_ScoreboardMsgSystem::Draw()
{
	if(m_messages.empty())
		return m_y;

	MessageListIterator end = m_messages.end();
	MessageListIterator it  = m_messages.begin();
	int y = 0;
	
	while(it != end)
	{
		CTF::DrawAlignedString(it->msg.c_str(), 
			m_x, m_y + y*FONT_HEIGHT*m_scale,
			m_scale, true, 1.0f,
			CTF::ALIGN_CENTER, CTF::ALIGN_NONE);

		++y;
		++it;
	}

	return m_y + (y+2)*FONT_HEIGHT*m_scale;
}

///////////////////////////////////////////////////////////////////////////////
//
//ClientText Implementation (messages anywhere)
//
///////////////////////////////////////////////////////////////////////////////

//
//parses the given string for justification info
//
//c = center
//e = end - ie width
//
//eg: "e-4" will generate width-4
//
float ParseCoordString(std::string& coord, float size, float min, float max)
{
	if(isalpha(static_cast<int>(coord[0])))
	{
		if(coord[0] == 'c' || coord[0] == 'C')
		{
			coord.erase(0, 1);
			if(coord[0] == '+')
				coord.erase(0, 1);

			return ((max-min)/2.0f + min) - (size)/2.0f + atof(coord.c_str());
		}

		if(coord[0] == 'e' || coord[0] == 'E')
		{
			coord.erase(0, 1);
			if(coord[0] == '+')
				coord.erase(0, 1);

			return (max - (size)) + atof(coord.c_str());
		}
	}

	return atof(coord.c_str());
}

//
//CG_DrawText_UI_f - called when console command "drawtext" is recieved
//
void CG_DrawText_UI_f()
{
	std::string msg;

	if(cgi.Argc() < 5)
	{
		cgi.Printf(	"\nUseage:\n"
					"clientdrawtext [x] [y] [scale] [life] [text]\n");
		return;
	}

	float scale	= atof(cgi.Argv(3));
	float life	= atof(cgi.Argv(4));

	for(int i=5; i<cgi.Argc(); ++i)
	{
		msg += cgi.Argv(i);

		//if this is the last word, dont bother adding a space after it
		if(i < cgi.Argc()-1)
			msg += " ";
	}
	
	//X
	float x	= ParseCoordString(std::string(CTF::TrimColor(cgi.Argv(1)).c_str()), 
					FONT_WIDTH*scale*CTF::TrimColor(msg.c_str()).length(), 0, SCREEN_W);
	
	//Y
	float y	= ParseCoordString(std::string(CTF::TrimColor(cgi.Argv(2)).c_str()), 
					FONT_HEIGHT*scale, 0, SCREEN_H);


	CTF::msg_free.AddItem(msg, x, y, scale, life);
}

//
//fake singleton access
//
CTF_ClientText& GetClientTextSystem()
{
	static CTF_ClientText msg;
	return msg;
}

//
//Clear - remove all client text items
//
void CTF_ClientText::Clear()
{
	m_textItems.clear();
}

//
//Draw all items
//
void CTF_ClientText::Draw()
{
	if(m_textItems.empty())
		return;

	TextItemListIterator end = m_textItems.end();
	TextItemListIterator it  = m_textItems.begin();

	CTF::ctf_fontalign_t horiz = CTF::ALIGN_NONE;
	CTF::ctf_fontalign_t vert  = CTF::ALIGN_NONE;
	float alpha = 1.0f;
	
	while(it != end)
	{
		if(it->x < 0.0f)
			horiz = CTF::ALIGN_CENTER;

		if(it->y < 0.0f)
			vert = CTF::ALIGN_CENTER;
		
		alpha = (it->deathtime-cg.time)/1000.0f;
		if(alpha > 1.0f) alpha = 1.0f;
		if(alpha < 0.0f) alpha = 0.0f;

		CTF::DrawAlignedString(it->msg.c_str(), it->x, it->y, it->scale, true, alpha, horiz, vert);

		horiz = vert = CTF::ALIGN_NONE;
		++it;
	}
}

//
//Remove old items, then draw
//
void CTF_ClientText::Update()
{
	if(m_textItems.empty())
		return;

	TextItemListIterator end = m_textItems.end();
	TextItemListIterator it  = m_textItems.begin();

	while(it != end)
	{
		if(cg.time >= it->deathtime)
		{
			TextItemListIterator tmp(it);
			++it;

			m_textItems.erase(tmp);
		}
		else
		{
			++it;
		}
	}

	Draw();
}

//
//Add text item
//
void CTF_ClientText::AddItem(std::string& sMsg, float fx, float fy, float fscale, float lifetime)
{
	m_textItems.push_back(
		CTF_ClientTextItem
			(
			sMsg,
			cg.time + (lifetime*1000.0f),
			fx, fy, fscale
			)
	);
}


