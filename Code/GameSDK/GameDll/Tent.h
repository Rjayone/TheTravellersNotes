/****************************************
* File: Tent.h
* Description: Перемещаемый игровой объект тент
* Created by: Andrew Medvedev
* Date: 22.08.2013
* Diko Source File
*****************************************/


#ifndef _TENT_
#define _TENT_

#include "PlaceableEntity.h"

class CTent : public IPlaceableEntity
{
public:
	CTent();
	virtual ~CTent();

	// IGameObjectExtension
	virtual bool Init(IGameObject * pGameObject);
	virtual void PostInit(IGameObject * pGameObject);
	virtual void InitClient(int channelId){}
	virtual void PostInitClient(int channelId){}
	virtual bool ReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params){ return true; }
	virtual void PostReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params){}
	virtual bool GetEntityPoolSignature(TSerialize signature) { return true; }
	virtual void Release(){ delete this; }
	virtual void FullSerialize(TSerialize ser){}
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int pflags){ return true; }
	virtual void PostSerialize(){}
	virtual void SerializeSpawnInfo(TSerialize ser){}
	virtual ISerializableInfoPtr GetSpawnInfo(){ return 0; }
	virtual void Update(SEntityUpdateContext& ctx, int updateSlot);
	virtual void HandleEvent(const SGameObjectEvent& event){}
	virtual void ProcessEvent(SEntityEvent& event);
	virtual void SetChannelId(uint16 id){}
	virtual void SetAuthority(bool auth){}
	virtual void PostUpdate(float frameTime){}
	virtual void PostRemoteSpawn(){}
	virtual void GetMemoryUsage(ICrySizer* s) const { s->Add(*this); }
	//~ IGameObjectExtension

	virtual void StartPlacing(IEntity *pObject);
	virtual void StopPlacing(IEntity *pEntity);
	virtual void OnUse();
	virtual void Reset();
	virtual void CheckLocalBounds();

private:

	static bool dynamiclyInit;

	bool m_bStartDrag;
	bool m_bFullInitDone;
	IEntity *m_pObject;
	IScriptTable *m_pScriptTable;
};

#endif