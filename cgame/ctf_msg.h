////////////////////////////////////////////////////////////////////
//
// Hud Messaging System Interface
//
// -Yorvik
//
////////////////////////////////////////////////////////////////////

#ifndef CTF_MSG_H_DEFINED
#define CTF_MSG_H_DEFINED

#ifdef __cplusplus

#pragma warning(push, 1)
#include <string>
#include <list>
#pragma warning(pop)

//declare global functions
extern "C"{
void CTF_InitMsgSystem(void);
void CTF_UpdateMsgSystem(void);
void CTF_ClearScoreboardMessages(void);
void CTF_UpdateScoreboardMessages(void);
}

//
//single message - ie a message and a time of birth
//
struct CTF_SingleMessage
{
	std::string		msg;
	float			deathTime;

	CTF_SingleMessage(std::string s, float f) : msg(s), deathTime(f) {}
	CTF_SingleMessage(const CTF_SingleMessage& message) : msg(message.msg), deathTime(message.deathTime) {}
};

//
//Message System class
//
class CTF_HudMsgSystem
{
public:
	CTF_HudMsgSystem();
	~CTF_HudMsgSystem();

	void SetWindowPos(float x, float y);
	void AddMessage(const std::string& msg, const float birthTime);
	void Clear();	
	void Init();
	void Update();

protected:
	typedef std::list<CTF_SingleMessage>	MessageList;
	typedef MessageList::iterator			MessageListIterator;


	float			m_x;
	float			m_y;
	float			m_scale;
	int				m_maxMessages;
	int				m_maxlen;
	
	MessageList		m_messages;	

	virtual float Draw();
};

//
//Team Message System Class
//
class CTF_TeamMsgSystem : public CTF_HudMsgSystem
{
	virtual float Draw();

public:
	CTF_TeamMsgSystem();
	~CTF_TeamMsgSystem();

	void Init();
};

//
//Center message class
//
class CTF_CenterMsgSystem : public CTF_HudMsgSystem
{
	virtual float Draw();

public:
	CTF_CenterMsgSystem()	{};
	~CTF_CenterMsgSystem()	{};

	void Init();
};


//
//Scoreboard Message Class
//
class CTF_ScoreboardMsgSystem : public CTF_HudMsgSystem
{
public:
	CTF_ScoreboardMsgSystem()	{};
	~CTF_ScoreboardMsgSystem()	{};

			void	Init();
			void	SetY(float y) {m_y = y;}
	virtual float	Draw();
};






//
//ClientTextItem
//
class CTF_ClientTextItem
{
public:
	std::string		msg;
	float			deathtime;
	float			x;
	float			y;
	float			scale;

	CTF_ClientTextItem(const CTF_ClientTextItem& other) :
		msg(other.msg),
		deathtime(other.deathtime),
		x(other.x),
		y(other.y),
		scale(other.scale)
	{}

	CTF_ClientTextItem(std::string sMsg, float fDeathTime, float fX, float fY, float fScale) :
		msg(sMsg),
		deathtime(fDeathTime),
		x(fX),
		y(fY),
		scale(fScale)
	{}
};

//
//ClientText
//
class CTF_ClientText
{
	typedef std::list<CTF_ClientTextItem>	TextItemList;
	typedef TextItemList::iterator			TextItemListIterator;

	TextItemList m_textItems;

	void Draw();

public:
	void Clear();
	void Update();
	void AddItem(std::string& sMsg, float fx, float fy, float scale = 1.0f, float lifetime = 5.0f);
};





#else //ifdef __cplusplus

//
//C STUFF GOES HERE
//
void CTF_InitMsgSystem(void);
void CTF_UpdateMsgSystem(void);
void CTF_ClearScoreboardMessages(void);
void CTF_UpdateScoreboardMessages(void);

#endif //ifdef __cplusplus

#endif //CTF_MSG_H_DEFINED