/****************************************

* File: RPGInventoryManager.h/cpp

* Description: Менеджер игровых объектов для инвентаря. Добавление, удаление, поиск и тд.

* Created by: Andrew Medvedev

* Date: 18.03.2014

* Diko Source File

* ToDo:

*****************************************/

#ifndef _RPGINVENTORYMANAGER_
#define _RPGINVENTORYMANAGER_

#include "IGameFramework.h"
#include "GameActions.h"
#include "BasicObject.h"

class CRPGInventoryManager : public IGameFrameworkListener, public IActionListener
{
public:
	CRPGInventoryManager();

	void OnPostUpdate(float fDeltaTime);
	void OnSaveGame(ISaveGame* pSaveGame);
	void OnLoadGame(ILoadGame* pLoadGame);
	void OnLevelEnd(const char* nextLevel);
	void OnActionEvent(const SActionEvent& event);
	void OnAction(const ActionId& action, int activationMode, float value);

	void AddObjects(IBasicObject *pBasicObject);
	void DeleteObject(EntityId id);
	void DeleteObject(char* sObjectName);
	bool Clear();

	IBasicObject* GetBasicObject(EntityId id);
	int GetObjectIndex(EntityId id);
	int GetObjectIndex(char* name);
private:
	std::vector<IBasicObject*> m_pBasicObjects;
	//IBasicObject* m_pBasicObjects;
	int m_nSize;
};

#endif