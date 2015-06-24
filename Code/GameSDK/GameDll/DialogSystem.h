/****************************************

* File: DialogSystem.h/cpp

* Description: �������� ������� � ������� ��� �������� ������� ��������

* Created by: Gomza Yakov

* Date: 20.08.2014

* Diko Source File

* ToDo: � ������ ����������

*****************************************/


#pragma once

#include "IGameFramework.h"
#include "DialogDispatcher.h"
#include "IFlashUI.h"
#include "Timer.h"

//��������� ��������� ���� �� ��������� ������� ������ � �������
struct SDialogPlayerAnswer
{
	string answer;			//����� ������
	int nextDialogId;		//�� ��������� ������� ���, ���� ����� �������� ������ ������� ������
	int audioDialogDuration;  //������ ���������� � �������
	string audioName;			//�����-���� ���������� �������
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


	//��������:
	//������ ������� ���������� �� CDialogDispatcher. ����� ��������, ��� ����� ����� ������ ������ � pTarget
	//���������:
	//pTarget - ��������(��), � ������� ����� ����� �������� ������
	void CanDialog(IEntity* pTarget);

	//��������:
	//����� �������� ���������� ���������� ���� � �������� � 1-�� ����� �������
	//���������:
	//pAIEntity - ��� ��������, � ������� �������������
	void StartDialog(IEntity *pAIEntity);

	//��������:
	//����� ��������� ������ � �������� ���� �������
	void StopDialog();

	//��������:
	//����� ������������ XML-����, � ��������� �� ��� ������ ������
	void SetDialogFromXML(int NextDialogId, const char* filePath);

	//��������:
	//����� ������������� �����(�����), ������� ������� ��������
	void SetAnswer(const char *text);

	//��������:
	//����� ��������� ���������, �� ������� ����������� �������� ������� ������
	//� ����������� ������� ������ ����� ���������� � �����
	void SetPlayerAnswer(SDialogPlayerAnswer* pPlayerAnswer, int index);

	//��������:
	//����� ���������� ���� �����
	void Skip();

	void SetDialogCam(IEntity *pAIEntity, const char *CameraName);
	void RotateCamFocus(IEntity *pEntity, const char *BoneName, const char *CameraName);
	void SetQuestCam(string camname);
	void SetAudio(const char *AudioName, bool bEnabled = false);


	////���������� �����������
	//��������:
	//����� ������������� �������� �����(��� � ������ ������), ������� ��������� ����������� �������
	void ShowDotDialogIcon();

	//��������:
	//����� ���������� �� ����� ���� ������� � ����������� �� �������� bShow
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
