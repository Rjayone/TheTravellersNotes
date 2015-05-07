/*******************************************************************************************************************************

* File: CraftSystem.h/cpp

* Description: Описание системы крафта на основе инвентаря

* Created by: Andrew Medvedev

* Date: 08.02.2014

* Diko Source File

* ToDo: 1. Установление EntityId при крафте нового предмета. Либо прописать это при дропе..

* Registration: Game.h/cpp

********************************************************************************************************************************/

#include "StdAfx.h"
#include "CraftSystem.h"
#include "InventoryItems.h"
#include "RPGInventory.h"
#include "Game.h"
#include "GameActions.h"
#include "GameCVars.h"
#include "UIVisibleManager.h"


#define ITEMS_FOLDER "/Libs/Items/Library/ItemsDescription.xml"

int SCraftList::count = 0;
static CUIVisibleManager g_UIVisibleManager;

CCraftSystem::CCraftSystem()
{
	m_fInitialTime = 0;

	IGameFramework* pGameFramework = g_pGame->GetIGameFramework();
	if (pGameFramework != NULL)
		pGameFramework->RegisterListener(this, "UICraft", FRAMEWORKLISTENERPRIORITY_GAME);
}

bool CCraftSystem::ReciveInventoryItems()
{
	CRPGInventory *pUIInventory = g_pGame->GetRPGInventory();
	if (!pUIInventory)
		return false;

	pCraftList.clear();

	for (int i = 0; i < pUIInventory->m_pItemsArray.size(); i++)
	{
		if (pUIInventory->m_pItemsArray[i]->type == EIT_Recipe)
		{
			const char* name = pUIInventory->m_pItemsArray[i]->name.c_str();
			SCraftList* item = GetRecipeDescription(name);
			AddToList(item);
		}
	}
	return true;
}

SCraftList* CCraftSystem::GetRecipeDescription(const char *name, int index)
{
	XmlNodeRef CraftDescriptionFile = gEnv->pSystem->LoadXmlFromFile(PathUtil::GetGameFolder() + ITEMS_FOLDER);
	if (CraftDescriptionFile)
	{
		for (int i = 0; i<CraftDescriptionFile->getChildCount(); i++)
		{
			XmlNodeRef cItems = CraftDescriptionFile->getChild(i);//<item.../>
			if (cItems != NULL)
			{
				SCraftList *item = new SCraftList;
				const char *temp = cItems->getAttr("class");
				int _type; cItems->getAttr("type", _type);
				if (!strcmp(temp, "Recipe") || !strcmp(temp, "recipe") || _type == EIT_Recipe)
				{
					int count = cItems->getChildCount();
					for (int j = 0; j < count; j++)
					{
						XmlNodeRef cRecipe = cItems->getChild(j);//<param ... >
						if (cRecipe != NULL)
						{
							if (!strcmp(name, cRecipe->getAttr("name")))
							{
								item->name = cRecipe->getAttr("name");
								item->description = cRecipe->getAttr("descr");
								item->needs[0] = cRecipe->getAttr("reg1");
								item->needs[1] = cRecipe->getAttr("reg2");
								item->needs[2] = cRecipe->getAttr("reg3");
								item->needs[3] = cRecipe->getAttr("reg4");
								item->needs[4] = cRecipe->getAttr("reg5");
								item->needs[5] = cRecipe->getAttr("reg6");
								cRecipe->getAttr("recipetype", item->type);
								item->condition[0] = cRecipe->getAttr("condition");
								return item;
							}
						}
					}
				}
			}
		}
	}
}

void CCraftSystem::CraftItem(SCraftList *pRecipe, const int index)
{
	//pRecipe - текущий рецепт, который выбран
	bool bOk = true;
	CRPGInventory *pInventory = g_pGame->GetRPGInventory();
	if (!pInventory || pInventory->m_pItemsArray.size() == 0)
		bOk = false;

	int size = pInventory->m_pItemsArray.size();
	for (int j = 0; j < _RECIPE_MAX_REG_COUNT_; j++)
	{
		int regCount = 0;//счетчик нужных предметов в инве
		char* reg = new char[strlen(pRecipe->needs[j]) + 1];
		strcpy(reg, pRecipe->needs[j]);

		if (!strcmp(pRecipe->needs[j], ""))
			break;

		int needsReg = GetRegCount(reg);
		for (int i = 0; i < size; i++)
		{
			if (!strcmp(pInventory->m_pItemsArray[i]->name, GetRegName(reg)))
				regCount++;
		}

		if (regCount < needsReg)
		{
			bOk = false;
			CryLogAlways("Lack of %s", GetRegName(reg));
		}
	}

	if (bOk)
	{
		for (int i = 0; i < _RECIPE_MAX_REG_COUNT_; i++)
		{
			if (!strcmp(pRecipe->needs[i], ""))
				break;

			char* needreg = new char[strlen(pRecipe->needs[i])+1];
			strcpy(needreg, pRecipe->needs[i]);
			int needsReg = GetRegCount(needreg);
			char* regname = GetRegName(needreg);

			for (int j = 0; j < needsReg; j++)
			{
				if (!DeleteUsedItems(regname))
					return;
			}
		}
	}

	SInventoryItem *pItem = pInventory->GetItemParamsXML(pRecipe->name);
	pInventory->AddItem(pItem);

}

void CCraftSystem::InitList()
{
	if (!ReciveInventoryItems())
		return;

	int size = pCraftList.size();
	for (int i = 0; i < size; i++)
	{	
		SUIArguments item;		
		item.AddArgument(pCraftList[i]->name);
		item.AddArgument(pCraftList[i]->description);
		item.AddArgument(pCraftList[i]->needs[0]);
		item.AddArgument(pCraftList[i]->needs[1]);
		item.AddArgument(pCraftList[i]->needs[2]);
		item.AddArgument(pCraftList[i]->needs[3]);
		item.AddArgument(pCraftList[i]->needs[4]);
		item.AddArgument(pCraftList[i]->needs[5]);
		item.AddArgument(pCraftList[i]->type);

		IUIElement *pUICraft = gEnv->pFlashUI->GetUIElement("CraftMenu");
		if (!pUICraft) return;
		pUICraft->CallFunction("AddRecipe", item);
	}
}

char* CCraftSystem::GetRecipeByIndex(int index)
{
	int size = pCraftList.size();
	for (int i = 0; i < size; i++)
	{
		if (i == index)
		{
			return ((char*)pCraftList[i]->name.c_str());
			CryLog("[CRaftSystem]: Delete %s, index %d ", pCraftList[i]->name, index);
		}
	}
	return 0;
}

void CCraftSystem::AddToList(SCraftList *pCraftItem)
{
	pCraftList.push_back(pCraftItem);
	/*int size = pCraftList.size();
	SCraftList *temp = new SCraftList[size];
	for (int i = 0; i < size; i++)
		temp[i] = pCraftList[i];
	delete[] pCraftList;

	pCraftList = new SCraftList[size + 1];
	for (int i = 0; i < size; i++)
		pCraftList[i] = temp[i];

	pCraftList[size] = *pCraftItem;
*/
	CryLogAlways("[CraftSystem]: Recipe %s was added", pCraftItem->name);
}

void CCraftSystem::DeleteItem(const char* name)
{
	if (name != NULL)
	{
		for (int i = 0; i<pCraftList.size(); i++)
		{
			if (!strcmp(pCraftList[i]->name, name))
			{
				CryLogAlways("[CraftSystem]: Recipe %s was deleted", pCraftList[i]->name);
				pCraftList.erase(pCraftList.begin() + i);
				return;
			}
		}
	}
}

void CCraftSystem::DeleteItem(int index)
{
	if (index >= 0)
	{
		CryLogAlways("[CraftSystem]: Recipe %s was deleted", pCraftList[index]->name);
		pCraftList.erase(pCraftList.begin() + index);	
	}
}

void CCraftEventListener::OnUIEvent(IUIElement* pSender, const SUIEventDesc& event, const SUIArguments& args)
{
	pCraft = g_pGame->GetCraftSystem();
	if (!pCraft)
		return;

	if (!strcmp(event.sDisplayName, "UpdateList"))
	{
		pCraft->InitList();
	}

	if (!strcmp(event.sDisplayName, "CraftItem"))
	{
		SCraftList *temp;
		int index;
		args.GetArg(0, index);

		temp = pCraft->GetCraftList();
		pCraft->CraftItem(&temp[index], -1);
	}

	if (!strcmp(event.sDisplayName, "InitList"))
	{
		pCraft->InitList();
	}
	if (!strcmp(event.sDisplayName, "Unload"))
	{
		pCraft->SetInitialTime(gEnv->pTimer->GetAsyncCurTime());
		pCraft->SetCloseEvent(true);
	}

	if (!strcmp(event.sDisplayName, "OnUse"))
	{
		int index = args.GetArg(0, index);

		SCraftList *item;
		item = pCraft->GetCraftList();

		CRPGInventory *pInv = g_pGame->GetRPGInventory();
		if (pInv == NULL) return;

		SInventoryItem *pInvItem = pInv->GetInventoryItemByName(item[index].name);
		if (pInvItem)
			pInv->OnUse(pInvItem->itemId);
	}

	if (!strcmp(event.sDisplayName, "DeleteItem"))
	{
		int index;
		args.GetArg(0, index);
		char* name = pCraft->GetRecipeByIndex(index);
		pCraft->DeleteItem(index);
	

		CRPGInventory *pInv = g_pGame->GetRPGInventory();
		if (pInv == NULL) return;

		SInventoryItem *Item = pInv->GetInventoryItemByName(name);
		if (Item)
			pInv->OnDropItem(Item->itemId, 4);
	}
	if (!strcmp(event.sDisplayName, "Close"))
	{
		// Старт таймера на закрытие элемента
		// см. OnPostUpdate()
		pCraft->SetInitialTime(gEnv->pTimer->GetAsyncCurTime());
		pCraft->SetCloseEvent(true);
	}
}

void CCraftEventListener::OnAction(const ActionId& action, int activationMode, float value)
{
	const CGameActions &actions = g_pGame->Actions();
	if (actions.craft == action && gEnv->pFlashUI)
	{
		if (IUIElement* pUICraft = gEnv->pFlashUI->GetUIElement("CraftMenu"))
		{
			CCraftSystem* pCraftSys = g_pGame->GetCraftSystem();
			if (!pCraftSys)
				return;

			//pUICraft->AddEventListener(this, "CraftMenu");
			if (!pUICraft->IsVisible())
			{
				//Скрываем прочий худ
				g_UIVisibleManager.HideAllUIElements();

				pUICraft->SetVisible(true);
				//pUICraft->CallFunction("ClearList");
				pCraftSys->InitList();
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
			else
			{
				pCraftSys->SetInitialTime(gEnv->pTimer->GetAsyncCurTime());
				pCraftSys->SetCloseEvent(true);
				g_UIVisibleManager.ShowHidenUIElements(true);
			}
		}
	}
}

CCraftEventListener::CCraftEventListener() : pCraft(NULL)
{
	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pAmMgr)
	{
		pAmMgr->AddExtraActionListener(this);
		pAmMgr->EnableActionMap("menus", true);
		IActionMap *map = pAmMgr->GetActionMap("menus");
		if (map)
			map->Enable(true);
	}
}

int CCraftSystem::GetRegCount(char* name)
{
	char *number = new char[1];
	char *tempName = new char[strlen(name) + 1];
	strcpy(tempName, name);

	int len = strlen(tempName) - 1;
	tempName[len] = '\0';//отрезаем скобочку
	int ind = 0;

	for (int i = 0; i<len; i++)
	{
		if ('(' == tempName[i])
		{
			ind = len - i - 1;
			number = &tempName[i + 1];
		}
	}
	number[ind] = '\0';
	return atoi(number);
}

void CCraftSystem::OnPostUpdate(float fDeltaTime)
{
	if (m_bCloseEvent)
	{
		if (ICVar* myVar = gEnv->pConsole->GetCVar("t_scale"))
			myVar->ForceSet("1");
		if (gEnv->pTimer->GetCurrTime() - m_fInitialTime >= 0.05)
		{
			m_bCloseEvent = false;
			IUIElement* pUICraft = gEnv->pFlashUI->GetUIElement("CraftMenu");
			if (!pUICraft)
				return;
			//pUICraft->CallFunction("ClearList");
			pUICraft->SetVisible(false);
			pUICraft->Unload();		
			g_pGameActions->FilterNoMove()->Enable(false);
			g_pGameActions->FilterNoMouse()->Enable(false);

			IRenderer* pRenderer = gEnv->pRenderer;
			CRY_ASSERT(pRenderer);
			pRenderer->EF_SetPostEffectParam("Dof_User_Active", 1.f, true);
			pRenderer->EF_SetPostEffectParam("Dof_User_FocusDistance", 10000, true);
			pRenderer->EF_SetPostEffectParam("Dof_User_FocusRange", 0, true);
			pRenderer->EF_SetPostEffectParam("Dof_User_BlurAmount", 10, true);
			pRenderer->EF_SetPostEffectParam("Dof_User_ScaleCoc", 1, true);
		}
	}
}

void CCraftSystem::OnSaveGame(ISaveGame* pSaveGame)
{}
void CCraftSystem::OnLoadGame(ILoadGame* pLoadGame)
{}
void CCraftSystem::OnLevelEnd(const char* nextLevel)
{}
void CCraftSystem::OnActionEvent(const SActionEvent& event)
{
	if (event.m_event == eAE_inGame && gEnv->pFlashUI)
	{
		if (IUIElement* pUICraft = gEnv->pFlashUI->GetUIElement("CraftMenu"))
		{
			pUICraft->AddEventListener(&Listener, "CraftMenu");
			pUICraft->SetVisible(false);
		}
	}

	if (event.m_event == eAE_unloadLevel && gEnv->pFlashUI)
	{
		if (IUIElement* pUICraft = gEnv->pFlashUI->GetUIElement("CraftMenu"))
			pUICraft->RemoveEventListener(&Listener);
	}
}

char* CCraftSystem::GetRegName(char* name)
{
	int len = strlen(name);
	int ind = 0;
	for (int i = 0; i < len; i++)
	{
		if ('(' == name[i])
			ind = i;
	}
	if (ind != 0)
		name[ind] = '\0';
	return name;
}

bool CCraftSystem::DeleteUsedItems(char* name)
{
	CRPGInventory *pInventory = g_pGame->GetRPGInventory();
	if (!pInventory)
		return false;

	bool deleted = pInventory->DeleteItem(name);
	return deleted;
}

SCraftList* CCraftSystem::GetCraftList()
{ return pCraftList[0]; }