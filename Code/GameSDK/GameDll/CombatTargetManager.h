#pragma once

#include "Game.h"

class CCombatTarget : public IGameFrameworkListener
{
public:
	CCombatTarget();
	~CCombatTarget();

	void ForceLookAt(EntityId id);
	void LookAtClosestTarget();

	EntityId GetClosestTarget();
	void GetScreenTargets();

	void CreateTagPointHelper();

	//IGameFrameworkListener
	void OnPostUpdate(float fDeltaTime);
	void OnSaveGame(ISaveGame* pSaveGame){}
	void OnLoadGame(ILoadGame* pLoadGame){}
	void OnLevelEnd(const char* nextLevel){}
	void OnActionEvent(const SActionEvent& event){}
	//~
private:
	std::vector<EntityId> m_availableTarget;
	EntityId currentTarget;
	IEntity* m_pHelper;
};