#pragma once

#include "PlayerStats.h"


class CStatStamina : public CPlayerStat, public IGameFrameworkListener
{
public:
	CStatStamina();
	~CStatStamina();

	void OnRun();
	void OnJump();

	void OnPostUpdate(float fDeltaTime);
	void OnSaveGame(ISaveGame* pSaveGame){}
	void OnLoadGame(ILoadGame* pLoadGame){}
	void OnLevelEnd(const char* nextLevel){}
	void OnActionEvent(const SActionEvent& event){}
};