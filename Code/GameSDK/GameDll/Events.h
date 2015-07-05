/*********************************************************************************************************
- File: Events.h
- Description: Класс с различными игровыми событиями
- Created by: Andrew Medvedev
- Date: 26:06:2015
- Edda Source File
********************************************************************************************************/

#pragma once

#include "IGameFramework.h"

//Интерфейс, в котором содержаться некоторые эвенты для упрощения работы
//Вы так же можете добавлять сюда те методы, которые хотите в дальенйшем привязать к событиям
//Для подписки на эти события:
// 1. Унаследуйте вашим классом IEvents
// 2. В конструкторе вашего класса напишите строчку g_pGame->GetEventsDispatcher()->AddListener(this);
// 3. Реализуйте необходимые методы
// 4. В деструкторе напишите g_pGame->GetEventDispatcher()->RemoveListener(this);
class IEvents 
{
public:
	//Описание:
	//Метод вызывается, когда пользователь переходит в игровой режим в движке
	virtual void OnPlayerEnterInEditoriGameMode(){}

	//Описание:
	//Метод вызывается, когда пользователь выходит из игрового режима в движке
	virtual void OnPlayerExitFromEditorGameMode(){}
};

class CEventsDispatcher : public IGameFrameworkListener
{
public:
	CEventsDispatcher();
	~CEventsDispatcher();

	//IGameFrameworkListener
	void OnPostUpdate(float fDeltaTime);
	void OnSaveGame(ISaveGame* pSaveGame){}
	void OnLoadGame(ILoadGame* pLoadGame){}
	void OnLevelEnd(const char* nextLevel){}
	void OnActionEvent(const SActionEvent& event){}
	//~

	//Описание:
	//Метод добавляет нового подписчика на событие.
	//Добавляемый класс, разумеется, должен наследовать IEvents
	//Если в массиве уже содержится данный подписчик, процедура игнорируется
	void AddListener(IEvents* pListener);

	//Описание:
	//Метод удаляет определенного подписчика
	void RemoveListener(IEvents* pListener);

	////Events
	//Описание:
	//Метод вызывается, когда пользователь переходит в игровой режим в движке
	void OnPlayerEnterInEditoriGameMode();

	//Описание:
	//Метод вызывается, когда пользователь выходит из игрового режима в движке
	void OnPlayerExitFromEditorGameMode();
private:
	std::vector<IEvents*> m_pListeners; //Массив слушателей
};