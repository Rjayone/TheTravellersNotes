/****************************************

* File: DialogSystem.h/cpp

* Description: Описание классов и функций для создания системы диалогов

* Created by: Gomza Yakov

* Date: 20.08.2014

* Diko Source File

* ToDo: В стадии разработки

*****************************************/


#pragma once

#include "IGameFramework.h"
#include "DialogDispatcher.h"
#include "IFlashUI.h"
#include "Timer.h"

//Структура описывает один из вариантов ответов игрока в диалоге
struct SDialogPlayerAnswer
{
	string answer;			//Текст ответа
	int nextDialogId;		//ид следующей реплики нпс, если игрок нажимает данный вариант овтета
	int audioDialogDuration;  //Длинна аудиофайла в милисек
	string audioName;			//Аудио-файл выбранного диалога
};

//Структура описывает варианты ответов НПС
struct SDialogNPCAnswer 
{
	string answer;		//Основной текст ответа(абзац)
	int currentPhraseId; //Ид текущего ответа
	int nextPhraseId;	//Ид следующей фразы. Дает возможность выстраивать цепочки далогов
	float audioDialogDuration;  //Длинна аудиофайла в милисек
	string audioName;			//Аудио-файл выбранного диалога
	string questCamera;			//Имя камеры для данного ответа нпс
};


class CUIDialogMenu : public IGameFrameworkListener, public IUIElementEventListener, public ITimerEvents
{
public:
	CUIDialogMenu();
	~CUIDialogMenu();
	IUIElement *GetUIElement();

	//IGameFrameworkListener
	void OnPostUpdate(float fDeltaTime);
	void OnSaveGame(ISaveGame* pSaveGame){}
	void OnLoadGame(ILoadGame* pLoadGame){}
	void OnLevelEnd(const char* nextLevel){}
	void OnActionEvent(const SActionEvent& event){}
	//~

	//ITimerEvents
	void OnTimerStarted(int id){}
	void OnTick(int id){}
	void OnTimerFinished(int id);
	//~

	//IUIElementEventListener
	void OnUIEvent(IUIElement* pSender, const SUIEventDesc& event, const SUIArguments& args);
	//~

	//IActionListener
	void OnAction(const ActionId& action, int activationMode, float value);
	//~


	//Описание:
	//Данная функция вызывается из CDialogDispatcher. Вызов означает, что игрок может начать диалог с pTarget
	//Принимает:
	//pTarget - сущность(АИ), с которой игрок может провести диалог
	void CanDialog(IEntity* pTarget);

	//Описание:
	//Метод начинает отображает диалоговое окно и начинает с 1-ой фразы диалога
	//Принимает:
	//pAIEntity - тот персонаж, с которым разговариваем
	void StartDialog(IEntity *pAIEntity);

	//Описание:
	//Метод завершает диалог и скрывает окно диалога
	void StopDialog();

	//Описание:
	//Метод обрабатывает XML-файл, и формирует на его основе диалог
	void SetDialogFromXML(int NextDialogId, const char* filePath);

	//Описание:
	//Метод устанавливает фразу(абзац), который говорит персонаж
	void SetAnswer(const char *text);

	//Описание:
	//Метод принимает стркутуру, из которой формируются варианты ответов игрока
	//В большинстве случаев данный метод вызывается в цикле
	void SetPlayerAnswer(SDialogPlayerAnswer* pPlayerAnswer, int index);

	//Описание:
	//Метод пропускает один абзац
	void Skip();

	void SetDialogCam(IEntity *pAIEntity, const char *CameraName);
	void RotateCamFocus(IEntity *pEntity, const char *BoneName, const char *CameraName);
	void SetQuestCam(string camname);
	void SetAudio(const char *AudioName, bool bEnabled = false);


	////Управление интерфейсом
	//Описание:
	//Метод воспроизводит анимацию точки(что в центер экрана), которая обозначит возможность диалога
	void ShowDotDialogIcon();

	//Описание:
	//Метод отображает на экран окно диалога в зависимости от значения bShow
	void DisplayDialogMenu(bool bShow = true);
	//~

private:
	CTimer* m_pDialogTimer;
	CDialogDispatcher* m_pDialogDispathcer;
	IUIElement *m_pUIDialogMenu;
	IEntity* m_pTarget;
	IEntity* m_pCurrentQuestCamera;

	SDialogNPCAnswer* m_pNPCAnswer;
	SDialogPlayerAnswer* m_pPlayerAnswer;

	bool m_bQuestCameraIsEnabled;
	const char* m_pFilePath;
};
