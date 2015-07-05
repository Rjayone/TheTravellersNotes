#ifndef _Backpack_H_
#define _Backpack_H_

#include "IGameObject.h"
#include "IGameFramework.h"

class CBackpack : public CGameObjectExtensionHelper<CBackpack, IGameObjectExtension>,
	public IActionListener
{
	CBackpack();
	~CBackpack();


	// IGameObjectExtension
	bool Init(IGameObject * pGameObject);
	void PostInit(IGameObject * pGameObject);
	void InitClient(int channelId){}
	void PostInitClient(int channelId) {}
	bool ReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params) { return true; }
	void PostReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params) {}
	bool GetEntityPoolSignature(TSerialize signature) { return true; }
	void Release(){ delete this; }
	void FullSerialize(TSerialize ser){}
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) { return false; }
	void PostSerialize(){}
	void SerializeSpawnInfo(TSerialize ser) {}
	ISerializableInfoPtr GetSpawnInfo() { return 0; }
	void Update(SEntityUpdateContext& ctx, int slot) {}
	void HandleEvent(const SGameObjectEvent& gameObjectEvent){}
	void ProcessEvent(SEntityEvent& entityEvent);
	void SetChannelId(uint16 id) {}
	void SetAuthority(bool auth) {}
	void PostUpdate(float frameTime) { CRY_ASSERT(false); }
	void PostRemoteSpawn() {}
	void GetMemoryUsage(ICrySizer *pSizer) const{ pSizer->Add(*this); }
	// ~IGameObjectExtension

	void OnAction(const ActionId& action, int activationMode, float value);
	//~//IActionListener

private:
	void Reset();
	IScriptTable *pScriptable;
	IEntity *pEntity;
};

#endif
