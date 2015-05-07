/****************************************************************************************************
* Craft system.
* Description: ������� ���������� �� ����� �����. �������� � ���� ��� ���� � ��������� ��� ������
* Created by: Andrew Medvedev
* Date: 13.08.2013
* Diko Source File
****************************************************************************************************/

#ifndef _CRAFT_SYSTEM_
#define _CRAFT_SYSTEM_

#include "IFlashUI.h"
#include "IGameFramework.h"
#include "IActionMapManager.h"
#include "InventoryItems.h"


class CCraftSystem;
class CCraftEventListener : public IUIElementEventListener, public  IActionListener
{
public:
	CCraftEventListener();
	void OnUIEvent(IUIElement* pSender, const SUIEventDesc& event, const SUIArguments& args);
	void OnAction(const ActionId& action, int activationMode, float value);

	CCraftSystem *pCraft;
};

class CCraftSystem : public IGameFrameworkListener
{
public:
	CCraftSystem();

	//���������� ��� �������� ���� � ���������� �� ���� ������ �� ������ ��������
	void InitList();

	//����� ���������� ������ ���, ����� ����������� ���� ������ � ��������� ������ pUIItems
	bool ReciveInventoryItems();

	//����� ������ ��� ��� ������ �������. ��������� ��� ������� � ���������� �� ���� ������ ������������ ��� ������
	SCraftList* GetRecipeDescription(const char *name, int index = -1);

	//����� �������� ������ �������� � ������ ����������
	void CraftItem(SCraftList *pCraftList, const int index);

	//���������� ��� ������� �� �������
	char* GetRecipeByIndex(int index);

	//===== ����������/�������� �������� =====
	//����� ��������� � ������ ����� ������
	void AddToList(SCraftList *pCraftItem);

	//����� ������� ������ �� ������ �� �����
	void DeleteItem(const char* name);
	void DeleteItem(int index);

	SCraftList* GetCraftList();

	int GetRegCount(char* name);
	char* GetRegName(char* name);
	bool DeleteUsedItems(char* name);
	void SetInitialTime(float time){ m_fInitialTime = time; }
	void SetCloseEvent(bool event){ m_bCloseEvent = event; }

	// IGameFrameworkListener
	VIRTUAL void OnPostUpdate(float fDeltaTime);
	VIRTUAL void OnSaveGame(ISaveGame* pSaveGame);
	VIRTUAL void OnLoadGame(ILoadGame* pLoadGame);
	VIRTUAL void OnLevelEnd(const char* nextLevel);
	VIRTUAL void OnActionEvent(const SActionEvent& event);
	// ~IGameFrameworkListener
private:
	std::vector<SCraftList*> pCraftList; //������ ��������
	//SCraftList* pCraftList;
	CCraftEventListener Listener;
	float m_fInitialTime; //������ ������� ��� �������� ���� ������
	bool m_bCloseEvent; // ���� ���� ������ ������ �������� - true;
};



#endif

/*
����� � ������������ ����� ���������, ������ ���������� ��������������� �������, ����������� ������� ����� ������� �� �������
��� ��� ������ � �������. ���� ���������, �� ������ ���� ��������������� ������ �������� ��� � �� �����.
*/