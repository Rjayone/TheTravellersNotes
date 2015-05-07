/*********************************************************************************************************
- File: BuildMenu.cpp
- Description: Основное флеш меню режима стройки с реализацией функций стройки и связи Флэш и Края
- Created by: Гомза Яков
- Date: 25.03.2014
- Diko Source File
- ToDo: Возможно дополнение экономической части
Разрешать вызов только в определенных зонах. Для этого написать нод, или от определенного объекта
Добавить функцию PostBuild для вызова функций после пострйоки здания
Посмотреть возможность кэша зданий
********************************************************************************************************/


#include "StdAfx.h"
#include "BuildMenu.h"
#include "GameActions.h"
#include "Globals.h"
#include "IViewSystem.h"
#include "UIVisibleManager.h"
#include "Economics.h"
#include "PlayerStatsManager.h"

static CUIVisibleManager g_UIVisibleManager;

CUIBuildMenu::CUIBuildMenu()
{
	m_pBuildMenu = NULL;
	m_pSelectedEntity = NULL;
	m_bCloseEvent = false;
	m_bBuildModeEnable = false;

	m_BuildCamera = new CBuildCamera;
	m_Helper = new CSelectionBuildHelper;

	IGameFramework *pGameFramework = g_pGame->GetIGameFramework();
	if (pGameFramework)
		pGameFramework->RegisterListener(this, "BuildMenu", EFRAMEWORKLISTENERPRIORITY::FRAMEWORKLISTENERPRIORITY_HUD);

	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pAmMgr != NULL)
		pAmMgr->AddExtraActionListener(this);
}

CUIBuildMenuEventListener::CUIBuildMenuEventListener()
{}

CUIBuildMenu::~CUIBuildMenu()
{
	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pAmMgr != NULL)
		pAmMgr->RemoveExtraActionListener(this);


	IGameFramework *pGameFramework = g_pGame->GetIGameFramework();
	if (pGameFramework)
		pGameFramework->UnregisterListener(this);
}


void CUIBuildMenuEventListener::OnUIEvent(IUIElement* pSender, const SUIEventDesc& Event, const SUIArguments& args)
{
	if (!strcmp(Event.sDisplayName, "Buy"))
	{
		int index;
		args.GetArg(0, index);


		//ToDo:
		//Код ниже стоит вынести в отдельную функция в классе CUIBuildMenu!
		CPlayerStatsManager *pPlayerStats = g_pGame->GetPlayerStatsManager();
		CSelectionBuildHelper* pHelper = g_pGame->GetBuildMenu()->GetSelectionHelper();
		CUIBuildMenu *pBuildMenu = g_pGame->GetBuildMenu();
		SBuilding *pBuild = pHelper->GetBuilding(index);
		CEconomics *pEconomics = g_pGame->GetEconomics();
		if (!pBuild) return;

		bool bLackOfRes = true;
		int Money = 1000;//pPlayerStats->GetStat(EPS_Money)->GetStatValue();
		int Wood  = pEconomics->GetResource(e_EconomicResourceWood )->GetCount();//pPlayerStats->GetStat(EPS_Wood )->GetStatValue();
		int Stone = pEconomics->GetResource(e_EconomicResourceStone)->GetCount();//pPlayerStats->GetStat(EPS_Stone)->GetStatValue();
		int Iron  = pEconomics->GetResource(e_EconomicResourceIron )->GetCount() ;//pPlayerStats->GetStat(EPS_Iron )->GetStatValue();

		if (Money >= pBuild->Cost)
		{
			if (Wood >= pBuild->Wood)
			{
				if (Stone >= pBuild->Stone)
				{
					if (Iron >= pBuild->Iron)
					{
						pBuildMenu->GetSelectionHelper()->SpawnBuilding(pBuild->Model);
						pPlayerStats->GetStat(EPS_Money)->SetValue(Money - pBuild->Cost);
						pPlayerStats->GetStat(EPS_Wood)->SetValue(Wood - pBuild->Wood);
						pPlayerStats->GetStat(EPS_Stone)->SetValue(Stone - pBuild->Stone);
						pPlayerStats->GetStat(EPS_Iron)->SetValue(Iron - pBuild->Iron);
						gEnv->pEntitySystem->RemoveEntity(pBuildMenu->GetSelectedEntity()->GetId());// удаление хэлпера
						
						pBuildMenu->m_PerformedBuildings.push_back(pBuild);
#ifdef __DELETE_BUILD_CAMERA__
						//Переключаем вид
						pBuildMenu->PriveousCamView();

						//Удаление камеры
						const char* temp = pBuildMenu->m_pSelectedEntity->GetName();
						char* name = new char;
						strcpy(name, temp);
						int i = 0, j;
						for (i, j = 11; j < strlen(name); i++, j++)
							name[i] = name[j];
						name[i] = '\0';
						string path = "BuildCamera" + (string)name;
						IEntity *pCam = gEnv->pEntitySystem->FindEntityByName(path);
						pBuildMenu->m_BuildCamera->m_pGameCameraEntity.erase(pBuildMenu->m_BuildCamera->m_pGameCameraEntity.begin() + atoi(name)-1);
						gEnv->pEntitySystem->RemoveEntity(pCam->GetId(), true);
#endif
						
						bLackOfRes = false;
					}
				}
			}
		}
		if (bLackOfRes == true)
		{
			CryLogAlways("Lack of resources");
			return;
		}
	}
	//~

	if (!strcmp(Event.sDisplayName, "Exit"))
	{
		g_pGame->GetBuildMenu()->Delay();
		g_pGame->GetBuildMenu()->SetBuildMode(false);
		IView* view = g_pGame->GetIGameFramework()->GetIViewSystem()->GetActiveView();
		view->LinkTo(g_pGame->GetIGameFramework()->GetClientActor()->GetEntity());
		g_pGameActions->FilterNoMove()->Enable(false);
		g_pGameActions->FilterNoMouse()->Enable(false);
	}

	//Изменения вида(переключение между камерами)
	if (Event.sDisplayName == string("ChangeViewNext"))
	{
		g_pGame->GetBuildMenu()->NextCamView();
	}

	if (Event.sDisplayName == string("ChangeViewPrev"))
	{
		g_pGame->GetBuildMenu()->PriveousCamView();
	}
}


void CUIBuildMenu::NextCamView()
{
	IView* view = g_pGame->GetIGameFramework()->GetIViewSystem()->GetActiveView();
	char* temp = (char*)gEnv->pEntitySystem->GetEntity( view->GetLinkedId())->GetName();
	char* name = new char;
	strcpy(name, temp);
	int i = 0, j;
	for (i, j = 11; j < strlen(name); i++, j++)
		name[i] = name[j];
	name[i] = '\0';
	int index = atoi(name);
	if (index < m_BuildCamera->m_pGameCameraEntity.size())
	{
		view->LinkTo(m_BuildCamera->m_pGameCameraEntity[index]); //тут +1 было
	}
}

void CUIBuildMenu::PriveousCamView()
{
	IView* view = g_pGame->GetIGameFramework()->GetIViewSystem()->GetActiveView();
	char* temp = (char*)gEnv->pEntitySystem->GetEntity(view->GetLinkedId())->GetName();
	char* name = new char;
	strcpy(name, temp);
	int i = 0, j;
	for (i, j = 11; j < strlen(name); i++, j++)
		name[i] = name[j];
	name[i] = '\0';
	int index = atoi(name) - 2;
	if (index < 0) index = m_BuildCamera->m_pGameCameraEntity.size() + index;
	if (index >= 0)
	{
		view->LinkTo(m_BuildCamera->m_pGameCameraEntity[index]);
	}
}


void CUIBuildMenu::Delay()
{
	if (gEnv->pTimer)
	{
		m_fInitialTime = gEnv->pTimer->GetCurrTime();
		m_bCloseEvent = true;
	}
}


void CUIBuildMenu::OnAction(const ActionId& action, int activationMode, float value)
{
	const CGameActions &actions = g_pGame->Actions();

	if (actions.SelectBuildHelper == action && m_bBuildModeEnable == true)
	{
		CSelectionBuildHelper *helper = g_pGame->GetBuildMenu()->GetSelectionHelper();
		helper->Select();
	}

	if (actions.BuildMode == action)
	{
		if (m_bBuildModeEnable == true)
		{
			g_pGame->GetBuildMenu()->Delay();
			m_bBuildModeEnable = false;

			IView* view = g_pGame->GetIGameFramework()->GetIViewSystem()->GetActiveView();
			view->LinkTo(g_pGame->GetIGameFramework()->GetClientActor()->GetEntity());

			g_pGame->GetBuildMenu()->DisplayBuildMenu(false);
			g_pGameActions->FilterNoMove()->Enable(false);
			g_pGameActions->FilterNoMouse()->Enable(false);
		}
		else
		{
			m_bBuildModeEnable = true;
			Init();
			g_pGame->GetBuildMenu()->DisplayBuildMenu(true);
			g_pGameActions->FilterNoMove()->Enable(true);
			g_pGameActions->FilterNoMouse()->Enable(true);
		}
	}

	if (actions.ChangeViewNext == action)
	{
		g_pGame->GetBuildMenu()->NextCamView();
	}

	if (actions.ChangeViewPrev == action)
	{
		g_pGame->GetBuildMenu()->PriveousCamView();
	}
}

void CUIBuildMenu::OnPostUpdate(float fDeltaTime)
{
	if (m_bCloseEvent)
	{
		if (gEnv->pTimer->GetCurrTime() - m_fInitialTime >= 0.2)
		{
			//закрываем флеш
			m_bCloseEvent = false;
			DisplayBuildMenu(false);
		}
	}
}

void CUIBuildMenu::Clear()
{
	m_pBuildMenu->CallFunction("Clear");
}

void CUIBuildMenu::ShowAcceptMenu(bool bAccept)
{
	SUIArguments args;
	args.AddArgument(bAccept);
	m_pBuildMenu->CallFunction("SetVisible", args);
}


void CUIBuildMenu::SendBuildingsInfo(SBuilding* build)
{
	SUIArguments args;
	args.AddArgument(build->Name);
	args.AddArgument(build->Descr);
	args.AddArgument(build->Additional);
	args.AddArgument(build->Type);
	args.AddArgument(build->Cost);
	args.AddArgument(build->Wood);
	args.AddArgument(build->Stone);
	args.AddArgument(build->Iron);
	m_pBuildMenu->CallFunction("CreateBuildingList", args);
}

bool CUIBuildMenu::Init()
{
	if (gEnv->pFlashUI)
	{
		if (m_pBuildMenu == NULL)
			m_pBuildMenu = gEnv->pFlashUI->GetUIElement("BuildMenu");
	}
	return true;
}

void CUIBuildMenu::DisplayBuildMenu(bool bShow)
{
	if (bShow)
	{
		m_pBuildMenu = gEnv->pFlashUI->GetUIElement("BuildMenu");
		if (m_pBuildMenu != NULL)
		{
			if (m_bBuildModeEnable == true)
			{
				//Скрываем прочий худ
				g_UIVisibleManager.HideAllUIElements();

				m_pBuildMenu->SetVisible(true);
				m_pBuildMenu->AddEventListener(&g_CUIBuildSystemEventListener, "BuildMenuListener");//Добавляем наш класс что бы слушать собыия

				//Включаем фильтры
				g_pGameActions->FilterNoMove()->Enable(true);
				g_pGameActions->FilterNoMouse()->Enable(true);
				m_BuildCamera->CreateBuildCamera();
			}
		}
	}
	else
	{
		if (m_pBuildMenu != NULL)
		{
			//Удаляем наш класс из слушателей что бы не было случайного краша
			m_pBuildMenu->RemoveEventListener(&g_CUIBuildSystemEventListener);
			m_pBuildMenu->SetVisible(false); //Пряем
			m_pBuildMenu->Unload(true); //Выгружаем

			//Отображаем ранее скрытые элементы
			g_UIVisibleManager.ShowHidenUIElements();
		}
		//Отключаем фильтры
		g_pGameActions->FilterNoMove()->Enable(false);
		g_pGameActions->FilterNoMouse()->Enable(false);
	}
}


void CUIBuildMenu::ShowSlideMenu(bool bShow)
{
	if (m_pBuildMenu != NULL)
	{
		if (bShow == true)
		{
			m_pBuildMenu->CallFunction("OpenMenu");
		}
		else
		{
			m_pBuildMenu->CallFunction("CloseMenu");
		}
	}
}
