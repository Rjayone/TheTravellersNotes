/****************************************

* File: DialogSystem.h/cpp

* Description: Описание классов и функций для создания системы диалогов

* Created by: Gomza Yakov

* Date: 20.08.2014

* Diko Source File

* ToDo: В стадии разработки

*****************************************/


#ifndef __UIDialogEvents__
#define __UIDialogEvents__

#include <IFlashUI.h>
#include "IActionMapManager.h"
#include "IGameFramework.h"


struct SAnswer
{
	string answer;
	string idNextDialog;
	string qcamback;
	string sec;
	string AudioName;
};

static char* sAIType;
static char* sAIName;
static bool  bQCam, StartCount, bStartPlayerCount;
static SAnswer *PlayersAnswers;
static char* FilePath;
static int nextDialogId;
static float fSec;
static IEntity *pEntityAI;


class CUIDialogMenuEventListener : public IUIElementEventListener
{
public:
	CUIDialogMenuEventListener(){}
	void OnUIEvent(IUIElement* pSender, const SUIEventDesc& event, const SUIArguments& args);	
};


class CUIDialogMenu : public IGameFrameworkListener, public IActionListener
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
	//~IGameFrameworkListener

	void OnAction(const ActionId& action, int activationMode, float value);
	

	void StartDialog(IEntity *pAIEntity, bool Show);
	void DisplayDialogMenu(bool bShow);
	void SetDialogFromXML(int NextDialogId = 1);
	void SetDialogCam(IEntity *pAIEntity, const char *CameraName);
	void SetAnswer(const char *text);
	void SetPlayerAnswer(string Answer, string Next, int count, string camback, string sec, string sAudioName);
	void OnTimer(float sec);
	void RotateCamFocus(IEntity *pEntity, const char *BoneName, const char *CameraName);
	void SetQuestCam(string camname);
	void SetAudio(const char *AudioName, bool bEnabled = false);

private:
	IUIElement *m_pUIDialogMenu;
	CUIDialogMenuEventListener m_UIDialogMenutEventListener;	
	int playerAnswerCount;
	bool  StartCount;
	bool bNextPhrase = false;
	ITexture *texture;
	
	
};

#endif