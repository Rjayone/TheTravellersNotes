// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#ifndef AUDIOSYSTEM_H_INCLUDED
#define AUDIOSYSTEM_H_INCLUDED

#include "ATL.h"
#include <TimeValue.h>

// Forward declarations.
class CAudioSystem;
class CAudioProxy;
struct IAudioSystemImplementation;

class CAudioThread : public CrySimpleThread<CAudioThread>
{
public:

	CAudioThread();
	~CAudioThread();

	void Init(CAudioSystem* const pAudioSystem);
	virtual void Run();

	bool IsActive() const;
	void Activate();
	void Deactivate();

private:

	CAudioSystem* m_pAudioSystem;
};

class CAudioSystem : public IAudioSystem
{
public:

	CAudioSystem();
	virtual ~CAudioSystem();

	// IAudioSystem
	VIRTUAL void										Initialize(IAudioSystemImplementation* const pImpl);
	VIRTUAL void										Release();
	VIRTUAL void										PushRequest(SAudioRequest const& rAudioRequestData);
	VIRTUAL void										AddRequestListener(void (*func)(SAudioRequestInfo const* const), void* const pRequestOwner);
	VIRTUAL void										RemoveRequestListener(void (*func)(SAudioRequestInfo const* const), void* const pRequestOwner);

	VIRTUAL void										ExternalUpdate();
	void														InternalUpdate();

	VIRTUAL bool										GetAudioTriggerID(char const* const sAudioTriggerName, TAudioControlID& rAudioTriggerID) const;
	VIRTUAL bool										GetAudioRtpcID(char const* const sAudioRtpcName, TAudioControlID& rAudioRtpcID) const;
	VIRTUAL bool										GetAudioSwitchID(char const* const sAudioSwitchName, TAudioControlID& rAudioStateID) const;
	VIRTUAL bool										GetAudioSwitchStateID(TAudioControlID const nSwitchID, char const* const sAudioSwitchStateName, TAudioSwitchStateID& rAudioStateID) const;
	VIRTUAL bool										GetAudioPreloadRequestID(char const* const sAudioPreloadRequestName, TAudioPreloadRequestID& rAudioPreloadRequestID) const;
	VIRTUAL bool										GetAudioEnvironmentID(char const* const sAudioEnvironmentName, TAudioEnvironmentID& rAudioEnvironmentID) const;

	VIRTUAL bool										ReserveAudioListenerID(TAudioObjectID& rAudioObjectID);
	VIRTUAL bool										ReleaseAudioListenerID(TAudioObjectID const nAudioObjectID);

	VIRTUAL void										OnCVarChanged(ICVar* const pCvar) {}
	VIRTUAL void										GetInfo(SAudioSystemInfo& rAudioSystemInfo);
	VIRTUAL char const*							GetConfigPath() const;

	VIRTUAL IAudioProxy*						GetFreeAudioProxy();
	VIRTUAL void										FreeAudioProxy(IAudioProxy* const pIAudioProxy);

	// If INCLUDE_AUDIO_PRODUCTION_CODE is not defined, these two unctions always return NULL
	VIRTUAL char const*							GetAudioControlName(EAudioControlType const eAudioEntityType, TATLIDType const nAudioEntityID);
	VIRTUAL char const*							GetAudioControlName(EAudioControlType const eAudioEntityType, TATLIDType const nAudioEntityID1, TATLIDType const nAudioEntityID2);
	// ~IAudioSystem

private:

	typedef std::deque<CAudioRequestInternal, STLSoundAllocator<CAudioRequestInternal> > TAudioRequests;
	typedef std::vector<CAudioProxy*, STLSoundAllocator<CAudioProxy*> > TAudioProxies;

	CAudioSystem(CAudioSystem const&);            // not defined; calls will fail at compile time
	CAudioSystem& operator=(CAudioSystem const&); // not defined; calls will fail at compile time

	bool														ShutDown();
	void														UpdateTime();
	bool														ProcessRequests(TAudioRequests& rRequestQueue);
	void														ProcessRequest(CAudioRequestInternal& rRequest);
	bool														ExecuteSyncCallbacks(TAudioRequests& rRequestQueue);

	bool														m_bSystemInitialized;

	CTimeValue											m_LastUpdateTime;
	float														m_fAsyncTimeInMS;
	float														m_fUpdateIntervalMS;

	CAudioThread										m_oMainAudioThread;

	enum EAudioRequestQueueType ATL_ENUM_TYPE
	{
		eARQT_ASYNCH	= 0,
		eARQT_SYNCH		= 1,

		eARQT_COUNT
	};

	enum EAudioRequestQueuePriority ATL_ENUM_TYPE
	{
		eARQP_HIGH		= 0,
		eARQP_NORMAL	= 1,
		eARQP_LOW			= 2,

		eARQP_COUNT
	};

	enum EAudioRequestQueueID ATL_ENUM_TYPE
	{
		eARQID_ONE	= 0,
		eARQID_TWO	= 1,

		eARQID_COUNT
	};

	TAudioRequests												m_aRequestQueues[eARQT_COUNT][eARQP_COUNT][eARQID_COUNT];
	TAudioRequests												m_aInternalRequestQueue;
	CAudioTriggerCallbackQueue						m_aTriggerCallbackQueue;

	CAudioTranslationLayer								m_oATL;

	CryEvent															m_oMainEvent;
	CryCriticalSection										m_oMainCS;
	CryCriticalSection										m_oInternalRequestQueueCS;

	TAudioProxies													m_apAudioProxies;

	CryFixedStringT<MAX_AUDIO_FILE_PATH_LENGTH>	m_sConfigPath;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	CATLDebugNameStore										m_oDebugNameStore;

	void																	DrawAudioDebugData();
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
};

#endif //AUDIOSYSTEM_H_INCLUDED
