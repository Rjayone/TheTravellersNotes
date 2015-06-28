/****************************************************************************************************************************
* File: BasicMoney.h/cpp

* Description: Базовый игровой инвентарный объект - деньги. Описывает количество денег если подобрать, рандом-функцию и модель

* Created by: Andrew Medvedev

* Date: 18.03.2014

* Diko Source File

* ToDo: дописать считывание из хмл
*****************************************************************************************************************************/

#include "StdAfx.h"
#include "BasicMoney.h"
#include "RPGInventoryManager.h"
#include "RPGInventory.h"
#include "PlayerStatsManager.h"
#include "InventoryItems.h"

#define DEFAULT_MODEL_PATH "GameSDK/Objects/misc/money/money_single_a.cgf"
#define DEFAULT_PARTICLE "bullet.hit_metal.c"

CBasicMoney::CBasicMoney()
{
	m_fSparkTimer = 0;
	m_bSparkEnable = false;
	m_pParticalEntity = NULL;

	m_EntityType = "BasicMoney";
	m_ItemId = 0;
	m_ModelPath = "";
	m_nCost = 0;
	m_nSize = EIIS_1x1;
	m_ObjectDescr = "";
	m_ObjectName = "";
	m_pParticalEffect = "";
}

bool CBasicMoney::Init(IGameObject * pGameObject)
{
	SetGameObject(pGameObject);
	return true;
}

void CBasicMoney::PostInit(IGameObject * pGameObject)
{
	GetEntity()->LoadGeometry(0, DEFAULT_MODEL_PATH);
	pGameObject->EnableUpdateSlot(this, 0);

	//Добавим сверкание
	SmartScriptTable prop = GetSmartScriptTable();
	if (prop)
	{
		prop->GetValue("bSparkEnable", m_bSparkEnable);
		if (m_bSparkEnable == true)
			StartSpark();
	}
	Reset();
}

void CBasicMoney::Reset()
{
	const char* model = NULL;
	const char* effect = NULL;
	SmartScriptTable propertiesTable = GetSmartScriptTable();
	if (!propertiesTable) return;
	if (!propertiesTable->GetValue("objModel", model)) return;
	if (!propertiesTable->GetValue("bSparkEnable", m_bSparkEnable)) return;
	if (!propertiesTable->GetValue("sParticle", effect)) return;

	m_ModelPath = model;
	m_pParticalEffect = effect;

	GetEntity()->FreeSlot(0);
	GetEntity()->LoadGeometry(0, model);
	Physicalize(PE_RIGID);
}

void CBasicMoney::Release()
{
	delete this;
}

bool CBasicMoney::ReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params)
{
	ResetGameObject();
	return true;
}

void CBasicMoney::FullSerialize(TSerialize ser)
{
	ser.BeginGroup("BasicMoney");
	if (ser.IsWriting())
	{
		ser.Value("m_EntityType", m_EntityType);
		ser.Value("m_ItemDescr", m_ObjectDescr);
		ser.Value("m_ItemId", m_ItemId);
		ser.Value("m_ItemName", m_ObjectName);
		ser.Value("m_ModelPath", m_ModelPath);
		ser.Value("m_nCount", m_nCount);
		ser.Value("m_nSize", m_nSize);
	}
	else
	{
		ser.Value("m_EntityType", m_EntityType);
		ser.Value("m_ItemDescr", m_ObjectDescr);
		ser.Value("m_ItemId", m_ItemId);
		ser.Value("m_ItemName", m_ObjectName);
		ser.Value("m_ModelPath", m_ModelPath);
		ser.Value("m_nCount", m_nCount);
		ser.Value("m_nSize", m_nSize);
	}
	ser.EndGroup();
}

void CBasicMoney::Update(SEntityUpdateContext& ctx, int updateSlot)
{
	if (m_bSparkEnable)
	{
		if (gEnv->pTimer->GetCurrTime() - m_fSparkTimer >= 10 + Random(0, 3) && m_pParticalEntity)
		{
			//Надо проверить на производительность
			StartSpark();
			m_fSparkTimer = gEnv->pTimer->GetAsyncCurTime() + Random(0, 5);
		}
	}
}

void CBasicMoney::ProcessEvent(SEntityEvent& event)
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

			CPlayerStatsManager *pStatsManager = g_pGame->GetPlayerStatsManager();
			if (pStatsManager != NULL)
			{
				CPlayerStat *pMoney = pStatsManager->GetStat(EPS_Money);
				if (!pMoney) return;

				IUIElement *UI = gEnv->pFlashUI->GetUIElement("Money");
				SUIArguments arg; arg.AddArgument(pMoney->GetStatValue());
				UI->CallFunction("SetMoney", arg);
			}

		} break;
	};
}

void CBasicMoney::OnPickUp(EntityId id)
{
	//как работает: В инит регистрируем объект в менеджере инв. объектов 
	//при срабатывании евента в ActionListener мы обращаемся к менеджеру инв. обектов и ищем зарегистрирован ли там такой
	//Если все окей то получаем на этот объект указатель(IBasicObject) и вызываем данную функцию
	CRPGInventory *pInventory = g_pGame->GetRPGInventory();
	SInventoryItem *pUIItem = new SInventoryItem();
	SmartScriptTable prop = GetSmartScriptTable();
	CRPGInventoryManager *pInventoryManager = g_pGame->GetRPGInventoryManager();
	if (!pInventory || !pUIItem || !prop || !pInventoryManager)
		return;

	int money = 0;
	bool rand = false;
	prop->GetValue("bRandom", rand);
	prop->GetValue("nMoney", money);

	if (rand)
	{
		//Надо бы ещё размер учесть
		money = Random(Random(1, 15), Random(25, 60));
	}
	pUIItem = GetItemParamsXML();
	pUIItem->objModel = "Money";
	pUIItem->itemId = GetEntityId();
	pUIItem->entityClassType = GetEntity()->GetClass()->GetName();
	pUIItem->type = EIT_Money;
	pUIItem->hiden = true;
	pInventory->AddItem(pUIItem);
	gEnv->pEntitySystem->RemoveEntity(id, true);
	pInventoryManager->DeleteObject(GetEntityId());

	AddMoney(money);
	//IActor *pActor = g_pGame->GetIGameFramework()->GetClientActor();
	//_smart_ptr<ISound> pSound = gEnv->pSoundSystem->CreateSound("sounds/weapons:scar:scar_hand_on_gun_fp", FLAG_SOUND_DEFAULT_3D);
	//if (pSound && pActor)
	//{
	//	pSound->SetPosition(pActor->GetEntity()->GetPos());
	//	pSound->Play();
	//}
}

void CBasicMoney::OnDrop(SInventoryItem *pItem, EntityId id)
{
	CRPGInventoryManager *pInventoryManager = g_pGame->GetRPGInventoryManager();
	IEntity *pEntity = gEnv->pEntitySystem->GetEntity(id);
	CBasicMoney *pBasicObject = static_cast<CBasicMoney*>(pInventoryManager->GetBasicObject(id));
	if (!pBasicObject || !pEntity || !pItem) return;

	pBasicObject->m_ItemId = id;
	pBasicObject->m_ModelPath = pItem->objModel;
	pBasicObject->m_nCost = pItem->cost;
	pBasicObject->m_nSize = pItem->size;
	pBasicObject->m_ObjectName = pItem->name;
	pBasicObject->m_ObjectDescr = pItem->description;

	IScriptTable *pScriptTable = pEntity->GetScriptTable();
	if (!pScriptTable) return;
	SmartScriptTable prop;

	//здесь прописываем все поля которые нужно установить для нового ентити. см. SInventoryItem
	if (pScriptTable->GetValue("Properties", prop))
	{
		prop->SetValue("objModel", (const char*)m_ModelPath);
		prop->SetValue("nItemType", EIT_Money);
		prop->SetValue("bRandom", false);
		prop->SetValue("bSparkEnable", m_bSparkEnable);
		prop->SetValue("nBagSize", m_nSize);
		prop->SetValue("sParticle", (const char*)m_pParticalEffect);
		prop->SetValue("nMoney", m_nCount);
	}

	//Придаем импульс
	pe_action_impulse action;
	IPhysicalEntity *pPhysEntity = pEntity->GetPhysics();
	action.angImpulse.Set(Random(0, 1), Random(0, 1), Random(0, 1));
	if (pPhysEntity != NULL) pPhysEntity->Action(&action);
}

void CBasicMoney::Physicalize(int type)
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

void CBasicMoney::AddMoney(int value)
{
	//Так как будет реализован класс денег, и он же будет подключен в менеджере статов.
	//Класс денег будет читать инвенатрь и на основе инва будет пересчитывать деньги и выводить
	CPlayerStatsManager *pStatsManager = g_pGame->GetPlayerStatsManager();
	if (pStatsManager != NULL)
	{
		CPlayerStat *pMoney = pStatsManager->GetStat(EPS_Money);
		if (!pMoney) return;

		value += pMoney->GetStatValue();
		pMoney->SetValue(value);

		IUIElement *UI = gEnv->pFlashUI->GetUIElement("Money");
		SUIArguments arg; arg.AddArgument(value);
		UI->CallFunction("SetMoney", arg);
	}
}


void CBasicMoney::StartSpark()
{
	const char* particleName = NULL;
	SmartScriptTable propertiesTable = GetSmartScriptTable();
	if (!propertiesTable) return;

	bool r = propertiesTable->GetValue("sParticle", particleName);
	if (!strcmp(particleName, "") || !r)
		particleName = DEFAULT_PARTICLE;

	IParticleEffect *pFireParticle = gEnv->pParticleManager->FindEffect(particleName);
	SEntitySpawnParams spawn;
	spawn.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("ParticleEffect");
	if (spawn.pClass)
	{
		m_pParticalEntity = gEnv->pEntitySystem->SpawnEntity(spawn);
		m_pParticalEntity->SetPos(GetEntity()->GetPos());
		if (pFireParticle && m_pParticalEntity)
		{
			m_pParticalEntity->FreeSlot(0);
			m_pParticalEntity->LoadParticleEmitter(0, pFireParticle);
		}
	}
}

SmartScriptTable CBasicMoney::GetSmartScriptTable()
{
	IScriptTable *pScriptTable = GetEntity()->GetScriptTable();
	if (!pScriptTable) return 0;

	SmartScriptTable propertiesTable;
	if (!pScriptTable->GetValue("Properties", propertiesTable)) return 0;
	else return propertiesTable;
}

SInventoryItem* CBasicMoney::GetItemParamsXML()
{
	SInventoryItem* pItem = new SInventoryItem;
	pItem->size = EIIS_1x1;
	pItem->name = "Coin";

	return pItem;
}