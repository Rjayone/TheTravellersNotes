/*********************************************************************************************************
- File: Props.h
- Description: Реализация сьульев/кроватей для взаимодействия с окружающим миром
- Created by: Гомза Яков
- Date: 11.06.2014
- Diko Source File
********************************************************************************************************/

#ifndef _Props_
#define _Props_

#include "IGameObject.h"
#include "IGameFramework.h"
#include "Player.h"

class CProps : public IActionListener, public CGameObjectExtensionHelper <CProps, IGameObjectExtension>, public IGameFrameworkListener // наследование основному классу вещей

{
public:
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
	void OnPostUpdate(float fDeltaTime);
	void OnSaveGame(ISaveGame* pSaveGame){}
	void OnLoadGame(ILoadGame* pLoadGame){}
	void OnLevelEnd(const char* nextLevel){}
	void OnActionEvent(const SActionEvent& event){}

	void OnAction(const ActionId& action, int activationMode, float value);
	//~//IActionListener

	CProps();
	//~CProps();

	void Reset();

	void InitOptions();

private:

	IScriptTable *pScriptable;
	IEntity *pEntity;
	ISkeletonAnim* pSkeleton;
	ICharacterInstance* pChar;
	bool bSitting;

};


#endif