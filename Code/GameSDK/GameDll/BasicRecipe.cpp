#include "StdAfx.h"
#include "BasicRecipe.h"
#include "CraftSystem.h"
#include "RPGInventoryManager.h"
#include "RPGInventory.h"
#include "Game.h"
#include "Globals.h"

#define DEFAULT_RECIPE_PATH "Objects/misc/Goods/Recipe/recipe.cgf"

CBasicRecipe::CBasicRecipe()
{
	m_EntityType = "BasicRecipe";
	m_ItemId = 0;
	m_ModelPath = "";
	m_nCost = 0;
	m_nSize = EIIS_1x1;
	m_ObjectDescr = "";
	m_ObjectName = "";
	m_sRecipeName = "";
}

bool CBasicRecipe::Init(IGameObject * pGameObject)
{
	SetGameObject(pGameObject);
	return true;
}

void CBasicRecipe::PostInit(IGameObject * pGameObject)
{
	GetEntity()->LoadGeometry(0, DEFAULT_RECIPE_PATH);
	pGameObject->EnableUpdateSlot(this, 0);
	m_ItemId = GetEntityId();
	Reset();
}

void CBasicRecipe::Update(SEntityUpdateContext& ctx, int updateSlot)
{
}

void CBasicRecipe::ProcessEvent(SEntityEvent& event)
{
	switch (event.event)
	{
		case ENTITY_EVENT_RESET: Reset(); break;
		case ENTITY_EVENT_EDITOR_PROPERTY_CHANGED: Reset(); break;
		case ENTITY_EVENT_START_GAME:
		{
			CRPGInventoryManager *pManager = g_pGame->GetRPGInventoryManager();
			if (!pManager) return;
			m_ItemId = GetEntityId();
			pManager->AddObjects(this);
		}break;
	};
}

void CBasicRecipe::OnUse()
{
}

void CBasicRecipe::Release()
{
	delete this;
}

//Ресет всех параметров, а так же обновление данных после их изменения
void CBasicRecipe::Reset()
{
	SmartScriptTable propertiesTable = GetSmartScriptTable();
	if (!propertiesTable) return;

	const char* modelPath = NULL;
	const char* recipeName = NULL;

	propertiesTable->GetValue("objModel", modelPath);
	propertiesTable->GetValue("sRecipeName", recipeName);

	m_ItemId = GetEntityId();
	m_EntityType = GetEntity()->GetClass()->GetName();
	m_ModelPath = modelPath;
	m_ObjectName = recipeName;

	GetEntity()->FreeSlot(0);
	GetEntity()->LoadGeometry(0, modelPath);
	Physicalize(PE_RIGID);
}

void CBasicRecipe::Physicalize(int type)
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

void CBasicRecipe::FullSerialize(TSerialize ser)
{
	ser.BeginGroup("BasicRecipe");
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

SInventoryItem* CBasicRecipe::GetItemParamsXML()
{
	SInventoryItem *item = new SInventoryItem;

	XmlNodeRef ItemsDescriptionFile = gEnv->pSystem->LoadXmlFromFile(PathUtil::GetGameFolder() + __ITEMS_FOLDER__);
	if (!ItemsDescriptionFile || !item)
		return 0;

	for (int i = 0; i<ItemsDescriptionFile->getChildCount(); i++)
	{
		XmlNodeRef cItems = ItemsDescriptionFile->getChild(i);
		if (cItems)
		{
			const char *className = cItems->getAttr("class");
			int _type; cItems->getAttr("type", _type);
			XmlNodeRef cParams = cItems->findChild("param");
			if (cParams && !strcmp(className, "BasicRecipe") && _type == EIT_Recipe)
			{
				for (int j = 0; j<cItems->getChildCount(); j++)
				{			
					if (XmlNodeRef cRecipeParams = cItems->getChild(j))
					{
						const char* itemName = cRecipeParams->getAttr("name");
						if (!strcmp(itemName, m_ObjectName))
						{
							item->name = itemName;
							item->description = cRecipeParams->getAttr("descr");
							cRecipeParams->getAttr("size", item->size);
							cRecipeParams->getAttr("cost", item->cost);
							item->type = _type;
							return item;
						}
					}
				}
			}
		}
	}
	return item;
}

SmartScriptTable CBasicRecipe::GetSmartScriptTable()
{
	IScriptTable *pScriptTable = GetEntity()->GetScriptTable();
	if (!pScriptTable) return 0;

	SmartScriptTable propertiesTable;
	if (!pScriptTable->GetValue("Properties", propertiesTable)) return 0;
	else return propertiesTable;
}

void CBasicRecipe::OnPickUp(EntityId id)
{
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
	pUIItem->type = EIT_Recipe;	
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

void CBasicRecipe::OnDrop(SInventoryItem *pItem, EntityId id)
{
	CRPGInventoryManager *pInventoryManager = g_pGame->GetRPGInventoryManager();
	IEntity *pEntity = gEnv->pEntitySystem->GetEntity(id);
	CBasicRecipe *pBasicRecipe = static_cast<CBasicRecipe*>(pInventoryManager->GetBasicObject(id));
	if (!pBasicRecipe || !pEntity || !pItem) return;

	//pInventoryManager->DeleteObject(GetEntityId());
	pBasicRecipe->m_ItemId = id;
	pBasicRecipe->m_ModelPath = pItem->objModel;
	pBasicRecipe->m_nCost = pItem->cost;
	pBasicRecipe->m_nSize = pItem->size;
	pBasicRecipe->m_ObjectName = pItem->name;
	pBasicRecipe->m_sRecipeName = pItem->name;
	pBasicRecipe->m_ObjectDescr = pItem->description;

	IScriptTable *pScriptTable = pEntity->GetScriptTable();
	if (!pScriptTable) return;

	SmartScriptTable prop;
	if (pScriptTable->GetValue("Properties", prop))
	{
		prop->SetValue("sRecipeName", (const char*)pBasicRecipe->m_ObjectName);
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
}
