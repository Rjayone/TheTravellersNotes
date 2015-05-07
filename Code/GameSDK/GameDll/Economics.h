/****************************************

* File: Economics.h/cpp

* Description: �������� ������� ���������

* Created by: Dmitriy Miroshnichenko & Andrew Medvedev

* Date: 24.11.2014

* Diko Source File

*****************************************/

#pragma once
#include "Game.h"
#include "EconomicResources.h"

// �������� ���������� �������� (� ��������)
#define TIMETOUPDATE 10

class CEconomics : public IGameFrameworkListener, public IActionListener {
	void AddResource(CEconomicResource *pResource);
	void DeleteResource(int type);
	void IncreaseResource(int type, int count); // ���������� count ������� type
	void DecreaseResource(int type, int count); // �������� count �������� type
	void ProcessAllResources(); // ����������� / ��������� ��� �������
	std::vector<CEconomicResource*> m_pResources;
	int m_lastTime; // ��������� ���, ����� ���������� �������, � ������� ������� �������

public:
	CEconomics();
	~CEconomics();

	void Init();
	CEconomicResource* GetResource(int type);

	void OnPostUpdate(float fDeltaTime);
	void OnSaveGame(ISaveGame* pSaveGame);
	void OnLoadGame(ILoadGame* pLoadGame);
	void OnLevelEnd(const char* nextLevel);
	void OnActionEvent(const SActionEvent& event);
	void OnAction(const ActionId& action, int activationMode, float value);
};