//
//CTF Reward icons
//like defence, assist & capture
//
//-Yorvik
//

#ifndef CTF_REWARD_H_INCLUDED
#define CTF_REWARD_H_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif

#pragma warning(push, 1)
#include <map>
#pragma warning(pop)

#include "cg_local.h"
#include "str.h"

namespace CTF{

//
//single reward class
//
class CTF_Reward
{
	int			m_stopDrawTime;	//time to stop drawing this reward
	int			m_count;
	qhandle_t	m_iconHandle;
	str			m_iconName;
	str			m_soundName;	//sound to play when we get this reward

public:
	
	CTF_Reward();

	void SetIconName(str name);
	void SetSound(str snd);
	void SetCount(int count, bool icon = true);
	void AddCount(int n, bool icon = true);
	void CacheIcon();

	bool		DrawIcon();
	qhandle_t	GetIconHandle()	const	{return m_iconHandle;}
	int			GetCount()		const	{return m_count;}
	const char*	GetSound()		const	{return m_soundName.c_str();}
};

//
//All rewards are collected and managed by this class
//
class CTF_RewardSystem
{
	typedef std::map<int, CTF_Reward>		CTF_RewardContainer;
	typedef CTF_RewardContainer::iterator	CTF_RewardContainerIterator;

	CTF_RewardContainer m_rewards;
	CTF_RewardSystem();

	CTF_RewardContainerIterator m_lastReward; //last reward given

public:

	void Clear();
	void AddReward(int rewardType, int count = 1, bool sound = true);
	void DrawRewards();
	void CacheIcons();
	float DrawRewardsSB(float y);

	static CTF_RewardSystem& Instance()
	{
		static CTF_RewardSystem sys;
		return sys;
	}
};


//singleton
CTF_RewardSystem& GetRewardSystem();

} //~namespace CTF

#endif //CTF_REWARD_H_INCLUDED