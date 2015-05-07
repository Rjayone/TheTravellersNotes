/****************************************

* File: RPGInventory.h/cpp

* Description: Описание системы инвентаря(визуальная часть)

* Created by: Andrew Medvedev

* Date: 08.02.2014

* Update: 29.03.2014

* Diko Source File

* ToDo:

*****************************************/

#ifndef _UIINVENTORY_H_
#define _UIINVENTORY_H_

#include "IGameFramework.h"
#include "IFlashUI.h"
#include "InventoryItems.h"

class CRPGInventory;
class CInventoryEventListener : public IUIElementEventListener, public  IActionListener
{
public:
	CInventoryEventListener();

	void OnUIEvent(IUIElement* pSender, const SUIEventDesc& event, const SUIArguments& args);
	void UpdateSlots(){}
	int* GetItemArgs(){ return itemArgs; }
	void OnAction(const ActionId& action, int activationMode, float value);

	int itemArgs[3];
	CRPGInventory *pInventory;
};

//----------------------------------------------------------
class CRPGInventory : public	IGameFrameworkListener
{
public:
	CRPGInventory();

	SInventoryItem* GetItemParamsXML(const char *params);
	void OnPickedUp(EntityId itemId);
	void OnDropItem(EntityId itemId, float impulseScale);
	void OnUse(EntityId id); // Если предмет использован в инвентаре

	bool DeleteItem(const char* name);
	bool DeleteItem(EntityId itemId);
	void AddItem(SInventoryItem *pItem);

	SInventoryItem* GetItemsArray();
	void ProcessEvent(SEntityEvent &event);
	SInventoryItem* GetInventoryItemById(EntityId id);
	SInventoryItem* GetInventoryItemByName(const char* name);
	CInventoryEventListener GetInventoryEventListener(){ return m_eventListener; }


	int GetItemIndex(EntityId itemId);
	int GetItemIndex(const char* name);
	IUIElement* GetInventoryUIInterface();
	int GetMaxSlotsCount(){ return m_SlotsCount; }

	void ResetUIItemsArray();

	// IGameFrameworkListener
	void OnPostUpdate(float fDeltaTime);
	void OnSaveGame(ISaveGame* pSaveGame);
	void OnLoadGame(ILoadGame* pLoadGame);
	void OnLevelEnd(const char* nextLevel);
	void OnActionEvent(const SActionEvent& event);
	// ~IGameFrameworkListener

	
	//SInventoryItem *m_pItemsArray; // Массив предметов
	std::vector<SInventoryItem*> m_pItemsArray;
private:
	CInventoryEventListener m_eventListener;
	IUIElement *m_pUIInventory;
	int m_SlotsCount; // Количество слотов инвентаря основной сумки	
	bool m_bGameStarted; // true если в игре, да бы исключить спаун левых итемов
};

#endif