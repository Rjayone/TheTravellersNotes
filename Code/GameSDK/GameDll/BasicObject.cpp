#include "StdAfx.h"
#include "BasicObject.h"
#include "RPGInventoryManager.h"
#include "RPGInventory.h"

#define DEFAULT_MODEL_PATH "GameSDK/Objects/box.cgf"

CBasicObject::CBasicObject()
{
	m_EntityType = "BasicObject";
	m_ItemId = 0;
	m_ModelPath = "";
	m_nCost = 0;
	m_nSize = EIIS_1x1;
	m_ObjectDescr = "";
	m_ObjectName = "";
}

bool CBasicObject::Init(IGameObject * pGameObject)
{
	SetGameObject(pGameObject);
	return true;
}

void CBasicObject::PostInit(IGameObject * pGameObject)
{
	GetEntity()->LoadGeometry(0, DEFAULT_MODEL_PATH);
	pGameObject->EnableUpdateSlot(this, 0);
	m_ItemId = GetEntityId();
	Reset();
}

void CBasicObject::Release()
{
	delete this;
}

void CBasicObject::FullSerialize(TSerialize ser)
{
	ser.BeginGroup("BasicObject");
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

void CBasicObject::Update(SEntityUpdateContext& ctx, int updateSlot)
{
}

void CBasicObject::HandleEvent(const SGameObjectEvent& event)
{
}

void CBasicObject::ProcessEvent(SEntityEvent& event)
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
		} break;		
	};
}

void CBasicObject::OnUse()
{
}

void CBasicObject::OnPickUp(EntityId id)
{
	CRPGInventory *pInventory = g_pGame->GetRPGInventory();
	SInventoryItem *pUIItem = new SInventoryItem();
	SmartScriptTable prop = GetSmartScriptTable();
	CRPGInventoryManager *pInventoryManager = g_pGame->GetRPGInventoryManager();
	if (!pInventory || !pUIItem || !prop || !pInventoryManager)
		return;
	
	if (pInventory->m_pItemsArray.size() > pInventory->GetMaxSlotsCount())//Если инвентарь полный
	{
		pe_action_impulse action;
		action.impulse.Set(Random(0, 0.5), Random(0, 0.5), Random(0, 1));
		action.angImpulse.Set(Random(0, 0.5), Random(0, 0.5), Random(0, 0.5));

		IPhysicalEntity *pPhysEntity = GetEntity()->GetPhysics();
		if (pPhysEntity) pPhysEntity->Action(&action); 
		return;
	}
	
	bool usable = false;
	char* itemName = NULL;
	prop->GetValue("bItemUsable", usable);
	prop->GetValue("objModel", itemName);
	if (usable == false) return;

	pUIItem = GetItemParamsXML();
	pUIItem->objModel = itemName;
	pUIItem->itemId = GetEntityId();
	pUIItem->entityClassType = GetEntity()->GetClass()->GetName();
	pUIItem->type = EIT_Other;
	pInventory->AddItem(pUIItem);
	gEnv->pEntitySystem->RemoveEntity(id, true);
	pInventoryManager->DeleteObject(GetEntityId());

	//Задаем звук проигрывания
	//IActor *pActor = g_pGame->GetIGameFramework()->GetClientActor();
	//_smart_ptr<ISound> pSound = gEnv->pSoundSystem->CreateSound("sounds/weapons:scar:scar_hand_on_gun_fp", FLAG_SOUND_DEFAULT_3D);
	//if (pSound && pActor)
	//{
	//	pSound->SetPosition(pActor->GetEntity()->GetPos());
	//	pSound->Play();
	//}
}

void CBasicObject::OnDrop(SInventoryItem* pItem, EntityId id)
{
	CRPGInventoryManager *pInventoryManager = g_pGame->GetRPGInventoryManager();
	IEntity *pEntity = gEnv->pEntitySystem->GetEntity(id);
	CBasicObject *pBasicObject = static_cast<CBasicObject*>(pInventoryManager->GetBasicObject(id));
	if (!pBasicObject || !pEntity || !pItem) return;

	//pInventoryManager->DeleteObject(GetEntityId());
	pBasicObject->m_ItemId = id;
	pBasicObject->m_ModelPath = pItem->objModel;
	pBasicObject->m_nCost = pItem->cost;
	pBasicObject->m_nSize = pItem->size;
	pBasicObject->m_ObjectName = pItem->name;
	pBasicObject->m_ObjectDescr = pItem->description;

	IScriptTable *pScriptTable = pEntity->GetScriptTable();
	if (!pScriptTable) return;

	SmartScriptTable prop;
	if (pScriptTable->GetValue("Properties", prop))
	{
		prop->SetValue("sItemName", (const char*)pBasicObject->m_ObjectName);
		prop->SetValue("bItemUsable", true);
		prop->SetValue("objModel", (const char*)pBasicObject->m_ModelPath);
		prop->SetValue("nItemType", (int)EIT_Other);
	}
	pBasicObject->Reset();

	//Придаем импульс
	pe_action_impulse action;
	IPhysicalEntity *pPhysEntity = pEntity->GetPhysics();
	action.angImpulse.Set(Random(0, 1), Random(0, 1), Random(0, 1));
	action.impulse = /*gEnv->pSystem->GetViewCamera().GetPosition() + */gEnv->pSystem->GetViewCamera().GetViewdir()*2 + Vec3(0, 0, 1);
	if (pPhysEntity != NULL) pPhysEntity->Action(&action);
}

void CBasicObject::Reset()
{
	SmartScriptTable prop = GetSmartScriptTable();
	char* name = NULL;
	char* model = NULL;
	prop->GetValue("objModel", model);
	prop->GetValue("sItemName", name);
	
	m_ModelPath = model;
	m_ObjectName = name;
	m_EntityType = "BasicObject";

	GetEntity()->FreeSlot(0);
	GetEntity()->LoadGeometry(0, m_ModelPath);
	Physicalize(PE_RIGID);
}

void CBasicObject::Physicalize(int type)
{
	IEntity *pEntity = GetEntity();
	if (pEntity != NULL)
	{
		float mass = 1;
		SmartScriptTable propertiesTable = GetSmartScriptTable();
		propertiesTable->GetValue("fMass", mass);

		SEntityPhysicalizeParams pp;
		pp.type = type;
		pp.nSlot = -1;
		pp.mass = mass;
		pp.nFlagsOR = pef_monitor_poststep;
		pp.fStiffnessScale = mass;
		pEntity->Physicalize(pp);
	}
}

bool CBasicObject::ReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params)
{
	ResetGameObject();
	return true;
}

SmartScriptTable CBasicObject::GetSmartScriptTable()
{
	IScriptTable *pScriptTable = GetEntity()->GetScriptTable();
	if (!pScriptTable) return 0;

	SmartScriptTable propertiesTable;
	if (!pScriptTable->GetValue("Properties", propertiesTable)) return 0;
	else return propertiesTable;
}

SInventoryItem* CBasicObject::GetItemParamsXML()
{
	SInventoryItem *item = new SInventoryItem();
	XmlNodeRef ItemsDescriptionFile = gEnv->pSystem->LoadXmlFromFile(PathUtil::GetGameFolder() + "/Libs/Items/Library/ItemsDescription.xml");
	if (ItemsDescriptionFile == NULL || item == NULL)
		return NULL;

	for (int i = 0; i<ItemsDescriptionFile->getChildCount(); i++)
	{
		XmlNodeRef cItems = ItemsDescriptionFile->getChild(i);//<item ... />
		if (cItems != NULL)
		{
			const char *className = cItems->getAttr("class");
			int _type = atoi(cItems->getAttr("type"));
			if (_type == EIT_Other)//<item class="Other" type="8">...
			{
				for (int j = 0; j < cItems->getChildCount(); j++)
				{
					XmlNodeRef cParams = cItems->getChild(j);//<param name.../>
					if (cParams != NULL)
					{
						const char* itemName = cParams->getAttr("name");
						if (!strcmp(itemName, m_ObjectName))
						{
							item->name = itemName;
							item->description = cParams->getAttr("descr");
							cParams->getAttr("size", item->size);
							cParams->getAttr("cost", item->cost);
							cParams->getAttr("type", item->type);
							return item;
						}
					}
				}
			}
		}
	}
	return item;
}

void CBasicObject::UpdateParams()
{}