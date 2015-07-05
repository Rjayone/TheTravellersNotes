/*********************************************************************************************************
- File: Events.h
- Description: ����� � ���������� �������� ���������
- Created by: Andrew Medvedev
- Date: 26:06:2015
- Edda Source File
********************************************************************************************************/

#pragma once

#include "IGameFramework.h"

//���������, � ������� ����������� ��������� ������ ��� ��������� ������
//�� ��� �� ������ ��������� ���� �� ������, ������� ������ � ���������� ��������� � ��������
//��� �������� �� ��� �������:
// 1. ����������� ����� ������� IEvents
// 2. � ������������ ������ ������ �������� ������� g_pGame->GetEventsDispatcher()->AddListener(this);
// 3. ���������� ����������� ������
// 4. � ����������� �������� g_pGame->GetEventDispatcher()->RemoveListener(this);
class IEvents 
{
public:
	//��������:
	//����� ����������, ����� ������������ ��������� � ������� ����� � ������
	virtual void OnPlayerEnterInEditoriGameMode(){}

	//��������:
	//����� ����������, ����� ������������ ������� �� �������� ������ � ������
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

	//��������:
	//����� ��������� ������ ���������� �� �������.
	//����������� �����, ����������, ������ ����������� IEvents
	//���� � ������� ��� ���������� ������ ���������, ��������� ������������
	void AddListener(IEvents* pListener);

	//��������:
	//����� ������� ������������� ����������
	void RemoveListener(IEvents* pListener);

	////Events
	//��������:
	//����� ����������, ����� ������������ ��������� � ������� ����� � ������
	void OnPlayerEnterInEditoriGameMode();

	//��������:
	//����� ����������, ����� ������������ ������� �� �������� ������ � ������
	void OnPlayerExitFromEditorGameMode();
private:
	std::vector<IEvents*> m_pListeners; //������ ����������
};