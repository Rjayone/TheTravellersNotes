/****************************************************************************************************************************
* File: BasicObject.h

* Description: Базовый игровой инвентарный объект. Создан в замен базовому дабы была возможность использовать его в архитайп ентити

* Created by: Andrew Medvedev

* Date: 22.08.2013

* -Update: 29.03.14

* Diko Source File

*****************************************************************************************************************************/

#ifndef _IBASIC_OBJECT_
#define _IBASIC_OBJECT_

#include "IGameObject.h"
#include "InventoryItems.h"

class IBasicObject : public CGameObjectExtensionHelper< IBasicObject, IGameObjectExtension >
{
public:
	// IGameObjectExtension
	virtual bool Init(IGameObject * pGameObject) = 0;
	virtual void PostInit(IGameObject * pGameObject) = 0;
	virtual void InitClient(int channelId) = 0;
	virtual void PostInitClient(int channelId) = 0;

	virtual bool ReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params) = 0;
	virtual void PostReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params) = 0;
	virtual bool GetEntityPoolSignature(TSerialize signature) = 0;
	virtual void Release() = 0;

	virtual void FullSerialize(TSerialize ser) = 0;
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int pflags) = 0;
	virtual void PostSerialize() = 0;
	virtual void SerializeSpawnInfo(TSerialize ser) = 0;
	virtual ISerializableInfoPtr GetSpawnInfo() = 0;
	virtual void Update(SEntityUpdateContext& ctx, int updateSlot) = 0;
	virtual void HandleEvent(const SGameObjectEvent& event) = 0;
	virtual void ProcessEvent(SEntityEvent& event) = 0;
	virtual void SetChannelId(uint16 id) = 0;
	virtual void SetAuthority(bool auth) = 0;
	virtual void PostUpdate(float frameTime) = 0;
	virtual void PostRemoteSpawn() = 0;
	virtual void GetMemoryUsage(ICrySizer* s) const = 0;
	//~ IGameObjectExtension

	virtual void OnUse() = 0;
	virtual void Reset() = 0;
	//~ IGameObjectExtension

	virtual void OnPickUp(EntityId id) = 0;
	virtual void OnDrop(SInventoryItem* pItem, EntityId id) = 0;
	virtual SmartScriptTable GetSmartScriptTable() = 0;

	virtual string GetObjectName() = 0;
	virtual string GetObjetDescription() = 0;
	virtual string GetModelPath() = 0;
	virtual string GetEntityType() = 0;

	virtual SInventoryItem* GetItemParamsXML() = 0;
protected:
	string m_ObjectName;
	string m_ObjectDescr;
	string m_ModelPath;
	string m_EntityType;

	int m_nSize;
	int m_nCost;

	EntityId m_ItemId;
};

#endif