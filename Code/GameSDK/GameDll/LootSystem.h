/****************************************

* File: DialogSystem.h/cpp

* Description: Система лута

* Created by: Gomza Yakov

* Date: 02.06.2015

* Diko Source File

* ToDo: В стадии разработки

*****************************************/


#ifndef __LootSystem__
#define __LootSystem__

#include <IFlashUI.h>
#include "IActionMapManager.h"
#include "IGameFramework.h"
#include "RPGInventory.h"
#include "GameActions.h"


class CLootOptions
{
public:
	int ItemId;
	int ItemCount;
};

class CItemOptions
{
public:
	char const* ItemClass;
	char const* ItemName;
	char const* ItemDescription;
	float ItemCost;
	EInventoryItemType ItemType;
	int ItemSize;
	bool isLooted;
};

class CLootSystem : public IActionListener, public IGameFrameworkListener
{
public: 
	CLootSystem();
	~CLootSystem();


	//IGameFrameworkListener
	void OnPostUpdate(float fDeltaTime){}
	void OnSaveGame(ISaveGame* pSaveGame){}
	void OnLoadGame(ILoadGame* pLoadGame){}
	void OnLevelEnd(const char* nextLevel){}
	void OnActionEvent(const SActionEvent& event){}
	//~IGameFrameworkListener

	void OnAction(const ActionId& action, int activationMode, float value);

	void InitLootSystem(CActor * pActor);
private:

	IEntity* pNpcEntity;
	float lootDistance = 3;
	CActor* pNPCActor;

	bool CheckForLoot(CActor* pActor);
	void StartLoot();
	int GetLoodId(IEntity* pNpcEntity);
	void SetItemsOptionsFromXml(int LootId);
	bool GetItemsForLoot();

	std::vector<CLootOptions> LootOptions;
	std::vector<CItemOptions> ItemsList;

	CRPGInventory RPGInventory;
};

#endif