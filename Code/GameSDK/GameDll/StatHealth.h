#ifndef _STAT_HEALTH_
#define _STAT_HEALTH_

#include "PlayerStats.h"

class CStatHealth : public CPlayerStat, public IGameFrameworkListener
{
public:
	CStatHealth();
	~CStatHealth();

	void OnPostUpdate(float fDeltaTime);
	void OnSaveGame(ISaveGame* pSaveGame){}
	void OnLoadGame(ILoadGame* pLoadGame){}
	void OnLevelEnd(const char* nextLevel){}
	void OnActionEvent(const SActionEvent& event);
};

#endif