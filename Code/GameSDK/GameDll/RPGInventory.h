/****************************************

* File: RPGInventory.h/cpp

* Description: Описание системы инвентаря(визуальная часть)

* Created by: Andrew Medvedev

* Date: 08.02.2014

* Update: 29.03.2014

* Edda Source File

* ToDo:

*****************************************/

#pragma once

#include "IGameFramework.h"
#include "IFlashUI.h"

struct SInventoryItem;

//----------------------------------------------------------
class CRPGInventory : public IGameFrameworkListener, public IUIElementEventListener, public  IActionListener
{
public:
	CRPGInventory();
	~CRPGInventory();
	IUIElement* GetUIElement();

	// IGameFrameworkListener
	void OnPostUpdate(float fDeltaTime);
	void OnSaveGame(ISaveGame* pSaveGame);
	void OnLoadGame(ILoadGame* pLoadGame);
	void OnLevelEnd(const char* nextLevel);
	void OnActionEvent(const SActionEvent& event);
	//~

	//IUIElementEventListener
	void OnUIEvent(IUIElement* pSender, const SUIEventDesc& event, const SUIArguments& args);
	//~

	//IActionListener
	void OnAction(const ActionId& action, int activationMode, float value);
	//~


	SInventoryItem* GetItemParamsXML(const char *params);
	void OnPickedUp(EntityId itemId);
	void OnDropItem(EntityId itemId, float impulseScale);
	void OnUse(EntityId id); // Если предмет использован в инвентаре

	bool DeleteItem(const char* name);
	bool DeleteItem(EntityId itemId);
	void AddItem(SInventoryItem *pItem, int count=1);

	SInventoryItem* GetInventoryItemById(EntityId id);
	SInventoryItem* GetInventoryItemByName(const char* name);

	int GetItemIndex(EntityId itemId);
	int GetItemIndex(const char* name);
	IUIElement* GetInventoryUIInterface();
	int GetMaxSlotsCount(){ return m_SlotsCount; }

	void ResetUIItemsArray();

	std::vector<SInventoryItem*> m_pItemsArray;

	void SetInventoryStatus(bool isDroped);
	void ShowInventory(IUIElement* pUIInventory);
private:	
	IUIElement *m_pUIInventory;
	int m_SlotsCount; // Количество слотов инвентаря основной сумки	
	bool m_bGameStarted; // true если в игре, да бы исключить спаун левых итемов

	bool m_bInventoryDroped; //Снят ли рюкзак(инвентарь)
	void HideInventory(); //Скрытие инвентаря
	IEntity* m_pBackpack;
};
