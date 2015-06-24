/****************************************
* File: BasicFood.h
* Description: Игровой объект - еда.
* Created by: Andrew Medvedev
* Date: 04.09.2013
* Diko Source File
* ToDo: реализовать фласки
*****************************************/

#include "StdAfx.h"
#include "BasicFood.h"
#include "RPGInventoryManager.h"
#include "RPGInventory.h"
#include "CharDevSys.h"
#include "Game.h"
#include "ParticleParams.h"

#define FILE_PATH "Game/Objects/misc/food/apple.cgf"
#define ITEMS_FOLDER "/Libs/Items/Library/ItemsDescription.xml"

//#define SPOILED_FOOD_PARTICLE "environment_fx.environment.flies.insects_flies_highQ"

CBasicFood::CBasicFood()
{
	m_EntityType = "BasicFood";
	m_ItemId = 0;
	m_ModelPath = "";
	m_nCost = 0;
	m_nSize = EIIS_1x1;
	m_ObjectDescr = "";
	m_ObjectName = "";	
}

//--------------------------------------
bool CBasicFood::Init(IGameObject *pGameObject)
{
	SetGameObject(pGameObject);
	m_ItemId = GetEntityId();

	return true;
}

//--------------------------------------
void CBasicFood::PostInit(IGameObject * pGameObject)
{
	GetEntity()->LoadGeometry(0, FILE_PATH);
	pGameObject->EnableUpdateSlot(this, 0);
	
	Reset();
}

//--------------------------------------
void CBasicFood::ProcessEvent(SEntityEvent& event)
{
	switch (event.event)
	{
		case ENTITY_EVENT_RESET:
			Reset();
			break;
		case ENTITY_EVENT_SCRIPT_EVENT:
		{
			const char* eventName = NULL;
			eventName = reinterpret_cast< const char* >(event.nParam[0]);
			if (eventName && !strcmp(eventName, "Reset")) //Не инвентарь
			{
				Reset();
			}
		}break;
		case ENTITY_EVENT_START_GAME:
		{
			CRPGInventoryManager *pManager = g_pGame->GetRPGInventoryManager();
			if (!pManager) return;
			m_ItemId = GetEntityId();
			pManager->AddObjects(this);
			InitSpoilParams();
		}break;
	};
}

//--------------------------------------
void CBasicFood::Update(SEntityUpdateContext& ctx, int updateSlot)
{
	auto timer = gEnv->pTimer->GetCurrTime();
	auto isIngame = gEnv->IsEditorGameMode();
	if (isIngame && startSpoilsTimer + endSpoilTime <= timer)
	{
		SetSpoiledFood();
	}
}

void CBasicFood::Reset()
{
	SmartScriptTable propertiesTable = GetSmartScriptTable();
	if (!propertiesTable) return;

	const char* modelPath = NULL;
	const char* itemName = NULL;

	propertiesTable->GetValue("objModel", modelPath);
	propertiesTable->GetValue("sItemName", itemName);
	propertiesTable->GetValue("spoilParticle", spoiledParticle);
	propertiesTable->GetValue("fEndSpoilTime", endSpoilTime);

	m_ItemId = GetEntityId();
	m_EntityType = GetEntity()->GetClass()->GetName();
	m_ModelPath = modelPath;
	m_ObjectName = itemName;

	GetEntity()->FreeSlot(0);
	GetEntity()->LoadGeometry(0, modelPath);
	Physicalize(PE_RIGID);
}

void CBasicFood::OnUse()
{
	SmartScriptTable propertiesTable = GetSmartScriptTable();
	if (!propertiesTable)
			return;

	int foodType = -1;
	propertiesTable->GetValue("type", foodType);

	CCharacterDevelopmentSystem *pCharDevSys = g_pGame->GetCharDevSys();
	if (pCharDevSys)  pCharDevSys->ChangeHungryStance(foodType);
}

SInventoryItem* CBasicFood::GetItemParamsXML()
{
	SInventoryItem *item = new SInventoryItem;

	XmlNodeRef ItemsDescriptionFile = gEnv->pSystem->LoadXmlFromFile(PathUtil::GetGameFolder() + ITEMS_FOLDER);
	if (!ItemsDescriptionFile || !item)
		return 0;

	for (int i = 0; i<ItemsDescriptionFile->getChildCount(); i++)
	{
		XmlNodeRef cItems = ItemsDescriptionFile->getChild(i);
		if (cItems)
		{
			const char *className = cItems->getAttr("class");
			XmlNodeRef cParams = cItems->findChild("param");
			if (cParams && !strcmp(className, "BasicFood"))
			{
				if (cItems->getChildCount() > 0)
				{
					for (int j = 0; j<cItems->getChildCount(); j++)
					{
						XmlNodeRef cItemParams = cItems->getChild(j);
						if (cItemParams)
						{
							const char* itemName = cItemParams->getAttr("name");
							if (!strcmp(itemName, m_ObjectName))
							{
								item->name = itemName;
								item->description = cParams->getAttr("descr");
								cItemParams->getAttr("size", item->size);
								cItemParams->getAttr("cost", item->cost);
								cItemParams->getAttr("type", item->type);
								return item;
							}
						}
					}
				}
			}
		}
	}
	return item;
}

SmartScriptTable CBasicFood::GetSmartScriptTable()
{
	IScriptTable *pScriptTable = GetEntity()->GetScriptTable();
	if (!pScriptTable) return 0;

	SmartScriptTable propertiesTable;
	if (!pScriptTable->GetValue("Properties", propertiesTable)) return 0;
	else return propertiesTable;
}

void CBasicFood::Physicalize(int type)
{
	IEntity *pEntity = GetEntity();
	if (pEntity == NULL) return;

	float mass = 1;
	SmartScriptTable propertiesTable = GetSmartScriptTable();
	propertiesTable->GetValue("fMass", mass);

	SEntityPhysicalizeParams pp;
	pp.type = type;
	pp.nSlot = -1;
	pp.mass = mass;
	pp.nFlagsOR = pef_monitor_poststep;
	pp.fStiffnessScale = 5;

	pEntity->Physicalize(pp);
}

void CBasicFood::OnPickUp(EntityId id)
{
	//как работает: В инит регистрируем объект в менеджере инв. объектов 
	//при срабатывании евента в ActionListener мы обращаемся к менеджеру инв. обектов и ищем зарегистрирован ли там такой
	//Если все окей то получаем на этот объект указатель(IBasicObject) и вызываем данную функцию
	CRPGInventory* pInventory = g_pGame->GetRPGInventory();
	CRPGInventoryManager *pInventoryManager = g_pGame->GetRPGInventoryManager();
	SmartScriptTable prop = GetSmartScriptTable();
	if (!pInventory || !prop || !pInventoryManager)
		return;

	if (pInventory->m_pItemsArray.size() > pInventory->GetMaxSlotsCount())	//Если инвентарь полный
	{
		pe_action_impulse action; // структура описания импульса
		action.impulse.Set(0, 0, 1); // сила импульса, вроде
		action.angImpulse.Set(1, 1, 0);// импульс вращения

		IPhysicalEntity *pPhysEntity = GetEntity()->GetPhysics(); //получаем физику объекта
		if (pPhysEntity != NULL) pPhysEntity->Action(&action); // выполняем импульс
		return;
	}

	bool usable = false;
	char* path = NULL;
	prop->GetValue("bItemUsable", usable);
	prop->GetValue("objModel", path);
	if (usable == false) return;

	SInventoryItem *pUIItem = GetItemParamsXML();
	pUIItem->objModel = path;
	pUIItem->itemId = GetEntityId();
	pUIItem->entityClassType = GetEntity()->GetClass()->GetName();
	pUIItem->type = EIT_Food;
	pInventory->AddItem(pUIItem);
	gEnv->pEntitySystem->RemoveEntity(id, true);
	pInventoryManager->DeleteObject(GetEntityId());


	//IActor *pActor = g_pGame->GetIGameFramework()->GetClientActor();
	//_smart_ptr<ISound> pSound = gEnv->pSoundSystem->CreateSound("sounds/weapons:scar:scar_hand_on_gun_fp", FLAG_SOUND_DEFAULT_3D);
	//if (pSound && pActor)
	//{
	//	pSound->SetPosition(pActor->GetEntity()->GetPos());
	//	pSound->Play();
	//}
}

void CBasicFood::OnDrop(SInventoryItem *pItem, EntityId id)
{
	CRPGInventoryManager *pInventoryManager = g_pGame->GetRPGInventoryManager();
	IEntity *pEntity = gEnv->pEntitySystem->GetEntity(id);
	CBasicFood *pBasicRecipe = static_cast<CBasicFood*>(pInventoryManager->GetBasicObject(id));
	if (!pBasicRecipe || !pEntity || !pItem) return;

	//pInventoryManager->DeleteObject(GetEntityId());
	pBasicRecipe->m_ItemId = id;
	pBasicRecipe->m_ModelPath = pItem->objModel;
	pBasicRecipe->m_nCost = pItem->cost;
	pBasicRecipe->m_nSize = pItem->size;
	pBasicRecipe->m_ObjectName = pItem->name;
	pBasicRecipe->m_ObjectDescr = pItem->description;

	IScriptTable *pScriptTable = pEntity->GetScriptTable();
	if (!pScriptTable) return;

	SmartScriptTable prop;
	if (pScriptTable->GetValue("Properties", prop))
	{
		prop->SetValue("bItemUsable", true);
		prop->SetValue("objModel", (const char*)pBasicRecipe->m_ModelPath);
		prop->SetValue("nItemType", (int)EIT_Other);
	}
	pBasicRecipe->Reset();

	//Придаем импульс
	pe_action_impulse action;
	IPhysicalEntity *pPhysEntity = pEntity->GetPhysics();
	action.angImpulse.Set(Random(0, 1), Random(0, 1), Random(0, 1));
	if (pPhysEntity != NULL) pPhysEntity->Action(&action);
	InitSpoilParams();
}

void CBasicFood::FullSerialize(TSerialize ser)
{
	ser.BeginGroup("BasicMoney");
	if (ser.IsWriting())
	{
		ser.Value("m_EntityType", m_EntityType);
		ser.Value("m_ItemDescr", m_ObjectDescr);
		ser.Value("m_ItemId", m_ItemId);
		ser.Value("m_ItemName", m_ObjectName);
		ser.Value("m_ModelPath", m_ModelPath);
		ser.Value("m_nSize", m_nSize);
	}
	else
	{
		ser.Value("m_EntityType", m_EntityType);
		ser.Value("m_ItemDescr", m_ObjectDescr);
		ser.Value("m_ItemId", m_ItemId);
		ser.Value("m_ItemName", m_ObjectName);
		ser.Value("m_ModelPath", m_ModelPath);
		ser.Value("m_nSize", m_nSize);
	}
	ser.EndGroup();
}

void CBasicFood::Release()
{
	delete this;
}

bool CBasicFood::ReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params)
{
	ResetGameObject();
	return true;
}

void CBasicFood::SetSpoiledFood()
{
	if (isSpoiled) return;
	

	IParticleEffect* pEffect = gEnv->pParticleManager->FindEffect(spoiledParticle);
	if (pEffect)
	{
		GetEntity()->LoadParticleEmitter(5, pEffect);
		isSpoiled = true;
	}
}

void CBasicFood::InitSpoilParams()
{
	isSpoiled = false;
	startSpoilsTimer = gEnv->pTimer->GetCurrTime();
}

