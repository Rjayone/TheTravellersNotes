/****************************************

* File: RPGInventory.h/cpp

* Description: Описание системы инвентаря

* Created by: Andrew Medvedev

* Date: 08.02.2014

* Diko Source File

* ToDo: 1. Реализовать возможность определения новых(доп.) сумок игрока(-)
2. Улучшить систему считывания информации для каждого типа предметов(+)
3. Уточнить ид предметов при выбрасывании(+)
4. При выбрасывание некоторых скравченых предметов выбрасыват их составляющие(-)

*****************************************/

#include "StdAfx.h"
#include "RPGInventory.h"
#include "RPGInventoryManager.h"

#include "Game.h"
#include "ISaveGame.h"
#include "ISerialize.h"

#include "Actor.h"
#include "Player.h"
#include "GameActions.h"

#include "DialogSystem.h"
#include "NPCControl.h"
#include "CameraMode.h"

#include "BasicFood.h"
#include "BasicRecipe.h"
#include "FirePlace.h"
#include "UIVisibleManager.h"
#include "InventoryItems.h"


#define ITEMS_FOLDER "/Libs/Items/Library/ItemsDescription.xml"
#define UI_NAME "Inventory"
#define INITIAL_SLOT_COUNT 15
#define USABLE_DISTANT 5//+RPGInventoryManager

int SInventoryItem::globalCount = 0;
static CUIVisibleManager g_UIVisibleManager;

CRPGInventory::CRPGInventory()
{
	m_bGameStarted = false;
	m_SlotsCount = INITIAL_SLOT_COUNT;
	m_pUIInventory = NULL;

	if (gEnv->pFlashUI != NULL)
	{
		m_pUIInventory = GetInventoryUIInterface();
		if (m_pUIInventory != NULL)
			m_pUIInventory->AddEventListener(this, "UIInventoryListener");
	}

	if (g_pGame->GetIGameFramework() != NULL)
		g_pGame->GetIGameFramework()->RegisterListener(this, "UIInventory", FRAMEWORKLISTENERPRIORITY_GAME);
}

CRPGInventory::~CRPGInventory()
{
}

IUIElement* CRPGInventory::GetUIElement()
{
	if (gEnv->pFlashUI)
	{
		m_pUIInventory = gEnv->pFlashUI->GetUIElement(UI_NAME);
		return m_pUIInventory;
	}
	return NULL;
}

void CRPGInventory::OnLoadGame(ILoadGame* pLoadGame)  {}
void CRPGInventory::OnLevelEnd(const char* nextLevel) {}

void CRPGInventory::OnActionEvent(const SActionEvent& event)
{
	if (event.m_event == eAE_inGame)
	{
		m_bGameStarted = true;
		m_pUIInventory = GetInventoryUIInterface();
		ResetUIItemsArray();
	}

	//UnLoad level -------------------------------------------------------------
	if (event.m_event == eAE_unloadLevel && m_pUIInventory != NULL)
	{
		m_pUIInventory->RemoveEventListener(this);
		m_pUIInventory->CallFunction("ResetItemsArray");
		m_pUIInventory->SetVisible(false);
		m_pUIInventory->Unload();
		m_bGameStarted = false;
	}
}

SInventoryItem* CRPGInventory::GetItemParamsXML(const char* name)
{
	SInventoryItem *item = new SInventoryItem();
	XmlNodeRef ItemsDescriptionFile = gEnv->pSystem->LoadXmlFromFile(PathUtil::GetGameFolder() + ITEMS_FOLDER);
	if (ItemsDescriptionFile == NULL || item == NULL)
		return NULL;

	for (int i = 0; i<ItemsDescriptionFile->getChildCount(); i++)
	{
		XmlNodeRef cItems = ItemsDescriptionFile->getChild(i);//<item ... />
		if (cItems != NULL)
		{
			const char *className = cItems->getAttr("class");
			int _type = atoi(cItems->getAttr("type"));
			if (_type == EIT_Weapon)
			{
				if (!strcmp(className, name))
				{
					item->name = className;
					item->description = cItems->getAttr("descr");
					cItems->getAttr("size", item->size);
					cItems->getAttr("cost", item->cost);
					cItems->getAttr("type", item->type);
					cItems->getAttr("hiden", item->hiden);
					return item;
				}
			}
		}
	}
	return item;
}

//Можно дописать рабту с оригинальными итемс
void CRPGInventory::OnPickedUp(EntityId itemId)
{
}


//жэсточайше пофиксить!
void CRPGInventory::OnDropItem(EntityId id, float impulseScale)
{
	if (m_bGameStarted == false)
		return;

	CRPGInventoryManager *pInventoryManager = g_pGame->GetRPGInventoryManager();
	SInventoryItem* pItem = GetInventoryItemById(id);
	if (!pInventoryManager || !pItem)
		return;

	SEntitySpawnParams spawn;
	IEntityClass *pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(pItem->entityClassType);
	if (!pClass) return;
	spawn.pClass = pClass;
	spawn.vPosition = gEnv->pSystem->GetViewCamera().GetPosition() + gEnv->pSystem->GetViewCamera().GetViewdir() + Vec3(0, 0, 1);
	spawn.nFlags |= ENTITY_FLAG_CASTSHADOW;
	IEntity *pEntity = gEnv->pEntitySystem->SpawnEntity(spawn);
	if (pEntity == NULL)
		return;

	IBasicObject *pBasicObject = static_cast<IBasicObject*>(g_pGame->GetIGameFramework()->GetGameObject(pEntity->GetId())->QueryExtension(pItem->entityClassType));
	if (!pBasicObject)
		return;

	pInventoryManager->AddObjects(pBasicObject);
	pBasicObject->OnDrop(pItem, pEntity->GetId());
	if (!DeleteItem(id))
		return;
}

void CRPGInventory::OnUIEvent(IUIElement* pSender, const SUIEventDesc& event, const SUIArguments& args)
{
	if (!strcmp(event.sDisplayName, "OnItemDelete"))
	{
		EntityId id;
		args.GetArg(0, id);
		SInventoryItem *pItem = GetInventoryItemById(id);
		if (pItem != NULL)
			OnDropItem(pItem->itemId, 4.0f);
	}

	if (!strcmp(event.sDisplayName, "OnUse"))
	{
		EntityId id;
		args.GetArg(0, id);
		OnUse(id);
	}
}

void CRPGInventory::OnPostUpdate(float fDeltaTime)
{
}

void CRPGInventory::OnSaveGame(ISaveGame* pSaveGame)
{
	TSerialize ser = pSaveGame->AddSection("CRPGInventory");
	ser.BeginGroup("UIInventory");
	if (ser.IsWriting() && !m_pItemsArray.size())
	{
		int size = m_pItemsArray.size();
		ser.Value("size", size);
		for (int i = 0; i < m_pItemsArray.size(); i++)
		{
			ser.BeginGroup("UIItem");
			ser.Value("name", m_pItemsArray[i]->name);
			ser.Value("itemId", m_pItemsArray[i]->itemId);
			ser.Value("objModel", m_pItemsArray[i]->objModel);
			ser.Value("entityClassType", m_pItemsArray[i]->entityClassType);
			ser.Value("type", m_pItemsArray[i]->type);
			ser.EndGroup();
		}
	}

	if (ser.IsReading())
	{
		int size = m_pItemsArray.size();
		ser.Value("size", size);
		for (int i = 0; i < m_pItemsArray.size(); i++)
		{
			ser.BeginGroup("UIItem");
			ser.Value("name", m_pItemsArray[i]->name);
			ser.Value("itemId", m_pItemsArray[i]->itemId);
			ser.Value("objModel", m_pItemsArray[i]->objModel);
			ser.Value("entityClassType", m_pItemsArray[i]->entityClassType);
			ser.Value("type", m_pItemsArray[i]->type);
			ser.EndGroup();
		}
	}
	ser.EndGroup();
}

bool CRPGInventory::DeleteItem(const char* name)
{
	int index = GetItemIndex(name);
	if (index == -1) return false;
	m_pItemsArray.erase(m_pItemsArray.begin()+index);
	ResetUIItemsArray();

	return true;
}

bool CRPGInventory::DeleteItem(EntityId itemId)
{
	int index = GetItemIndex(itemId);
	if (index == -1) return false;
	m_pItemsArray.erase(m_pItemsArray.begin() + index);
	ResetUIItemsArray();

	return true;
}

void CRPGInventory::AddItem(SInventoryItem *pItem,int count)
{
	//Если количество итемов меньше чем начальное значение сумки
	if (m_pItemsArray.size() < m_SlotsCount || gEnv->pFlashUI)
	{
		for (int i = 0; i < count; i++)
		{

			m_pItemsArray.push_back(pItem);
			CryLogAlways("[RPGInventory]: Item %s was added", pItem->name);

			if (pItem->hiden == true)
				return;

			SUIArguments args;
			args.AddArgument(string(pItem->name));
			args.AddArgument(string(pItem->description));
			args.AddArgument(pItem->size);
			args.AddArgument(pItem->cost);
			args.AddArgument(pItem->type);
			args.AddArgument((int)pItem->itemId);

			m_pUIInventory = GetInventoryUIInterface();
			m_pUIInventory->CallFunction("CreateItem", args);
		}
	}
}

SInventoryItem* CRPGInventory::GetInventoryItemById(EntityId id)
{
	for (int i = 0; i<m_pItemsArray.size(); i++)
	if (m_pItemsArray[i]->itemId == id)
		return m_pItemsArray[i];
	return NULL;
}

SInventoryItem* CRPGInventory::GetInventoryItemByName(const char* name)
{
	for (int i = 0; i<m_pItemsArray.size(); i++)
	if (!strcmp(name, m_pItemsArray[i]->name))
		return m_pItemsArray[i];
	return NULL;
}

int CRPGInventory::GetItemIndex(EntityId itemId)
{
	for (int i = 0; i < m_pItemsArray.size(); i++)
	if (itemId == m_pItemsArray[i]->itemId)
		return i;
	return -1;
}

int CRPGInventory::GetItemIndex(const char* name)
{
	for (int i = 0; i < m_pItemsArray.size(); i++)
	if (!strcmp(name, m_pItemsArray[i]->name))
		return i;
	return -1;
}

void CRPGInventory::OnUse(EntityId id)
{
	SEntitySpawnParams spawn;
	IEntity *pEntity = gEnv->pEntitySystem->GetEntity(id);
	if (pEntity == NULL)
		return;

	const char* className = pEntity->GetClass()->GetName();
	IGameObjectExtension *pGO = g_pGame->GetIGameFramework()->GetGameObject(id)->QueryExtension(className);
	if (!pEntity)
		return;

	if (!strcmp(className, "FirePlace"))
	{
		CFirePlace *pFire = static_cast<CFirePlace*>(pGO);
		m_pUIInventory->SetVisible(false);
		g_pGameActions->FilterNoMove()->Enable(false);
		g_pGameActions->FilterNoMouse()->Enable(false);
		pFire->OnUse();
	}

	if (!strcmp(className, "BasicFood"))
	{
		CBasicFood *pFood = static_cast<CBasicFood*>(pGO);
		pFood->OnUse();
	}
}

void CRPGInventory::OnAction(const ActionId& action, int activationMode, float value)
{
	const CGameActions &actions = g_pGame->Actions();
	if (actions.inventory == action)
	{
		if (gEnv->pFlashUI)
		{
			if (IUIElement* pUIInventory = gEnv->pFlashUI->GetUIElement(UI_NAME))
			{
				//Закомменчено из-за ассерта, о двойной регистрации слушателя
				//pUIInventory->AddEventListener(this, "CRPGInventory");
				if (!pUIInventory->IsVisible())
				{
					//Скрываем прочий худ
					g_UIVisibleManager.HideAllUIElements();

					SActionEvent event(eAE_inGame);
					OnActionEvent(event);

					IRenderer* pRenderer = gEnv->pRenderer;
					CRY_ASSERT(pRenderer);
					pRenderer->EF_SetPostEffectParam("Dof_User_Active", 1.f, true);
					pRenderer->EF_SetPostEffectParam("Dof_User_FocusDistance", 0, true);
					pRenderer->EF_SetPostEffectParam("Dof_User_FocusRange", 0, true);
					pRenderer->EF_SetPostEffectParam("Dof_User_BlurAmount", 10000, true);
					pRenderer->EF_SetPostEffectParam("Dof_User_ScaleCoc", 1000, true);

					if (ICVar* myVar = gEnv->pConsole->GetCVar("t_scale"))
						myVar->ForceSet("0.1");

					pUIInventory->SetVisible(true);
					g_pGameActions->FilterNoMove()->Enable(true);
					g_pGameActions->FilterNoMouse()->Enable(true);
				}
				else
				{
					g_UIVisibleManager.ShowHidenUIElements();
					pUIInventory->SetVisible(false);
					g_pGameActions->FilterNoMove()->Enable(false);
					g_pGameActions->FilterNoMouse()->Enable(false);
					pUIInventory->Unload();
					pUIInventory->RemoveEventListener(this);

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
		}
	}

	if (actions.drop == action)
	{
		CPlayer *pPlayer = (CPlayer*)g_pGame->GetIGameFramework()->GetClientActor();
		if (!pPlayer)
			return;

		IItem *pItem = pPlayer->GetCurrentItem();
		if (pItem != NULL)
		{
			OnDropItem(pItem->GetEntityId(), 1.0);
		}
	}

	if (actions.tringle == action)
	{
	}

	//if (actions.use == action && activationMode == 1)
	//{
	//	IEntity *pEntity = CGetMouseEntity::GetMouseEntity(USABLE_DISTANT);
	//	CRPGInventoryManager* pInventoryManager = g_pGame->GetRPGInventoryManager();
	//	if (pEntity != NULL && pInventoryManager != NULL)
	//	{
	//		IBasicObject *pBasicObject = pInventoryManager->GetBasicObject(pEntity->GetId());
	//		if (!pBasicObject)
	//			return;
	//		pBasicObject->OnPickUp(pEntity->GetId());
	//		//pInventory = g_pGame->GetRPGInventory();
	//		//if (pInventory != NULL)
	//			//pInventory->OnPickedUp(pEntity->GetId());
	//	}

	if (actions.MoveTo == action)
	{
	}
}

//-------------------------------------------------------------------
IUIElement* CRPGInventory::GetInventoryUIInterface()
{
	if (gEnv->pFlashUI != NULL)
	{
		if (m_pUIInventory != NULL)
			return m_pUIInventory;

		IUIElement* pUIInventory = gEnv->pFlashUI->GetUIElement(UI_NAME);
		return pUIInventory;
	}
	return NULL;
}

//-------------------------------------------------------------------
void CRPGInventory::ResetUIItemsArray()
{
	//Функция делает отчистку и перезапись итемтов во флеше
	//m_pUIInventory->CallFunction("ResetItemsArray");
	if (m_pItemsArray.size() != 0)
	{
		for (int i = 0; i < m_pItemsArray.size(); i++)
		{
			if (m_pItemsArray[i]->hiden == true)
				continue;

			SUIArguments args;
			args.AddArgument((string)m_pItemsArray[i]->name);
			args.AddArgument(string(m_pItemsArray[i]->description));
			args.AddArgument(m_pItemsArray[i]->size);
			args.AddArgument(m_pItemsArray[i]->cost);
			args.AddArgument(m_pItemsArray[i]->type);
			args.AddArgument(m_pItemsArray[i]->itemId);
			GetUIElement();
			m_pUIInventory->CallFunction("CreateItem", args);
		}
	}
}