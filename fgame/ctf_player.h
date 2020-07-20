///////////////////////////////////////////////////////////////////////////////
//
// This file is included inside the class header of 'Player', so we don't want 
// any included guards, and we don't want to create any classes at global scope.
// doing it this way means we put as few changes as possible in existing files.
//
//
///////////////////////////////////////////////////////////////////////////////
//
// class Player ...
// {
//  

public:

	//typedefs
	typedef std::vector<CTF_Tech*>		TECH_LIST;
	typedef TECH_LIST::iterator			TECH_LIST_ITERATOR;
	typedef TECH_LIST::const_iterator	TECH_LIST_CONSTITERATOR;

	typedef std::vector< std::pair<int, float> >	REWARD_LIST; //count : time of last reward (type of reward is calculated: it-begin)
	typedef REWARD_LIST::iterator					REWARD_LIST_ITERATOR;
	typedef REWARD_LIST::const_iterator				REWARD_LIST_CONSTITERATOR;



	void SetFlag(CTF_Flag* pFlag)
	{
		m_pFlag = pFlag;
	}

	CTF_Flag* GetFlag()
	{
		return m_pFlag;
	}

	// Change the team of the player
	void ChangeTeam(Event* ev);

	// Drop the flag
	void DropFlag();

	//start a level
	void CTF_ClientBegin();
	void CTF_Init();

	//disconnect
	void CTF_Disconnect();

	//remove floating crosshair
	void CTF_RemoveCrosshair();

	//intermission
	bool CTF_IntermissionReady() const {return m_intermissionReady;}

	//shield blockable
	bool CTF_InShieldRange(Vector&);

	//obituary
	void CTF_Obituary(Entity* attacker, Entity* inflictor, int meansofdeath);

	//
	//TECH bits
	//
	const TECH_LIST& GetTechList() const {return m_techs;}

	bool		GiveTech(CTF_Tech* tech);
	CTF_Tech*	GetTech(int id) const;
	void		DropTechs();
	void		RemoveTech(int id);

	//these return cumulative effects for various tech powers due to all techs we have
	float		TechDamage(bool effect = false,		Vector pos = Vector(0, 0, 0));
	float		TechProtection(bool effect = false, Vector pos = Vector(0, 0, 0));
	float		TechEmpathy(bool effect = false,	Vector pos = Vector(0, 0, 0));
	float		TechAcro(bool effect = false,		Vector pos = Vector(0, 0, 0));

	//
	//Rewards
	//
	const REWARD_LIST& GetRewardList() const {return m_rewards;}
	void AddReward(int rewardType, int count = 1);

	//stats
	void CTF_Reset();
	int	 GetCaps()			const	{return num_captures;}
	int  GetScore()			const	{return num_kills;}
	int	 GetDeaths()		const	{return num_deaths;}
	int	 GetSuicides()		const	{return num_suicides;}

	int	 LastKillerNum()	const	{return m_lastKiller ? m_lastKiller->entnum : -1;}
	void AddScore(int score);
	void AddWater(float amount);
	void AddCaps(int caps);

	//sectator following bits
	void FollowNext(teamtype_t team = TEAM_NONE);
	void Follow(Player* player);
	bool IsFollowing(Player* player);
	void StopFollowing();

	//
	//ctf replacements for common player functions
	//
	virtual void CTF_Killed			(Event* ev);
	virtual void CTF_Kill			(Event* ev);
	virtual void CTF_DamageEvent	(Event* ev);
	virtual void CTF_Respawn		(Event* ev);
	virtual void CTF_SpawnBloodyGibs(Event* ev);
	virtual void CTF_GotKill		(Event* ev);
	virtual void CTF_DeadStopAnim	(Event* ev);

	// Team time
		void SetTeamJoinTime(float time) {m_teamJoinTime = time;}
		float GetTeamJoinTime() const {return m_teamJoinTime;}

private:
	// Update status for CTF game
	void CTF_UpdateStatus(); 

	void CTF_InitSpawnPoint();
	bool CTF_Gib();
	void CTF_ClientThink();

	void CTF_DoRewards(Player* attacker);

/////////////////////////////////////////////////////////////////////////////
// Data
	CTF_Flag*	m_pFlag;			// The flag (if any) that this player is carying

	int			num_captures;		//how many caps we have
	int			num_suicides;		//how many times we suicided
	Entity*		m_lastKiller;		//pointer to the client that killed us last
	float		m_lastKillTime;		//time we last got a kill
	int			m_killStreak;		//how many kills we have in quick sucession
	float		m_lastLocationCheck;//time of last location update

	bool		m_intermissionReady;//are we ready to switch to next map

	int			m_followEnt;

	TECH_LIST	m_techs;
	str			m_techInfoString;

	REWARD_LIST	m_rewards;

	bool		m_gibbed;

	float		m_teamJoinTime;		// The we joined our current team
//
// blah....
//
// };