/****************************************

* File: DialogSystem.h/cpp

* Description: Описание классов и функций для создания системы диалогов

* Created by: Gomza Yakov

* Date: 20.08.2014

* Diko Source File

*****************************************/


#pragma once

#include "IGameFramework.h"
#include "DialogDispatcher.h"
#include "IFlashUI.h"
#include "Timer.h"
#include "Events.h"
#include "Dot.h"
#include "DialogEvents.h"

class CUIVisibleManager;

//Структура описывает один из вариантов ответов игрока в диалоге
struct SDialogPlayerAnswer
{
	string answer;			//Текст ответа
	int nextDialogId;		//ид следующей реплики нпс, если игрок нажимает данный вариант овтета
	int audioDialogDuration;  //Длинна аудиофайла в милисек
	string audioName;			//Аудио-файл выбранного диалога
	int parentId;		//ид родительской фразы
	SDialogPlayerAnswer() : nextDialogId(0), audioDialogDuration(0)
	{
	}
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
	bool withoutMessage; //Испоьзуется, если диалог нужно начать сразу с ответов игрока. При этом начальная речь нпс воспроизводится
	const char* specialEvent; //Выход, торговля и тд


	//---------------------------------------------------------------
	struct SDialogActionQuest
	{
		const char* questTitle;
		int status;
		SDialogActionQuest() : questTitle(""), status(0) // 1 - получен, 2 - сдан, 3 - провален
		{}
	};
	//~

	std::vector<SDialogActionQuest> questActions;


	SDialogNPCAnswer() : currentPhraseId(-1), nextPhraseId(-1), audioDialogDuration(0.0f), withoutMessage(false), specialEvent("")
	{
	}
};


class CUIDialogMenu : public IGameFrameworkListener,
	public IUIElementEventListener,
	public ITimerEvents,
	public IEvents,
	public CUIDot
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

	//IEvents
	void OnPlayerEnterInEditoriGameMode();

	//Описание:
	//Метод вызывается, когда пользователь выходит из игрового режима в движке
	void OnPlayerExitFromEditorGameMode();
	//~

	//Описание:
	//Данная функция вызывается из CDialogDispatcher. Вызов означает, что игрок может начать диалог с pTarget
	//Принимает:
	//pTarget - сущность(АИ), с которой игрок может провести диалог
	void CanDialog(IEntity* pTarget);

	//Описание:
	//Метод означает, что будет начат диалог. Используется для выставления параметров и прочех мелочей
	//Принимает:
	//pAIEntity - тот персонаж, с которым разговариваем
	void WillStartDialog(IEntity *pAIEntity);

	//Описание:
	//Начинает диалог
	void StartDialog();

	void ContinueDialog(int nextDialogId);
	//Описание:
	//Метод завершает диалог и скрывает окно диалога
	void StopDialog();

	//Описание:
	//Метод возвращает значение идет ли в данный момент диалог
	bool IsDialogStarted() { return m_bDialogStarted; }

	//Описание:
	//Метод обрабатывает XML-файл, и формирует на его основе диалог
	void SetDialogFromXML(int NextDialogId, const char* filePath = NULL);
	void ParseXML();

	//Описание:
	//Метод устанавливает фразу(абзац), который говорит персонаж
	void SetAnswer(const char *text, int id);

	//Описание:
	//Метод принимает стркутуру, из которой формируются варианты ответов игрока
	//В большинстве случаев данный метод вызывается в цикле
	void SetPlayerAnswer(SDialogPlayerAnswer* pPlayerAnswer);

	//Описание:
	//Метод пропускает один абзац
	void Skip(int next);

	//Описание:
	//Выход из диалога
	void Exit();

	void SetDialogCam(IEntity *pAIEntity, const char *CameraName);
	void RotateCamFocus(IEntity *pEntity, const char *BoneName, const char *CameraName);
	void SetQuestCam(string camname);
	void SetAudio(const char *AudioName, bool bEnabled = false);

	//Описание:
	//Метод включает/выключает глубину резкости фона
	void SetRenderStatus(bool bOn);

	void AddEventListener(IDialogEvents* pListener);
	void RemoveEventListener(IDialogEvents* pListener);

private:
	CTimer* m_pDialogTimer;
	CDialogDispatcher* m_pDialogDispathcer;

	IUIElement *m_pUIDialogMenu;
	CUIVisibleManager* m_pVisibleManager;
	IEntity* m_pTarget;
	IEntity* m_pCurrentQuestCamera;
	IEntity* m_pAudioTrigger;

	std::vector<SDialogNPCAnswer*> m_pNPCAnswer;
	std::vector<SDialogPlayerAnswer*> m_pPlayerAnswer;
	int m_nLastParentDialogId;

	bool m_bDialogStarted;		//Указывает идет ли диалог в данный момент

	bool m_bQuestCameraIsEnabled;
	const char* m_pFilePath;

	//Listeners
	std::vector<IDialogEvents*> m_pListeners;
};