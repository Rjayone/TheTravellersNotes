/****************************************

* File: Economics.h/cpp

* Description: Описание системы экономики

* Created by: Dmitriy Miroshnichenko & Andrew Medvedev

* Date: 24.11.2014

* Diko Source File

* ToDo: 1. Сделать возможность прибавлять разные ресурсы в разные промежутки времени?
2. Импорт ресурсов из XML?
3. Сохранение.
4. Меню.

*****************************************/

#include "StdAfx.h"
#include "Economics.h"
#include "UIVisibleManager.h"

// временно, заглушки для сохранения / загрузки
#define saveResource
#define getResources() m_pResources

// :-----------------------------------------------------------------:
// Описание системы

CEconomics::CEconomics(){
	IGameFramework *pGF = g_pGame->GetIGameFramework();
	if (pGF == NULL)
		return;
	pGF->RegisterListener(this, "Economics", EFRAMEWORKLISTENERPRIORITY::FRAMEWORKLISTENERPRIORITY_DEFAULT);

	// подписываемся на прослушивание событий
	IActionMapManager* pAmMgr = pGF->GetIActionMapManager();
	if (pAmMgr)
		pAmMgr->AddExtraActionListener(this);

	m_lastTime = 0;
}

CEconomics::~CEconomics(){
	IGameFramework *pGF = g_pGame->GetIGameFramework();
	if (pGF == NULL)
		return;
	pGF->UnregisterListener(this);

	IActionMapManager* pAmMgr = pGF->GetIActionMapManager();
	if (pAmMgr)
		pAmMgr->RemoveExtraActionListener(this);
}

void CEconomics::Init(){
	CEconomicResource* pWoodResource = new CEconomicResource(e_EconomicResourceWood, 0, 20, 100, "Wood");
	CEconomicResource* pIronResource = new CEconomicResource(e_EconomicResourceIron, 0, 20, 100, "Iron");
	CEconomicResource* pStoneResource = new CEconomicResource(e_EconomicResourceStone, 0, 20, 100, "Stone");

	AddResource(pWoodResource);
	AddResource(pIronResource);
	AddResource(pStoneResource);
}


// :-----------------------------------------------------------------:
// Свойства системы


void CEconomics::AddResource(CEconomicResource *pResource){
	m_pResources.push_back(pResource);
	CryLog("[CEconomics]: Resource with name %s added", pResource->GetName());
}

void CEconomics::DeleteResource(int type){
	for (int i = 0; i < m_pResources.size(); i++){
		if (m_pResources[i]->GetType() == type){
			CryLog("[CEconomics]: Resource with name %s deleted", m_pResources[i]->GetName());
			m_pResources.erase(m_pResources.begin() + i);
		}
	}
}

CEconomicResource* CEconomics::GetResource(int type){
	for (int i = 0; i < m_pResources.size(); i++){
		if (m_pResources[i]->GetType() == type)
			return m_pResources[i];
	}
	CryLog("[CEconomics]: Resource %d not found", type);
	return NULL;
}

void CEconomics::IncreaseResource(int type, int count){
	CEconomicResource *resource = GetResource(type);
	resource->SetCount(resource->GetCount() + count);
	CryLog("[CEconomics]: Resource %d increased (%d)", type, count);
}

void CEconomics::DecreaseResource(int type, int count){
	CEconomicResource *resource = GetResource(type);
	resource->SetCount(resource->GetCount() - count);
	CryLog("[CEconomics]: Resource %d decreased (%d)", type, count);
}

void CEconomics::ProcessAllResources(){
	IUIElement *pElement = gEnv->pFlashUI->GetUIElement("EconomicMenu");
	if (pElement){
		for (int i = 0; i < m_pResources.size(); i++){
			m_pResources[i]->SetCount(m_pResources[i]->GetModifier(), true);
			SUIArguments args;
			args.AddArgument(m_pResources[i]->GetType());
			args.AddArgument(m_pResources[i]->GetCount());
			pElement->CallFunction("SetResource", args);
		}
	}
	else {
		for (int i = 0; i < m_pResources.size(); i++){
			m_pResources[i]->SetCount(m_pResources[i]->GetModifier(), true);
		}
	}
	CryLog("[CEconomics]: All resource increased");
}

// :-----------------------------------------------------------------:
// События

void CEconomics::OnPostUpdate(float fDeltaTime){
	float curTime = gEnv->pTimer->GetCurrTime();
	if (curTime - m_lastTime >= TIMETOUPDATE){
		ProcessAllResources();
		m_lastTime = curTime;
	}
}

void CEconomics::OnSaveGame(ISaveGame* pSaveGame){
	// пробегаем по всем ресурсам.
	// получаем кол-во и тип каждого.
	// и потом будем как-нибудь сохранять.
	for (int i = 0; i < m_pResources.size(); i++){
		int type = m_pResources[i]->GetType();
		int count = m_pResources[i]->GetCount();
		saveResource(type, count);
	}
}

void CEconomics::OnLoadGame(ILoadGame* pLoadGame){
	m_pResources = getResources();
}

void CEconomics::OnLevelEnd(const char* nextLevel){}

void CEconomics::OnAction(const ActionId& action, int activationMode, float value){
	/* if (action == g_pGameActions->economics && gEnv->pFlashUI){
		if (IUIElement* pUIEconomics = gEnv->pFlashUI->GetUIElement("EconomicMenu")){
			if (!pUIEconomics->IsVisible()){
				static CUIVisibleManager g_UIVisibleManager;
				g_UIVisibleManager.HideAllUIElements(); // скрывает hud
				pUIEconomics->SetVisible(true);
				IRenderer* pRenderer = gEnv->pRenderer;
				CRY_ASSERT(pRenderer);
				pRenderer->EF_SetPostEffectParam("Dof_User_Active", 1.f, true);
				pRenderer->EF_SetPostEffectParam("Dof_User_FocusDistance", 0, true);
				pRenderer->EF_SetPostEffectParam("Dof_User_FocusRange", 0, true);
				pRenderer->EF_SetPostEffectParam("Dof_User_BlurAmount", 10000, true);
				pRenderer->EF_SetPostEffectParam("Dof_User_ScaleCoc", 1000, true);

				g_pGameActions->FilterNoMove()->Enable(true);
				g_pGameActions->FilterNoMouse()->Enable(true);
				if (ICVar* myVar = gEnv->pConsole->GetCVar("t_scale"))
					myVar->ForceSet("0.1");
			}
			else {
				pUIEconomics->SetVisible(false);
				pUIEconomics->Unload();
				g_pGameActions->FilterNoMove()->Enable(false);
				g_pGameActions->FilterNoMouse()->Enable(false);

				IRenderer* pRenderer = gEnv->pRenderer;
				CRY_ASSERT(pRenderer);
				pRenderer->EF_SetPostEffectParam("Dof_User_Active", 1.f, true);
				pRenderer->EF_SetPostEffectParam("Dof_User_FocusDistance", 10000, true);
				pRenderer->EF_SetPostEffectParam("Dof_User_FocusRange", 0, true);
				pRenderer->EF_SetPostEffectParam("Dof_User_BlurAmount", 10, true);
				pRenderer->EF_SetPostEffectParam("Dof_User_ScaleCoc", 1, true);

				if (ICVar* myVar = gEnv->pConsole->GetCVar("t_scale"))
					myVar->ForceSet("1");
			}
		}
	} */
}

void CEconomics::OnActionEvent(const SActionEvent& event){}