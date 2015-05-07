/****************************************

* File: Economics.h/cpp

* Description: Описание системы экономики

* Created by: Dmitriy Miroshnichenko & Andrew Medvedev

* Date: 24.11.2014

* Diko Source File

*****************************************/

#pragma once
#include "Game.h"
#include "EconomicResources.h"

// интервал добавления ресурсов (в секундах)
#define TIMETOUPDATE 10

class CEconomics : public IGameFrameworkListener, public IActionListener {
	void AddResource(CEconomicResource *pResource);
	void DeleteResource(int type);
	void IncreaseResource(int type, int count); // прибавляет count ресурса type
	void DecreaseResource(int type, int count); // отнимает count ресурсов type
	void ProcessAllResources(); // увеличивает / уменьшает все ресурсы
	std::vector<CEconomicResource*> m_pResources;
	int m_lastTime; // последний раз, когда обновились ресурсы, с момента запуска системы

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