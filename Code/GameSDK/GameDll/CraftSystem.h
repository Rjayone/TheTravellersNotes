/****************************************************************************************************
* Craft system.
* Description: Система отвечающая за крафт вещей. Включает в себя хмл файл с описанием для крафта
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

	//Вызывается при открытии окна и отправляет во флеш данные из списка рецептов
	void InitList();

	//Метод вызывается каждый раз, когда открывается окно крафта и заполняет объект pUIItems
	bool ReciveInventoryItems();

	//Вызов каждый раз при выборе рецепта. Принимает имя рецепта и отправляет во флеш список необходимого для крафта
	SCraftList* GetRecipeDescription(const char *name, int index = -1);

	//Метод получает список рецептов и индекс выбранного
	void CraftItem(SCraftList *pCraftList, const int index);

	//Вовзращает имя рецепта по индексу
	char* GetRecipeByIndex(int index);

	//===== Добавление/Удаление рецептов =====
	//Метод добавляет в список новый рецепт
	void AddToList(SCraftList *pCraftItem);

	//Метод удаляет рецепт из списка по имени
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
	std::vector<SCraftList*> pCraftList; //Список рецептов
	//SCraftList* pCraftList;
	CCraftEventListener Listener;
	float m_fInitialTime; //Момент времени для закрытия окна крафта
	bool m_bCloseEvent; // Если была нажата кнопка закрытия - true;
};



#endif

/*
Когда я переключаюсь между рецептами, должна вызываться соответствующая функция, аргументами которой будет ялвятся ид рецепта
или его индекс в массиве. Если последнее, то должен быть синхронизирован список рецептов тут и во флеше.
*/