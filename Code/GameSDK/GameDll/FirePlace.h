/****************************************
* File: FirePlace.h
* Description: Игровой объект - костер
* Created by: Andrew Medvedev
* Date: 22.08.2013
* Diko Source File
*****************************************/

#ifndef _FIREPLACE_
#define _FIREPLACE_

#include "IGameObject.h"
#include "PlaceableEntity.h"
#include "SearchDataStruct.h"

class CFirePlace : public IPlaceableEntity, public IActionListener
{
public:

	enum State {
		IDLE,
		PLACING,
		PLACED,
		FLAME_FADE_IN,
		BURNING,
		FLAME_FADE_OUT,
		BURNED
	};


	CFirePlace();
	virtual ~CFirePlace();

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

	void OnAction(const ActionId& action, int activationMode, float value);

	virtual void StartPlacing();
	virtual void StopPlacing();
	virtual void CheckLocalBounds(){};
	virtual void OnUse();
	virtual void Reset();

	void StartFire();
	void StopFire();

	const ISearchDataStruct& getSearchStruct() { return *searchStruct;  }
	State GetState() {	return state;	}
	void SetState(State newState) {	 state = newState;  }

private:

	void UpdatePlacing();

	SmartScriptTable m_pScriptTable;
	IEntity* m_pParticalEntity;
	State state;
	static ISearchDataStruct* searchStruct;

	// Color mutiplier (implementation of brightness)
	float m_fColorMultipier;

	// The color of a light coming from the flame in it`s peak brightness
	ColorF m_fullColor;

	// Default start color
	ColorF m_noColor;

	// Light properties
	//CDLight lightProperties;

	ILightSource* m_pLightSource;

	// Time of fading in / out in seconds
	float m_fFadeTimeLimit;

	// Accumulates time, that passed since fade in or fade out event started
	float m_fFadeTimeAcc;
};

#endif