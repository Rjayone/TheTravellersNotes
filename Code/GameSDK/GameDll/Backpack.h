#ifndef _Backpack_H_
#define _Backpack_H_

#include "IGameObject.h"
#include "IGameFramework.h"

class CBackpack : public CGameObjectExtensionHelper<CBackpack, IGameObjectExtension>, public IGameFrameworkListener, public IActionListener
{
	CBackpack();
	//~CBackpack();


	// IGameObjectExtension
	virtual bool Init(IGameObject * pGameObject);
	virtual void PostInit(IGameObject * pGameObject);
	virtual void InitClient(int channelId){}
	virtual void PostInitClient(int channelId) {}
	virtual bool ReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params) { return true; }
	virtual void PostReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params) {}
	virtual bool GetEntityPoolSignature(TSerialize signature) { return true; }
	virtual void Release(){ delete this; }
	virtual void FullSerialize(TSerialize ser){}
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) { return false; }
	virtual void PostSerialize(){}
	virtual void SerializeSpawnInfo(TSerialize ser) {}
	virtual ISerializableInfoPtr GetSpawnInfo() { return 0; }
	virtual void Update(SEntityUpdateContext& ctx, int slot) {}
	virtual void HandleEvent(const SGameObjectEvent& gameObjectEvent){}
	virtual void ProcessEvent(SEntityEvent& entityEvent);
	virtual void SetChannelId(uint16 id) {}
	virtual void SetAuthority(bool auth) {}
	virtual void PostUpdate(float frameTime) { CRY_ASSERT(false); }
	virtual void PostRemoteSpawn() {}
	virtual void GetMemoryUsage(ICrySizer *pSizer) const{ pSizer->Add(*this); }
	// ~IGameObjectExtension


	//IActionListener
	void OnPostUpdate(float fDeltaTime){}
	void OnSaveGame(ISaveGame* pSaveGame){}
	void OnLoadGame(ILoadGame* pLoadGame){}
	void OnLevelEnd(const char* nextLevel){}
	void OnActionEvent(const SActionEvent& event){}

	void OnAction(const ActionId& action, int activationMode, float value);
	//~//IActionListener

private:
	void Reset();
	IScriptTable *pScriptable;
	IEntity *pEntity;
};

#endif
