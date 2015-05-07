// CryEngine Source File.
// Copyright (C), Crytek, 1999-2014.

#include "stdafx.h"
#include "AudioSystem.h"
#include "SoundCVars.h"
#include "AudioProxy.h"
#include <ITimer.h>

///////////////////////////////////////////////////////////////////////////
CAudioThread::CAudioThread()
	:	m_pAudioSystem(NPTR)
{
}

///////////////////////////////////////////////////////////////////////////
CAudioThread::~CAudioThread()
{
}

///////////////////////////////////////////////////////////////////////////
void CAudioThread::Init(CAudioSystem* const pAudioSystem)
{
	m_pAudioSystem = pAudioSystem;
}

//////////////////////////////////////////////////////////////////////////
void CAudioThread::Run()
{
	while (IsStarted())
	{
		m_pAudioSystem->InternalUpdate();
	}
}

///////////////////////////////////////////////////////////////////////////
void CAudioThread::Activate()
{
	Start(0, "MainAudioThread", THREAD_PRIORITY_BELOW_NORMAL);
}

///////////////////////////////////////////////////////////////////////////
void CAudioThread::Deactivate()
{
	Cancel();
	Stop();
	WaitForThread();
}

///////////////////////////////////////////////////////////////////////////
bool CAudioThread::IsActive() const
{
	return IsStarted();
}

//////////////////////////////////////////////////////////////////////////
CAudioSystem::CAudioSystem()
	:	m_bSystemInitialized(false)
	,	m_LastUpdateTime()	
	,	m_fAsyncTimeInMS(0.0f)
	,	m_fUpdateIntervalMS(0.0f)
	,	m_oATL()
{
	m_apAudioProxies.reserve(g_SoundCVars.m_nAudioObjectPoolSize);
	m_oMainAudioThread.Init(this);

	CATLAudioObjectBase::SetAudioTriggerCallbackQueue(&m_aTriggerCallbackQueue);
}

//////////////////////////////////////////////////////////////////////////
CAudioSystem::~CAudioSystem()
{
}

//////////////////////////////////////////////////////////////////////////
void CAudioSystem::PushRequest(SAudioRequest const& rAudioRequestData)
{
	FUNCTION_PROFILER_ALWAYS(gEnv->pSystem, PROFILE_AUDIO);
	
	CAudioRequestInternal oRequest(rAudioRequestData);

	switch (rAudioRequestData.pData->eRequestType)
	{
	case eART_AUDIO_CALLBACK_MANAGER_REQUEST:
		{
			CryAutoLock<CryCriticalSection> oAutoLock(m_oInternalRequestQueueCS);// TODO: replace with a proper wait-free queue
			m_aInternalRequestQueue.push_back(oRequest);

			break;
		}
	default:
		{
			assert(gEnv->mMainThreadId == CryGetCurrentThreadId());

			bool const bIsAsync = (rAudioRequestData.nFlags & eARF_SYNC_CALLBACK) == 0;

			TATLEnumFlagsType const nQueueType = bIsAsync ? eARQT_ASYNCH : eARQT_SYNCH;
			TATLEnumFlagsType nQueuePriority = eARQP_LOW;

			if ((rAudioRequestData.nFlags & eARF_PRIORITY_HIGH) != 0)
			{
				nQueuePriority = eARQP_HIGH;
			}
			else if ((rAudioRequestData.nFlags & eARF_PRIORITY_NORMAL) != 0)
			{
				nQueuePriority = eARQP_NORMAL;
			}

			// This call might happen recursively (originating in "ExecuteSyncCallbacks")
			{
				CryAutoLock<CryCriticalSection> oAutoLock(m_oMainCS);
				m_aRequestQueues[nQueueType][nQueuePriority][eARQID_ONE].push_back(oRequest);
			}

			if ((rAudioRequestData.nFlags & eARF_EXECUTE_BLOCKING) != 0)
			{
				m_oMainEvent.Wait();
				m_oMainEvent.Reset();

				if (!bIsAsync)
				{
					CryAutoLock<CryCriticalSection> oAutoLock(m_oMainCS);

					// Notify the listeners from producer thread.
					ExecuteSyncCallbacks(m_aRequestQueues[eARQT_SYNCH][nQueuePriority][eARQID_ONE]);
				}
			}

			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CAudioSystem::AddRequestListener(void (*func)(SAudioRequestInfo const* const), void* const pRequestOwner)
{
	SAudioRequest oRequest;
	oRequest.nFlags = eARF_PRIORITY_HIGH | eARF_EXECUTE_BLOCKING;
	SAudioManagerRequestData<eAMRT_ADD_REQUEST_LISTENER> oRequestData;
	oRequestData.func						= func;
	oRequestData.pRequestOwner	= pRequestOwner;
	oRequest.pOwner							= pRequestOwner; // This makes sure that the listener is notified.
	oRequest.pData							= &oRequestData;

	PushRequest(oRequest);
}

//////////////////////////////////////////////////////////////////////////
void CAudioSystem::RemoveRequestListener(void (*func)(SAudioRequestInfo const* const), void* const pRequestOwner)
{
	SAudioRequest oRequest;
	oRequest.nFlags = eARF_PRIORITY_HIGH | eARF_EXECUTE_BLOCKING;
	SAudioManagerRequestData<eAMRT_REMOVE_REQUEST_LISTENER> oRequestData;
	oRequestData.func						= func;
	oRequestData.pRequestOwner	= pRequestOwner;
	oRequest.pOwner							= pRequestOwner; // This makes sure that the listener is notified.
	oRequest.pData							= &oRequestData;

	PushRequest(oRequest);
}

//////////////////////////////////////////////////////////////////////////
void CAudioSystem::ExternalUpdate()
{
	FUNCTION_PROFILER_ALWAYS(gEnv->pSystem, PROFILE_AUDIO);

	assert(gEnv->mMainThreadId == CryGetCurrentThreadId());

	{
		CryAutoLock<CryCriticalSection> oAutoLock(m_oMainCS);

		bool const bSyncCallback = 
			ExecuteSyncCallbacks(m_aRequestQueues[eARQT_SYNCH][eARQP_HIGH][eARQID_ONE]) ||
			ExecuteSyncCallbacks(m_aRequestQueues[eARQT_SYNCH][eARQP_NORMAL][eARQID_ONE]) ||
			ExecuteSyncCallbacks(m_aRequestQueues[eARQT_SYNCH][eARQP_LOW][eARQID_ONE]);
	}

	m_aTriggerCallbackQueue.ExecuteAll();

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	DrawAudioDebugData();
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
}

//////////////////////////////////////////////////////////////////////////
void CAudioSystem::UpdateTime()
{
	CTimeValue const CurrentAsyncTime(gEnv->pTimer->GetAsyncTime());
	m_fAsyncTimeInMS		= (CurrentAsyncTime - m_LastUpdateTime).GetMilliSeconds();
	m_LastUpdateTime		= CurrentAsyncTime;
	m_fUpdateIntervalMS	+= m_fAsyncTimeInMS;
}

//////////////////////////////////////////////////////////////////////////
void CAudioSystem::InternalUpdate()
{
	bool bSleep = false;
	{
		FUNCTION_PROFILER_ALWAYS(gEnv->pSystem, PROFILE_AUDIO);

		UpdateTime();

		// Process HIGH if not empty, else NORMAL, else LOW.
		bool const bAsynch = 
			ProcessRequests(m_aRequestQueues[eARQT_ASYNCH][eARQP_HIGH][eARQID_TWO]) ||
			ProcessRequests(m_aRequestQueues[eARQT_ASYNCH][eARQP_NORMAL][eARQID_TWO]) ||
			ProcessRequests(m_aRequestQueues[eARQT_ASYNCH][eARQP_LOW][eARQID_TWO]);

		bool const bSynch = 
			ProcessRequests(m_aRequestQueues[eARQT_SYNCH][eARQP_HIGH][eARQID_TWO]) ||
			ProcessRequests(m_aRequestQueues[eARQT_SYNCH][eARQP_NORMAL][eARQID_TWO]) ||
			ProcessRequests(m_aRequestQueues[eARQT_SYNCH][eARQP_LOW][eARQID_TWO]);

		if (m_oInternalRequestQueueCS.TryLock())
		{
			ProcessRequests(m_aInternalRequestQueue);
			m_oInternalRequestQueueCS.Unlock();
		}

		// Minimum update interval of 2 ms for now.
		if (m_fUpdateIntervalMS > 2.0f)
		{
			m_oATL.Update(m_fUpdateIntervalMS);
			m_fUpdateIntervalMS = 0.0f;
		}

		// The execution time of this block defines the maximum amount of time the producer will be locked.
		{
			CryAutoLock<CryCriticalSection> oAutoLock(m_oMainCS);

			m_aRequestQueues[eARQT_ASYNCH][eARQP_HIGH  ][eARQID_TWO].swap(m_aRequestQueues[eARQT_ASYNCH][eARQP_HIGH  ][eARQID_ONE]);
			m_aRequestQueues[eARQT_ASYNCH][eARQP_NORMAL][eARQID_TWO].swap(m_aRequestQueues[eARQT_ASYNCH][eARQP_NORMAL][eARQID_ONE]);
			m_aRequestQueues[eARQT_ASYNCH][eARQP_LOW   ][eARQID_TWO].swap(m_aRequestQueues[eARQT_ASYNCH][eARQP_LOW   ][eARQID_ONE]);
			m_aRequestQueues[eARQT_SYNCH ][eARQP_HIGH  ][eARQID_TWO].swap(m_aRequestQueues[eARQT_SYNCH ][eARQP_HIGH  ][eARQID_ONE]);
			m_aRequestQueues[eARQT_SYNCH ][eARQP_NORMAL][eARQID_TWO].swap(m_aRequestQueues[eARQT_SYNCH ][eARQP_NORMAL][eARQID_ONE]);
			m_aRequestQueues[eARQT_SYNCH ][eARQP_LOW   ][eARQID_TWO].swap(m_aRequestQueues[eARQT_SYNCH ][eARQP_LOW   ][eARQID_ONE]);

			bSleep =	m_aRequestQueues[eARQT_ASYNCH][eARQP_HIGH  ][eARQID_TWO].empty() &&
				m_aRequestQueues[eARQT_ASYNCH][eARQP_NORMAL][eARQID_TWO].empty() &&
				m_aRequestQueues[eARQT_ASYNCH][eARQP_LOW   ][eARQID_TWO].empty() &&
				m_aRequestQueues[eARQT_SYNCH ][eARQP_HIGH  ][eARQID_TWO].empty() &&
				m_aRequestQueues[eARQT_SYNCH ][eARQP_NORMAL][eARQID_TWO].empty() &&
				m_aRequestQueues[eARQT_SYNCH ][eARQP_LOW   ][eARQID_TWO].empty();

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
			m_oDebugNameStore.SyncChanges(m_oATL.GetDebugStore());
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
		}
	}
	
	if (bSleep)
	{
		CrySleep(2);
	}
}

///////////////////////////////////////////////////////////////////////////
void CAudioSystem::Initialize(IAudioSystemImplementation* const pImpl)
{
	if (m_oMainAudioThread.IsActive())
	{
		m_oMainAudioThread.Deactivate();
	}

	m_sConfigPath = CryFixedStringT<MAX_AUDIO_FILE_PATH_LENGTH>((PathUtil::GetGameFolder() + "/libs/gameaudio/").c_str()); 

	m_oATL.Initialize(pImpl);

	m_oMainAudioThread.Activate();

	for (int i = 0; i < g_SoundCVars.m_nAudioObjectPoolSize; ++i)
	{
		POOL_NEW_CREATE(CAudioProxy, pAudioProxy);
		m_apAudioProxies.push_back(pAudioProxy);
	}
}

///////////////////////////////////////////////////////////////////////////
bool CAudioSystem::ShutDown()
{
	if (!m_apAudioProxies.empty())
	{
		TAudioProxies::const_iterator Iter(m_apAudioProxies.begin());
		TAudioProxies::const_iterator const IterEnd(m_apAudioProxies.end());

		for (; Iter != IterEnd; ++Iter)
		{
			POOL_FREE(*Iter);
		}
	}

	stl::free_container(m_apAudioProxies);

	m_oMainAudioThread.Deactivate();

	return m_oATL.ShutDown();
}

///////////////////////////////////////////////////////////////////////////
bool CAudioSystem::GetAudioTriggerID(char const* const sAudioTriggerName, TAudioControlID& rAudioTriggerID) const
{
	return m_oATL.GetAudioTriggerID(sAudioTriggerName, rAudioTriggerID);
}

///////////////////////////////////////////////////////////////////////////
bool CAudioSystem::GetAudioRtpcID(char const* const sAudioRtpcName, TAudioControlID& rAudioRtpcID) const
{
	return m_oATL.GetAudioRtpcID(sAudioRtpcName, rAudioRtpcID);
}

///////////////////////////////////////////////////////////////////////////
bool CAudioSystem::GetAudioSwitchID(char const* const sAudioStateName, TAudioControlID& rAudioStateID) const
{
	return m_oATL.GetAudioSwitchID(sAudioStateName, rAudioStateID);
}

///////////////////////////////////////////////////////////////////////////
bool CAudioSystem::GetAudioSwitchStateID(
	TAudioControlID const nSwitchID, 
	char const* const sAudioSwitchStateName, 
	TAudioSwitchStateID& rAudioStateID) const
{
	return m_oATL.GetAudioSwitchStateID(nSwitchID, sAudioSwitchStateName, rAudioStateID);
}

//////////////////////////////////////////////////////////////////////////
bool CAudioSystem::GetAudioPreloadRequestID(char const* const sAudioPreloadRequestName, TAudioPreloadRequestID& rAudioPreloadRequestID) const
{
	return m_oATL.GetAudioPreloadRequestID(sAudioPreloadRequestName, rAudioPreloadRequestID);
}

///////////////////////////////////////////////////////////////////////////
bool CAudioSystem::GetAudioEnvironmentID(char const* const sAudioEnvironmentName, TAudioEnvironmentID& rAudioEnvironmentID) const
{
	return m_oATL.GetAudioEnvironmentID(sAudioEnvironmentName, rAudioEnvironmentID);
}

///////////////////////////////////////////////////////////////////////////
bool CAudioSystem::ReserveAudioListenerID(TAudioObjectID& rAudioObjectID)
{
	return m_oATL.ReserveAudioListenerID(rAudioObjectID);
}

///////////////////////////////////////////////////////////////////////////
bool CAudioSystem::ReleaseAudioListenerID(TAudioObjectID const nAudioObjectID)
{
	return m_oATL.ReleaseAudioListenerID(nAudioObjectID);
}

///////////////////////////////////////////////////////////////////////////
void CAudioSystem::GetInfo(SAudioSystemInfo& rAudioSystemInfo)
{
	//TODO: 
}

///////////////////////////////////////////////////////////////////////////
char const* CAudioSystem::GetConfigPath() const
{
	return m_sConfigPath.c_str(); 
}

//////////////////////////////////////////////////////////////////////////
IAudioProxy* CAudioSystem::GetFreeAudioProxy()
{
	CAudioProxy* pAudioProxy = NPTR;

	if (!m_apAudioProxies.empty())
	{
		pAudioProxy = m_apAudioProxies.back();
		m_apAudioProxies.pop_back();
	}
	else 
	{
		POOL_NEW(CAudioProxy, pAudioProxy);

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
		if (pAudioProxy == NPTR)
		{
			CryFatalError("<Audio>: Failed to create new AudioProxy instance!");
		}
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
	}

	return static_cast<IAudioProxy*>(pAudioProxy);
}

//////////////////////////////////////////////////////////////////////////
void CAudioSystem::FreeAudioProxy(IAudioProxy* const pIAudioProxy)
{
	CAudioProxy* const pAudioProxy = static_cast<CAudioProxy*>(pIAudioProxy);

	if (m_apAudioProxies.size() < g_SoundCVars.m_nAudioObjectPoolSize)
	{
		m_apAudioProxies.push_back(pAudioProxy);
	}
	else
	{
		POOL_FREE(pIAudioProxy);
	}
}

///////////////////////////////////////////////////////////////////////////
char const* CAudioSystem::GetAudioControlName(EAudioControlType const eAudioEntityType, TATLIDType const nAudioEntityID)
{
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	char const* sResult = NPTR;
	switch (eAudioEntityType)
	{
	case eACT_AUDIO_OBJECT:
		{
			sResult = m_oDebugNameStore.LookupAudioObjectName(nAudioEntityID);
			
			break;
		}
	case eACT_TRIGGER:
		{
			sResult = m_oDebugNameStore.LookupAudioTriggerName(nAudioEntityID);

			break;
		}
	case eACT_RTPC:
		{
			sResult = m_oDebugNameStore.LookupAudioRtpcName(nAudioEntityID);

			break;
		}
	case eACT_SWITCH:
		{
			sResult = m_oDebugNameStore.LookupAudioSwitchName(nAudioEntityID);

			break;
		}
	case eACT_PRELOAD:
		{
			sResult = m_oDebugNameStore.LookupAudioPreloadRequestName(nAudioEntityID);

			break;
		}
	case eACT_ENVIRONMENT:
		{
			sResult = m_oDebugNameStore.LookupAudioEnvironmentName(nAudioEntityID);

			break;
		}
	case eACT_SWITCH_STATE:
		{
			g_AudioLogger.Log(eALT_WARNING, "GetAudioConstrolName() for eACT_SWITCH_STATE was called with one AudioEntityID, needs two: SwitchID and StateID");
			break;
		}
	case eACT_NONE: // fall-through
	default:
		{
			break;
		}
	}

	return sResult;

#else  // INCLUDE_AUDIO_PRODUCTION_CODE
	return NPTR;
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
}

///////////////////////////////////////////////////////////////////////////
char const* CAudioSystem::GetAudioControlName(EAudioControlType const eAudioEntityType, TATLIDType const nAudioEntityID1, TATLIDType const nAudioEntityID2)
{
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	char const* sResult = NPTR;

	switch (eAudioEntityType)
	{
	case eACT_AUDIO_OBJECT:
	case eACT_TRIGGER:
	case eACT_RTPC:
	case eACT_SWITCH:
	case eACT_PRELOAD:
	case eACT_ENVIRONMENT:
		{
			g_AudioLogger.Log(eALT_WARNING, "GetAudioConstrolName() was called with two AudioEntityIDs for a control that requires only one");
			break;
		}
	case eACT_SWITCH_STATE:
		{
			sResult = m_oDebugNameStore.LookupAudioSwitchStateName(nAudioEntityID1, nAudioEntityID2);

			break;
		}
	case eACT_NONE: // fall-trhough
	default:
		{
			break;
		}
	}

	return sResult;

#else  // INCLUDE_AUDIO_PRODUCTION_CODE
	return NPTR;
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
}

///////////////////////////////////////////////////////////////////////////
void CAudioSystem::Release()
{
	// Unload global audio binary data.
	SAudioManagerRequestData<eAMRT_UNLOAD_AFCM_DATA_BY_SCOPE> oAMData(eADS_GLOBAL);
	SAudioRequest oAudioRequestData;
	oAudioRequestData.nFlags = eARF_PRIORITY_HIGH | eARF_EXECUTE_BLOCKING;
	oAudioRequestData.pData = &oAMData;
	PushRequest(oAudioRequestData);

	ShutDown();
	POOL_FREE(this);

	g_SoundCVars.UnregisterVariables();

	// The AudioSystem must be the last object that is freed from the audio memory pool before it is destroyed and the first that is allocated from it!
	uint8* pMemSystem = g_MemoryPoolPrimary.Data();
	g_MemoryPoolPrimary.UnInitMem();
	delete[] pMemSystem;
}

//////////////////////////////////////////////////////////////////////////
bool CAudioSystem::ExecuteSyncCallbacks(TAudioRequests& rRequestQueue)
{
	FUNCTION_PROFILER_ALWAYS(gEnv->pSystem, PROFILE_AUDIO);

	bool bSuccess = false;

	if (!rRequestQueue.empty())
	{
		bSuccess = true;

		// Don't use iterators because "NotifyListener" can invalidate
		// the rRequestQueue container, or change its size.
		for (size_t i = 0, nIncrement = 1; i < rRequestQueue.size(); i += nIncrement)
		{
			nIncrement = 1;
			CAudioRequestInternal const& rRequest = rRequestQueue[i];

			if (rRequest.eStatus == eARS_SUCCESS || rRequest.eStatus == eARS_FAILURE)
			{
				nIncrement = 0;
				m_oATL.NotifyListener(rRequest);
				rRequestQueue.erase(rRequestQueue.begin() + i);
			}
		}
	}

	return bSuccess;
}

//////////////////////////////////////////////////////////////////////////
void CAudioSystem::ProcessRequest(CAudioRequestInternal& rRequest)
{
	m_oATL.ProcessRequest(rRequest);
}

//////////////////////////////////////////////////////////////////////////
bool CAudioSystem::ProcessRequests(TAudioRequests& rRequestQueue)
{
	bool bSuccess = false;

	if (!rRequestQueue.empty())
	{
		// TODO: Properly define success here or void this method's return type!
		bSuccess = true;

		// Don't use iterators because "NotifyListener" can invalidate
		// the rRequestQueue container, or change its size.
		for (size_t i = 0, nIncrement = 1; i < rRequestQueue.size(); i += nIncrement)
		{
			nIncrement = 1;
			CAudioRequestInternal& rRequest = rRequestQueue[i];

			if ((rRequest.nInternalInfoFlags & (eARIF_WAITING_FOR_REMOVAL | eARIF_WAITING_FOR_SYNC_CALLBACK)) == 0)
			{
				if (rRequest.eStatus == eARS_NONE)
				{
					rRequest.eStatus = eARS_PENDING;

					ProcessRequest(rRequest);
				}
				else
				{
					// TODO: handle pending requests!
				}

				if (rRequest.eStatus != eARS_PENDING)
				{
					if ((rRequest.nFlags & eARF_SYNC_CALLBACK) == 0)
					{
						m_oATL.NotifyListener(rRequest);

						if ((rRequest.nFlags & eARF_EXECUTE_BLOCKING) != 0)
						{
							m_oMainEvent.Set();
						}

						rRequestQueue.erase(rRequestQueue.begin() + i);
						nIncrement = 0;
					}
					else
					{
						if ((rRequest.nFlags & eARF_EXECUTE_BLOCKING) != 0)
						{
							rRequest.nInternalInfoFlags |= eARIF_WAITING_FOR_REMOVAL;
							m_oMainEvent.Set();
						}
						else
						{
							rRequest.nInternalInfoFlags |= eARIF_WAITING_FOR_SYNC_CALLBACK;
						}
					}
				}
			}
		}
	}

	return bSuccess;
}

//////////////////////////////////////////////////////////////////////////
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
void CAudioSystem::DrawAudioDebugData()
{
	if (g_SoundCVars.m_nDrawAudioDebug > 0)
	{
		SAudioRequest oRequest;
		oRequest.nFlags = eARF_PRIORITY_HIGH | eARF_EXECUTE_BLOCKING;
		SAudioManagerRequestData<eAMRT_DRAW_DEBUG_INFO> oRequestData;
		oRequest.pData = &oRequestData;

		PushRequest(oRequest);
	}
}
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
