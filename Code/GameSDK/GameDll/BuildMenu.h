/*********************************************************************************************************
- File: BuildMenu.h
- Description: Основное флеш меню режима стройки с реализацией функций стройки и связи Флэш и Края
- Created by: Гомза Яков
- Date: 25.03.2014
- Diko Source File
- ToDo:Возможно дополнение экономической части
********************************************************************************************************/

#ifndef __UIBuildMenu__
#define __UIBuildMenu__

#include <IFlashUI.h>
#include "IActionMapManager.h"
#include "IGameFramework.h"
#include "Camera.h"
#include "BuildSelectionHelper.h"


//Класс предназначен для слушанья событий для юи
class CUIBuildMenuEventListener : public IUIElementEventListener
{
public:
	CUIBuildMenuEventListener();

	//Описание:
	//Функция срабатывает при различных UI-евентах
	void OnUIEvent(IUIElement* pSender, const SUIEventDesc& Event, const SUIArguments& args);
};


//Данный класс представляет собой механизм работы с меню постройки:
//Отображение, заполнения списка зданий, подверждение покупки а так же само строительство.
class CUIBuildMenu : public IGameFrameworkListener, public IActionListener
{
public:
	CUIBuildMenu();
	~CUIBuildMenu();

	//Описание:
	//В данном методе происходит инициализация и получение указателя на флеш-меню.
	//Возвращает: true, если все окей, иначе false.
	bool Init();

	//Описание:
	//Функция скрывает/отображет текущий UI-елемент(m_pBuildMenu), а так же выполняет его регистрация для слушания событий
	//Принимает: true - отобразить, flase - скрыть
	void DisplayBuildMenu(bool bShow);


	//Функции флеш-вызовов ниже
	//Описание:
	//Функция отправляет во флеш данные для инициализации списков объектов, которые можно построить
	//Для этого достаточно заполнить структуру SBuilding и отправить её в момент отображения и-фейса.
	//Принимает: Указатель на структуру, описывающие конкретное здание.
	//Примеченая: Все параметры зданий описаны в XML-файле.
	void SendBuildingsInfo(SBuilding* build);

	//Описание:
	//Функция чистит список зданий во флеш-меню.
	void Clear();

	//Описание:
	//Функция предназначена для вывода меню подтверждения покупки здания
	void ShowAcceptMenu(bool bAccept);

	//Описание:
	//Отображение бокового меню, на котором отображен список доступных зданий для текущего места строительства.
	//Принимает: true - показать, false - скрыть
	void ShowSlideMenu(bool bShow = false);

	//Описание:
	//Функция была создана для скрытия меню через малый промежуток времени(0.01 сек)
	//Это было сделано из-за того, что вылетал эксепшен, если закрыть меню на кнопку.
	void Delay();
	//~

	//Функции работы с камерой:
	//Переход к следующей точке постройки, переход к предыдущей точке.
	//Описание:
	//Функция выбирает следующую камеру, где возможна постройка здания.
	void NextCamView();
	void PriveousCamView();
	//~


	//IGameFrameworkListener
	//Описание:
	//В данном момент используется для работы с таймером.
	void OnPostUpdate(float fDeltaTime);
	void OnSaveGame(ISaveGame* pSaveGame){}
	void OnLoadGame(ILoadGame* pLoadGame){}
	void OnLevelEnd(const char* nextLevel){}
	void OnActionEvent(const SActionEvent& event){}
	//~


	//Функции ддя работы с выбором сущности на сцене
	//Описание:
	//Функция слушает нажатия кнопок
	//Принимает: ссылку на структуру ActionId, в которой указан экшен, который был вызван
	//Все экшены описаны в defaultProfile.xml, объявлены в GameActions.action,
	//activationmode: 1 - onMouseDown, 2 - onMouseUp, 4 - Hold.
	void OnAction(const ActionId& action, int activationMode, float value);
	void SetSelectedEntity(IEntity *pEnt){ m_pSelectedEntity = pEnt; }
	IEntity* GetSelectedEntity(){ return m_pSelectedEntity; }
	CSelectionBuildHelper* GetSelectionHelper(){ return m_Helper; }
	//~

	bool IsBuildModeEnable() { return m_bBuildModeEnable; }
	void SetBuildMode(bool seter){ m_bBuildModeEnable = seter; }

	friend class CSelectionBuildHelper;
	friend class CUIBuildMenuEventListener;

	std::vector <SBuilding*> GetPerformedBuildings(){
		return m_PerformedBuildings;
	}
private:
	CUIBuildMenuEventListener g_CUIBuildSystemEventListener; //объект слушателя.
	CSelectionBuildHelper* m_Helper; //Хэлпер, который выбран
	CBuildCamera *m_BuildCamera; //Текущая камера постройки
	IUIElement *m_pBuildMenu; //Указатель на флеш-меню постройки
	IEntity* m_pSelectedEntity; //Хэлпер, приведенный к IEntity

	float m_fInitialTime; //Момент времени для закрытия окна
	bool m_bCloseEvent; // Если была нажата кнопка закрытия - true;
	bool m_bBuildModeEnable; //true - если режим включен

	std::vector <SBuilding*> m_PerformedBuildings; // построенные здания
};

#endif