/*********************************************************************************************************
- File: BuildSelectionHelper.h
- Description: ���������� ������ �������, ���������� ��� ���������� � ��������������� ������ ������
- Created by: ����� ����
- Date: 25.03.2014
- Update: 19.07.14 by �������� ������
- Diko Source File
- ToDo:�������� ���������� �������� ������� ��� �������
********************************************************************************************************/

#ifndef _BUILD_HELPER_
#define _BUILD_HELPER_

#include "IGameObject.h"
#include "IGameFramework.h"

class CBuildHelper : public CGameObjectExtensionHelper <CBuildHelper, IGameObjectExtension> // ������������ ��������� ������ �����
{
public:
	CBuildHelper();

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

	void OnPostUpdate(float fDeltaTime);
	void OnSaveGame(ISaveGame* pSaveGame){}
	void OnLoadGame(ILoadGame* pLoadGame){}
	void OnLevelEnd(const char* nextLevel){}
	void OnActionEvent(const SActionEvent& event){}

	void Reset();
private:
	IScriptTable *pScriptable;
};


#endif