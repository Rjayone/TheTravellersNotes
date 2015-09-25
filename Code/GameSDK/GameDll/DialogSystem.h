/****************************************

* File: DialogSystem.h/cpp

* Description: �������� ������� � ������� ��� �������� ������� ��������

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

//��������� ��������� ���� �� ��������� ������� ������ � �������
struct SDialogPlayerAnswer
{
	string answer;			//����� ������
	int nextDialogId;		//�� ��������� ������� ���, ���� ����� �������� ������ ������� ������
	int audioDialogDuration;  //������ ���������� � �������
	string audioName;			//�����-���� ���������� �������
	int parentId;		//�� ������������ �����
	SDialogPlayerAnswer() : nextDialogId(0), audioDialogDuration(0)
	{
	}
};

//��������� ��������� �������� ������� ���
struct SDialogNPCAnswer 
{
	string answer;		//�������� ����� ������(�����)
	int currentPhraseId; //�� �������� ������
	int nextPhraseId;	//�� ��������� �����. ���� ����������� ����������� ������� �������
	float audioDialogDuration;  //������ ���������� � �������
	string audioName;			//�����-���� ���������� �������
	string questCamera;			//��� ������ ��� ������� ������ ���
	bool withoutMessage; //�����������, ���� ������ ����� ������ ����� � ������� ������. ��� ���� ��������� ���� ��� ���������������
	const char* specialEvent; //�����, �������� � ��


	//---------------------------------------------------------------
	struct SDialogActionQuest
	{
		const char* questTitle;
		int status;
		SDialogActionQuest() : questTitle(""), status(0) // 1 - �������, 2 - ����, 3 - ��������
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

	//��������:
	//����� ����������, ����� ������������ ������� �� �������� ������ � ������
	void OnPlayerExitFromEditorGameMode();
	//~

	//��������:
	//������ ������� ���������� �� CDialogDispatcher. ����� ��������, ��� ����� ����� ������ ������ � pTarget
	//���������:
	//pTarget - ��������(��), � ������� ����� ����� �������� ������
	void CanDialog(IEntity* pTarget);

	//��������:
	//����� ��������, ��� ����� ����� ������. ������������ ��� ����������� ���������� � ������ �������
	//���������:
	//pAIEntity - ��� ��������, � ������� �������������
	void WillStartDialog(IEntity *pAIEntity);

	//��������:
	//�������� ������
	void StartDialog();

	void ContinueDialog(int nextDialogId);
	//��������:
	//����� ��������� ������ � �������� ���� �������
	void StopDialog();

	//��������:
	//����� ���������� �������� ���� �� � ������ ������ ������
	bool IsDialogStarted() { return m_bDialogStarted; }

	//��������:
	//����� ������������ XML-����, � ��������� �� ��� ������ ������
	void SetDialogFromXML(int NextDialogId, const char* filePath = NULL);
	void ParseXML();

	//��������:
	//����� ������������� �����(�����), ������� ������� ��������
	void SetAnswer(const char *text, int id);

	//��������:
	//����� ��������� ���������, �� ������� ����������� �������� ������� ������
	//� ����������� ������� ������ ����� ���������� � �����
	void SetPlayerAnswer(SDialogPlayerAnswer* pPlayerAnswer);

	//��������:
	//����� ���������� ���� �����
	void Skip(int next);

	//��������:
	//����� �� �������
	void Exit();

	void SetDialogCam(IEntity *pAIEntity, const char *CameraName);
	void RotateCamFocus(IEntity *pEntity, const char *BoneName, const char *CameraName);
	void SetQuestCam(string camname);
	void SetAudio(const char *AudioName, bool bEnabled = false);

	//��������:
	//����� ��������/��������� ������� �������� ����
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

	bool m_bDialogStarted;		//��������� ���� �� ������ � ������ ������

	bool m_bQuestCameraIsEnabled;
	const char* m_pFilePath;

	//Listeners
	std::vector<IDialogEvents*> m_pListeners;
};