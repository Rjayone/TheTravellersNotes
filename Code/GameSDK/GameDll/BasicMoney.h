/****************************************************************************************************************************
* File: BasicMoney.h/cpp

* Description: Базовый игровой инвентарный объект - деньги. Описывает количество денег если подобрать, рандом-функцию и модель

* Created by: Andrew Medvedev

* Date: 18.03.2014

* Diko Source File

*****************************************************************************************************************************/

#ifndef _BASIC_MONEY_
#define _BASIC_MONEY_

#include "BasicObject.h"

enum EBagSize
{
	EBS_SingleCoin,
	EBS_SomeCoins,
	EBS_SmallBag,
	EBS_MediumBag,
	EBS_BigBag
};

class CBasicMoney : public IBasicObject
{
public:
	CBasicMoney();

	// IGameObjectExtension
	bool Init(IGameObject * pGameObject);
	void PostInit(IGameObject * pGameObject);
	void InitClient(int channelId){}
	void PostInitClient(int channelId){}

	bool ReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params);
	void PostReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params){}
	bool GetEntityPoolSignature(TSerialize signature){ return true; }
	void Release();

	void FullSerialize(TSerialize ser);
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int pflags){ return true; }
	void PostSerialize(){}
	void SerializeSpawnInfo(TSerialize ser){}
	ISerializableInfoPtr GetSpawnInfo(){ return 0; }

	void Update(SEntityUpdateContext& ctx, int updateSlot);
	void HandleEvent(const SGameObjectEvent& event){}
	void ProcessEvent(SEntityEvent& event);

	void SetChannelId(uint16 id){}
	void SetAuthority(bool auth){}

	void PostUpdate(float frameTime){}
	void PostRemoteSpawn(){}
	void GetMemoryUsage(ICrySizer* s) const{ s->Add(this); }
	//~ IGameObjectExtension

	void OnUse(){}
	void OnPickUp(EntityId id);
	void OnDrop(SInventoryItem* pItem, EntityId id);
	void Reset();

	void Physicalize(int type = 2/*PE_RIGID*/);
	void AddMoney(int value);
	void StartSpark();
	SInventoryItem* GetItemParamsXML();
	SmartScriptTable GetSmartScriptTable();

	string GetObjectName(){ return m_ObjectName; }
	string GetObjetDescription(){ return m_ObjectDescr; }
	string GetModelPath(){ return m_ModelPath; }
	string GetEntityType(){ return m_EntityType; }
private:
	int m_nCount;
	float m_fSparkTimer;

	bool m_bSparkEnable;
	IEntity *m_pParticalEntity;
	string m_pParticalEffect;
};

#endif