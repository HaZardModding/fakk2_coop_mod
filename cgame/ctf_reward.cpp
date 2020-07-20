//
//CTF Reward icons
//like defence, assist & capture
//
//-Yorvik
//

#pragma warning(disable:4786)
#include "ctf_reward.h"
#include "ctf_cg_drawtools.h"
#include "cg_commands.h"

const int	MAX_REWARD_DRAW_TIME	= 5000;
const float	REWARD_FADE_TIME		= 2000.0f;
const float	REWARD_ICON_SIZE		= 64.0f;
const float REWARD_ICON_Y			= 50.0f;

//compensate for C ugliness (with some more ugliness)
extern "C"{
	void CTF_AddReward(int type, int count, qboolean playsound)
	{
		CTF::GetRewardSystem().AddReward(type, count, playsound != qfalse);
	}

	void CTF_ClearRewards()
	{
		CTF::GetRewardSystem().Clear();
	}

	void CTF_CahceRewardIcons()
	{
		CTF::GetRewardSystem().CacheIcons();
	}
}//~extern "C"



enum reward_t
{
	REWARD_ASSIST,
	REWARD_DEFENCE,
	REWARD_CAPTURE,

	REWARD_MAX
};


namespace CTF{


//singleton
CTF_RewardSystem& GetRewardSystem()
{
	return CTF_RewardSystem::Instance();
}

///////////////////////////////////////////////////////////
//Single reward implementation
///////////////////////////////////////////////////////////
CTF_Reward::CTF_Reward() :
	m_iconHandle(-1),
	m_stopDrawTime(-1),
	m_count(0)
{}

void CTF_Reward::SetIconName(str name)
{
	m_iconName = name;
}

void CTF_Reward::SetSound(str snd)
{
	m_soundName = snd;
}

void CTF_Reward::CacheIcon()
{
	m_iconHandle = cgi.R_RegisterShaderNoMip(m_iconName.c_str());
}

void CTF_Reward::SetCount(int count, bool icon)
{
	m_count = count;

	if(icon)
		m_stopDrawTime = cg.time + MAX_REWARD_DRAW_TIME;
}

void CTF_Reward::AddCount(int n, bool icon)
{
	m_count += n;

	if(icon)
		m_stopDrawTime = cg.time + MAX_REWARD_DRAW_TIME;
}

//
//Draw
//
bool CTF_Reward::DrawIcon()
{
	if(cg.time > m_stopDrawTime || m_count <= 0 || m_iconHandle <= 0)
		return false;

	//fade out...
	float alpha = (m_stopDrawTime-cg.time) / REWARD_FADE_TIME;
	if(alpha > 1.0f) alpha = 1.0f;
	if(alpha < 0.0f) alpha = 0.0f;

	CTF::SetColor(1, 1, 1, alpha);

	//if we have too many icons to fit on the screen width, draw a single icon with the
	//amount written below it
	if(REWARD_ICON_SIZE * m_count >= SCREEN_W)
	{
		CTF::DrawPic(SCREEN_W/2 - REWARD_ICON_SIZE/2, REWARD_ICON_Y, 
			REWARD_ICON_SIZE, REWARD_ICON_SIZE, 0, 0, 1, 1, m_iconHandle);

		CTF::DrawAlignedString(va("%d", m_count), 0, REWARD_ICON_Y+REWARD_ICON_SIZE, 
			1.2f, true, alpha, CTF::ALIGN_CENTER, CTF::ALIGN_NONE);

		return true;
	}


	
	float x = (SCREEN_W/2) - (REWARD_ICON_SIZE/2) - ((m_count-1)*(REWARD_ICON_SIZE/2));
	for(int i=0; i<m_count; ++i, x += REWARD_ICON_SIZE)
	{
		CTF::DrawPic(x, REWARD_ICON_Y,
			REWARD_ICON_SIZE, REWARD_ICON_SIZE, 0, 0, 1, 1, m_iconHandle);
	}

	return true;
}

///////////////////////////////////////////////////////////
//Reward system class implementation
///////////////////////////////////////////////////////////

CTF_RewardSystem::CTF_RewardSystem() :
	m_lastReward(m_rewards.end())
{
	const char* icons[REWARD_MAX] = {
		"textures/hud/reward_assist.tga",
		"textures/hud/reward_defence.tga",
		"textures/hud/reward_capture.tga"
	};

	const char* sounds[REWARD_MAX] = {
		"sound/ctf/feedback/assist.wav",
		"sound/ctf/feedback/defend.wav",
		""
	};



	
	CTF_Reward dummy;

	for(int i=0; i<REWARD_MAX; ++i)
	{
		dummy.SetIconName(icons[i]);
		dummy.SetSound(sounds[i]);
		m_rewards.insert(std::make_pair(static_cast<reward_t>(i), dummy));
	}
}

//
//Add to the reward count
//
void CTF_RewardSystem::AddReward(int rewardType, int count, bool sound)
{
	CTF_RewardContainerIterator it = m_rewards.find(rewardType);

	if(it == m_rewards.end())
		return;

	m_lastReward = it;
	it->second.AddCount(count, sound);

	//play sound
	if(sound && strlen(it->second.GetSound()) > 0)
		cgi.S_StartLocalSound(it->second.GetSound());
}

//
//Draw reward icons
//
void CTF_RewardSystem::DrawRewards()
{
	if(m_lastReward == m_rewards.end())
		return;

	if(!m_lastReward->second.DrawIcon())
		m_lastReward = m_rewards.end();
}

//
//Cache reward icons
//
void CTF_RewardSystem::CacheIcons()
{
	for(CTF_RewardContainerIterator it = m_rewards.begin(); it != m_rewards.end(); ++it)
		it->second.CacheIcon();
}

//
//draw all reward icons for the scoreboard
//
float CTF_RewardSystem::DrawRewardsSB(float y)
{
	const float scale = 0.7f;
	const float icon_size = REWARD_ICON_SIZE * scale;

	str countString = "";
	qhandle_t handle = -1;
	float x = (SCREEN_W/2) - (icon_size/2) - ((m_rewards.size()-1)*(icon_size/2));

	for(CTF_RewardContainerIterator it = m_rewards.begin(); it != m_rewards.end(); ++it)
	{
		handle = it->second.GetIconHandle();

		if(handle <= 0)
			continue;

		countString = va("%d", it->second.GetCount());
		CTF::DrawPic(x, y, icon_size, icon_size, 0, 0, 1, 1, handle);
		CTF::DrawString(countString.c_str(), 
			x + icon_size/2 - (countString.length()*(FONT_WIDTH*scale/2.0f)), 
			y+icon_size, scale, true, 1.0f);

		x += icon_size;
	}

	return y + icon_size*1.5f + FONT_HEIGHT*scale;
}

//
//zero out all our rewards
//
void CTF_RewardSystem::Clear()
{
	for(CTF_RewardContainerIterator it = m_rewards.begin(); it != m_rewards.end(); ++it)
		it->second.SetCount(0, false);
}

}//~namespace CTF