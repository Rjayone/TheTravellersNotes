#include "StdAfx.h"
#include "BasicShield.h"
#include "Globals.h"
#include "RPGInventoryManager.h"
#include "RPGInventory.h"

#define DEFAULT_SHIELD_PATH ""


CBasicShield::CBasicShield()
{
	m_EntityType = "BasicShield";
	m_ItemId = 0;
	m_ModelPath = "";
	m_nCost = 0;
	m_nSize = EIIS_2x2;
	m_ObjectDescr = "";
	m_ObjectName = "";
	m_nWearout = 0;
}

bool CBasicShield::Init(IGameObject * pGameObject)
{
	SetGameObject(pGameObject);
	return true;
}

void CBasicShield::PostInit(IGameObject * pGameObject)
{
	GetEntity()->LoadGeometry(0, DEFAULT_SHIELD_PATH);
	pGameObject->EnableUpdateSlot(this, 0);
	m_ItemId = GetEntityId();
	Reset();
}

void CBasicShield::Update(SEntityUpdateContext& ctx, int updateSlot)
{
}

void CBasicShield::ProcessEvent(SEntityEvent& event)
{
	switch (event.event)
	{
		case ENTITY_EVENT_RELOAD_SCRIPT: Reset(); break;
		case ENTITY_EVENT_EDITOR_PROPERTY_CHANGED:  Reset(); break;
		case ENTITY_EVENT_START_GAME:
		{
			CRPGInventoryManager *pManager = g_pGame->GetRPGInventoryManager();
			if (!pManager) return;
			m_ItemId = GetEntityId();
			pManager->AddObjects(this);
		}break;
		case ENTITY_EVENT_SCRIPT_EVENT:
		{
			 const char* eventName = NULL;
			 eventName = reinterpret_cast<const char*>(event.nParam[0]);
			 if (eventName && !strcmp(eventName, "OnPropertyChange"))
			 {
				  Reset();
			 }
		}break;
	};
}

void CBasicShield::OnUse()
{
}

void CBasicShield::Release()
{
	delete this;
}

//Ресет всех параметров, а так же обновление данных после их изменения
void CBasicShield::Reset()
{
	SmartScriptTable propertiesTable = GetSmartScriptTable();
	if (!propertiesTable) return;

	const char* modelPath = NULL;
	const char* itemName = NULL;
	int wearout = 0;

	propertiesTable->GetValue("objModel", modelPath);
	propertiesTable->GetValue("sItemName", itemName);
	propertiesTable->GetValue("nWearout", wearout);

	if (wearout > 30 && wearout <= 60)
	{
		if (modelPath[strlen(modelPath) - 6] == '_' && modelPath[strlen(modelPath) - 5] == '2');
		else
		{
			((char*)modelPath)[strlen(modelPath) - 4] = '\0';
			strcat((char*)modelPath, "_2.cgf");
		}
	}
	if (wearout > 60 && wearout <= 99)
	{
		if (modelPath[strlen(modelPath) - 6] == '_' && modelPath[strlen(modelPath) - 5] == '3');
		else
		{
			((char*)modelPath)[strlen(modelPath) - 4] = '\0';
			strcat((char*)modelPath, "_3.cgf");
		}
	}

	m_ItemId = GetEntityId();
	m_EntityType = GetEntity()->GetClass()->GetName();
	m_ModelPath = modelPath;
	m_ObjectName = itemName;
	m_nWearout = wearout;

	GetEntity()->FreeSlot(0);
	GetEntity()->LoadGeometry(0, modelPath);
	Physicalize(PE_RIGID);
}

void CBasicShield::Physicalize(int type)
{
	IEntity *pEntity = GetEntity();
	if (pEntity == NULL) return;

	float mass = 1;
	SmartScriptTable propertiesTable = GetSmartScriptTable();
	if (!propertiesTable->GetValue("fMass", mass))
		CryLogAlways("[CBasicShield]: No such field as fMass");

	SEntityPhysicalizeParams pp;
	pp.type = type;
	pp.nSlot = -1;
	pp.mass = mass;
	pp.nFlagsOR = pef_monitor_poststep;
	pp.fStiffnessScale = mass * 3;

	pEntity->Physicalize(pp);
}

void CBasicShield::FullSerialize(TSerialize ser)
{
	ser.BeginGroup("BasicShield");
	if (ser.IsWriting())
	{
		ser.Value("m_EntityType", m_EntityType);
		ser.Value("m_ItemDescr", m_ObjectDescr);
		ser.Value("m_ItemId", m_ItemId);
		ser.Value("m_ItemName", m_ObjectName);
		ser.Value("m_ModelPath", m_ModelPath);
		ser.Value("m_nSize", m_nSize);
		ser.Value("m_nWearout", m_nWearout);
	}
	else
	{
		ser.Value("m_EntityType", m_EntityType);
		ser.Value("m_ItemDescr", m_ObjectDescr);
		ser.Value("m_ItemId", m_ItemId);
		ser.Value("m_ItemName", m_ObjectName);
		ser.Value("m_ModelPath", m_ModelPath);
		ser.Value("m_nSize", m_nSize);
		ser.Value("m_nWearout", m_nWearout);
	}
	ser.EndGroup();
}

SInventoryItem* CBasicShield::GetItemParamsXML()
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
			if (cParams && !strcmp(className, "BasicShield") && _type == EIT_Shield)
			{
				for (int j = 0; j<cItems->getChildCount(); j++)
				{			
					if (XmlNodeRef cShieldParams = cItems->getChild(j))
					{
						const char* itemName = cShieldParams->getAttr("name");
						if (!strcmp(itemName, m_ObjectName))
						{
							item->name = itemName;
							item->description = cShieldParams->getAttr("descr");
							cShieldParams->getAttr("size", item->size);
							cShieldParams->getAttr("cost", item->cost);
							cShieldParams->getAttr("hiden", item->hiden);
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

SmartScriptTable CBasicShield::GetSmartScriptTable()
{
	IScriptTable *pScriptTable = GetEntity()->GetScriptTable();
	if (!pScriptTable) return 0;

	SmartScriptTable propertiesTable;
	if (!pScriptTable->GetValue("Properties", propertiesTable)) return 0;
	else return propertiesTable;
}

void CBasicShield::OnPickUp(EntityId id)
{
	CRPGInventory* pInventory = g_pGame->GetRPGInventory();
	CRPGInventoryManager *pInventoryManager = g_pGame->GetRPGInventoryManager();
	SmartScriptTable prop = GetSmartScriptTable();
	if (!pInventory || !prop || !pInventoryManager)
		return;

	if (pInventory->m_pItemsArray.size() > pInventory->GetMaxSlotsCount())	//Если инвентарь полный
	{
		pe_action_impulse action;
		IPhysicalEntity *pPhysEntity = GetEntity()->GetPhysics();
		action.impulse.Set(0, 0, 1);
		action.angImpulse.Set(1, 1, 0);
		if (pPhysEntity != NULL) pPhysEntity->Action(&action);
		return;
	}

	bool usable = false;
	char* path = NULL;
	int wearout = 0;
	prop->GetValue("bItemUsable", usable);
	prop->GetValue("objModel", path);
	prop->GetValue("nWearout", wearout);
	if (usable == false) return;

	SInventoryItem *pUIItem = GetItemParamsXML();
	pUIItem->objModel = path;
	pUIItem->itemId = GetEntityId();
	pUIItem->entityClassType = GetEntity()->GetClass()->GetName();
	pUIItem->type = EIT_Shield;
	pUIItem->wearout = wearout;
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

void CBasicShield::OnDrop(SInventoryItem *pItem, EntityId id)
{
	CRPGInventoryManager *pInventoryManager = g_pGame->GetRPGInventoryManager();
	IEntity *pEntity = gEnv->pEntitySystem->GetEntity(id);
	CBasicShield *pBasicShield = static_cast<CBasicShield*>(pInventoryManager->GetBasicObject(id));
	if (!pBasicShield || !pEntity || !pItem) return;

	pBasicShield->m_ItemId = id;
	pBasicShield->m_ModelPath = pItem->objModel;
	pBasicShield->m_nCost = pItem->cost;
	pBasicShield->m_nSize = pItem->size;
	pBasicShield->m_ObjectName = pItem->name;
	pBasicShield->m_ObjectDescr = pItem->description;
	pBasicShield->m_nWearout = pItem->wearout;

	IScriptTable *pScriptTable = pEntity->GetScriptTable();
	if (!pScriptTable) return;

	SmartScriptTable prop;
	if (pScriptTable->GetValue("Properties", prop))
	{
		prop->SetValue("sItemName", (const char*)pBasicShield->m_ObjectName);
		prop->SetValue("bItemUsable", true);
		prop->SetValue("objModel", (const char*)pBasicShield->m_ModelPath);
		prop->SetValue("nItemType", (int)EIT_Other);
		prop->SetValue("nWearout", pBasicShield->m_nWearout);
	}
	pBasicShield->Reset();

	//Придаем импульс
	pe_action_impulse action;
	IPhysicalEntity *pPhysEntity = pEntity->GetPhysics();
	action.angImpulse.Set(Random(0, 1), Random(0, 1), Random(0, 1));
	if (pPhysEntity != NULL) pPhysEntity->Action(&action);
}

bool CBasicShield::ReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params)
{
	ResetGameObject();
	return true;
}

void CBasicShield::HandleEvent(const SGameObjectEvent& event)
{
}