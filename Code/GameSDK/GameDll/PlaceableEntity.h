/****************************************
* File: PlaceableEntity.h
* Description: Перемещаемый игровой объект
* Created by: Andrew Medvedev
* Date: 22.08.2013
* Diko Source File
*****************************************/

#ifndef _IPLACEABLE_ENTITY_
#define _IPLACEABLE_ENTITY_

#include "IGameObject.h"

class IPlaceableEntity : public CGameObjectExtensionHelper< IPlaceableEntity, IGameObjectExtension >
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

	virtual void StartPlacing() = 0;
	virtual void StopPlacing() = 0;
	virtual void CheckLocalBounds() = 0;
	virtual void OnUse() = 0;
	virtual void Reset() = 0;
};

#endif //_IPLACEABLE_ENTITY_