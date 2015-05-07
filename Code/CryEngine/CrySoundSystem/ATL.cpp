// CryEngine Source File.
// Copyright (C), Crytek, 1999-2014.

#include "stdafx.h"
#include "ATL.h"
#include "AudioSystemImpl_NULL.h"
#include "SoundCVars.h"
#include "AudioProxy.h"
#include "ATLAudioObject.h"
#include <IAudioSystemImplementation.h>
#include <ISystem.h>
#include <IPhysics.h>
#include <IRenderer.h>

///////////////////////////////////////////////////////////////////////////
inline EAudioRequestResult ConvertToRequestResult(EAudioRequestStatus eAudioRequestStatus)
{
	EAudioRequestResult eResult = eARR_NONE;

	switch (eAudioRequestStatus)
	{
	case eARS_SUCCESS:
		{
			eResult = eARR_SUCCESS;

			break;
		}
	case eARS_FAILURE:
		{
			eResult = eARR_FAILURE;

			break;
		}
	case eARS_NONE: // Intentional fallthrough!
	default:
		{
			eResult = eARR_NONE;

			break;
		}
	}

	return eResult;
}

///////////////////////////////////////////////////////////////////////////
CAudioTranslationLayer::CAudioTranslationLayer()
	: m_pGlobalAudioObject(NPTR)
	,	m_nGloabalAudioObjectID(1)
	, m_nTriggerInstanceIDCounter(1)
	, m_oAudioEventMgr()
	, m_oAudioObjectMgr()
	, m_oAudioListenerMgr()
	, m_oFileCacheMgr(m_cPreloadRequests)
	, m_oAudioEventListenerMgr()
	, m_oXMLProcessor(m_cTriggers, m_cRtpcs, m_cSwitches, m_cEnvironments, m_cPreloadRequests, m_oFileCacheMgr)
	, m_bIsRunning(false)
	, m_bIsMuted(false)
	, m_pImpl(NPTR)
{
	gEnv->pSystem->GetISystemEventDispatcher()->RegisterListener(this);

	InitATLControlIDs();
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	m_oAudioEventMgr.SetDebugNameStore(&m_oDebugNameStore);
	m_oAudioObjectMgr.SetDebugNameStore(&m_oDebugNameStore);
	m_oXMLProcessor.SetDebugNameStore(&m_oDebugNameStore);
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
}

//////////////////////////////////////////////////////////////////////////
CAudioTranslationLayer::~CAudioTranslationLayer()
{
	if (m_bIsRunning || (m_pImpl != NPTR))
	{
		// the ATL has not yet been properly shut down
		ShutDown();
	}

	gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener(this);
}

///////////////////////////////////////////////////////////////////////////
bool CAudioTranslationLayer::Initialize(IAudioSystemImplementation* pImpl)
{
	if (pImpl == NPTR)
	{
		POOL_NEW(CAudioSystemImpl_NULL, pImpl);
	}

	if (pImpl != m_pImpl)
	{
		m_bIsRunning = false;

		ShutDown();
		SetpImpl(pImpl);

		// add the callback for the obstruction calculation
		gEnv->pPhysicalWorld->AddEventClient(
			EventPhysRWIResult::id,
			&CATLAudioObject::CPropagationProcessor::OnObstructionTest,
			1);

		m_bIsRunning = true;
	}

	// TODO: Rather parse a "default data" type XML to import ATL required controls!
	CAudioProxy::s_nObstructionCalcSwitchID											= static_cast<TAudioControlID>(AudioStringToID("ObstrOcclCalcType"));	
	CAudioProxy::s_aObstructionCalcStateIDs[eAOOCT_IGNORE]			= static_cast<TAudioSwitchStateID>(AudioStringToID("Ignore"));
	CAudioProxy::s_aObstructionCalcStateIDs[eAOOCT_SINGLE_RAY]	= static_cast<TAudioSwitchStateID>(AudioStringToID("SingleRay"));
	CAudioProxy::s_aObstructionCalcStateIDs[eAOOCT_MULTI_RAY]		= static_cast<TAudioSwitchStateID>(AudioStringToID("MultiRay"));

	return true;
}

///////////////////////////////////////////////////////////////////////////
bool CAudioTranslationLayer::ShutDown()
{
	m_bIsRunning = false;
	if (gEnv->pPhysicalWorld != NPTR)
	{
		// remove the callback for the obstruction calculation
		gEnv->pPhysicalWorld->RemoveEventClient(
			EventPhysRWIResult::id,
			&CATLAudioObject::CPropagationProcessor::OnObstructionTest,
			1);
	}

	ReleasepImpl();
	return true;
}

///////////////////////////////////////////////////////////////////////////
void CAudioTranslationLayer::ProcessRequest(CAudioRequestInternal& rRequest)
{
	EAudioRequestStatus eResult = eARS_NONE;

	switch (rRequest.pData->eRequestType)
	{
	case eART_AUDIO_OBJECT_REQUEST:
		{
			if (rRequest.nAudioObjectID != INVALID_AUDIO_OBJECT_ID)
			{
				CATLAudioObject* const pObject = m_oAudioObjectMgr.LookupID(rRequest.nAudioObjectID);

				if (pObject != NPTR)
				{
					eResult = ProcessAudioObjectRequest(pObject, rRequest.pData);
				}
				else
				{
					g_AudioLogger.Log(eALT_WARNING, "ATL received a request to a non-existent AudioObject: %u", rRequest.nAudioObjectID);
					eResult = eARS_FAILURE_INVALID_OBJECT_ID;
				}
			}
			else
			{
				eResult = ProcessGlobalObjectRequest(rRequest.pData);
			}

			break;
		}
	case eART_AUDIO_LISTENER_REQUEST:
		{
			if (rRequest.nAudioObjectID == INVALID_AUDIO_OBJECT_ID)
			{
				// No specific listener ID supplied, therefore we assume default listener ID.
				rRequest.nAudioObjectID = m_oAudioListenerMgr.GetDefaultListenerID();
			}

			CATLListenerObject* const pListener = m_oAudioListenerMgr.LookupID(rRequest.nAudioObjectID);

			if (pListener != NPTR)
			{
				eResult = ProcessAudioListenertRequest(pListener, rRequest.pData);
			}
			else
			{
				g_AudioLogger.Log(eALT_ERROR, "Could not find listener with ID: %u", rRequest.nAudioObjectID);
				assert(false);
			}

			break;
		}
	case eART_AUDIO_CALLBACK_MANAGER_REQUEST:
		{
			eResult = ProcessAudioCallbackManagerRequest(rRequest.pData);

			break;
		}
	case eART_AUDIO_MANAGER_REQUEST:
		{
			eResult = ProcessAudioManagerRequest(rRequest);

			break;
		}
	default:
		{
			g_AudioLogger.Log(eALT_ERROR, "Unknown audio request type: %d", static_cast<int>(rRequest.pData->eRequestType));
			assert(false);

			break;
		}
	}

	rRequest.eStatus = eResult;
}

///////////////////////////////////////////////////////////////////////////
void CAudioTranslationLayer::Update(float const fUpdateIntervalMS)
{
	UpdateSharedData();

	if (m_bIsRunning)
	{
		m_oAudioEventMgr.Update(fUpdateIntervalMS);
		m_oAudioObjectMgr.Update(fUpdateIntervalMS, m_oSharedData.m_oActiveListenerPosition);
		m_oAudioListenerMgr.Update(fUpdateIntervalMS);
		m_oFileCacheMgr.Update();

		m_pImpl->Update(fUpdateIntervalMS);
	}
}

///////////////////////////////////////////////////////////////////////////
bool CAudioTranslationLayer::GetAudioTriggerID(char const* const sAudioTriggerName, TAudioControlID& rAudioTriggerID) const
{
	TAudioControlID const nTriggerID = static_cast<TAudioControlID const>(AudioStringToID(sAudioTriggerName));

	if (stl::find_in_map(m_cTriggers, nTriggerID, NPTR) != NPTR)
	{
		rAudioTriggerID = nTriggerID;
	}
	else
	{
		rAudioTriggerID = INVALID_AUDIO_CONTROL_ID;
	}

	return (rAudioTriggerID != INVALID_AUDIO_CONTROL_ID);
}

///////////////////////////////////////////////////////////////////////////
bool CAudioTranslationLayer::GetAudioRtpcID(char const* const sAudioRtpcName, TAudioControlID& rAudioRtpcID) const
{
	TAudioControlID const nRtpcID = static_cast<TAudioControlID const>(AudioStringToID(sAudioRtpcName));

	if (stl::find_in_map(m_cRtpcs, nRtpcID, NPTR) != NPTR)
	{
		rAudioRtpcID = nRtpcID;
	}
	else
	{
		rAudioRtpcID = INVALID_AUDIO_CONTROL_ID;
	}

	return (rAudioRtpcID != INVALID_AUDIO_CONTROL_ID);
}

///////////////////////////////////////////////////////////////////////////
bool CAudioTranslationLayer::GetAudioSwitchID(char const* const sAudioStateName, TAudioControlID& rAudioSwitchID) const
{
	TAudioControlID const nSwitchID = static_cast<TAudioControlID const>(AudioStringToID(sAudioStateName));

	if (stl::find_in_map(m_cSwitches, nSwitchID, NPTR) != NPTR)
	{
		rAudioSwitchID = nSwitchID;
	}
	else
	{
		rAudioSwitchID = INVALID_AUDIO_CONTROL_ID;
	}

	return (rAudioSwitchID != INVALID_AUDIO_CONTROL_ID);
}

///////////////////////////////////////////////////////////////////////////
bool CAudioTranslationLayer::GetAudioSwitchStateID(
	TAudioControlID const nSwitchID, 
	char const* const sAudioSwitchStateName, 
	TAudioSwitchStateID& rAudioStateID) const
{
	rAudioStateID = INVALID_AUDIO_CONTROL_ID;

	CATLSwitch const* const pSwitch = stl::find_in_map(m_cSwitches, nSwitchID, NPTR);

	if (pSwitch != NPTR)
	{
		TAudioSwitchStateID const nStateID = static_cast<TAudioSwitchStateID const>(AudioStringToID(sAudioSwitchStateName));

		CATLSwitchState const* const pState = stl::find_in_map(pSwitch->cStates, nStateID, NPTR);

		if (pState != NPTR)
		{
			rAudioStateID = nStateID;
		}
	}

	return (rAudioStateID != INVALID_AUDIO_CONTROL_ID);
}

//////////////////////////////////////////////////////////////////////////
bool CAudioTranslationLayer::GetAudioPreloadRequestID(char const* const sAudioPreloadRequestName, TAudioPreloadRequestID& rAudioPreloadRequestID) const
{
	TAudioPreloadRequestID const nAudioPreloadRequestID = static_cast<TAudioPreloadRequestID const>(AudioStringToID(sAudioPreloadRequestName));

	if (stl::find_in_map(m_cPreloadRequests, nAudioPreloadRequestID, NPTR) != NPTR)
	{
		rAudioPreloadRequestID = nAudioPreloadRequestID;
	}
	else
	{
		rAudioPreloadRequestID = INVALID_AUDIO_PRELOAD_REQUEST_ID;
	}

	return (rAudioPreloadRequestID != INVALID_AUDIO_PRELOAD_REQUEST_ID);
}

///////////////////////////////////////////////////////////////////////////
bool CAudioTranslationLayer::GetAudioEnvironmentID(char const* const sAudioEnvironmentName, TAudioEnvironmentID& rAudioEnvironmentID) const
{
	TAudioEnvironmentID const nEnvironmentID = static_cast<TAudioControlID const>(AudioStringToID(sAudioEnvironmentName));

	if (stl::find_in_map(m_cEnvironments, nEnvironmentID, NPTR) != NPTR)
	{
		rAudioEnvironmentID = nEnvironmentID;
	}
	else
	{
		rAudioEnvironmentID = INVALID_AUDIO_CONTROL_ID;
	}

	return (rAudioEnvironmentID != INVALID_AUDIO_CONTROL_ID);
}

///////////////////////////////////////////////////////////////////////////
bool CAudioTranslationLayer::ReserveAudioObjectID(TAudioObjectID& rAudioObjectID)
{
	return m_oAudioObjectMgr.ReserveID(rAudioObjectID);
}

///////////////////////////////////////////////////////////////////////////
bool CAudioTranslationLayer::ReleaseAudioObjectID(TAudioObjectID const nAudioObjectID)
{
	bool const bSuccess = m_oAudioObjectMgr.ReleaseID(nAudioObjectID);

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	if (bSuccess)
	{
		m_oDebugNameStore.RemoveAudioObject(nAudioObjectID);
	}
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

	return bSuccess;
}

///////////////////////////////////////////////////////////////////////////
bool CAudioTranslationLayer::ReserveAudioListenerID(TAudioObjectID& rAudioObjectID)
{
	return m_oAudioListenerMgr.ReserveID(rAudioObjectID);
}

///////////////////////////////////////////////////////////////////////////
bool CAudioTranslationLayer::ReleaseAudioListenerID(TAudioObjectID const nAudioObjectID)
{
	return m_oAudioListenerMgr.ReleaseID(nAudioObjectID);
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioTranslationLayer::ParseControlsData(char const* const pConfigFolderPath, EATLDataScope const eDataScope)
{
	m_oXMLProcessor.ParseControlsData(pConfigFolderPath, eDataScope);

	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioTranslationLayer::ParsePreloadsData(char const* const pConfigFolderPath, EATLDataScope const eDataScope)
{
	m_oXMLProcessor.ParsePreloadsData(pConfigFolderPath, eDataScope);

	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioTranslationLayer::ClearControlsData(EATLDataScope const eDataScope)
{
	m_oXMLProcessor.ClearControlsData(eDataScope);

	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioTranslationLayer::ClearPreloadsData(EATLDataScope const eDataScope)
{
	m_oXMLProcessor.ClearPreloadsData(eDataScope);

	return eARS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
void CAudioTranslationLayer::NotifyListener(CAudioRequestInternal const& rRequest)
{
	if (rRequest.pOwner != NPTR)
	{
		TATLEnumFlagsType nSpecificAudioRequest = INVALID_AUDIO_ENUM_FLAG_TYPE;

		switch (rRequest.pData->eRequestType)
		{
		case eART_AUDIO_MANAGER_REQUEST:
			{
				SAudioManagerRequestDataInternalBase const* const pRequestDataBase = static_cast<SAudioManagerRequestDataInternalBase const* const>(rRequest.pData.get());
				nSpecificAudioRequest = static_cast<TATLEnumFlagsType>(pRequestDataBase->eType);

				break;
			}
		case eART_AUDIO_CALLBACK_MANAGER_REQUEST:
			{
				SAudioCallbackManagerRequestDataInternalBase const* const pRequestDataBase = static_cast<SAudioCallbackManagerRequestDataInternalBase const* const>(rRequest.pData.get());
				nSpecificAudioRequest = static_cast<TATLEnumFlagsType>(pRequestDataBase->eType);

				break;
			}
		case eART_AUDIO_OBJECT_REQUEST:
			{
				SAudioObjectRequestDataInternalBase const* const pRequestDataBase = static_cast<SAudioObjectRequestDataInternalBase const* const>(rRequest.pData.get());
				nSpecificAudioRequest = static_cast<TATLEnumFlagsType>(pRequestDataBase->eType);

				break;
			}
		case eART_AUDIO_LISTENER_REQUEST:
			{
				SAudioListenerRequestDataInternalBase const* const pRequestDataBase = static_cast<SAudioListenerRequestDataInternalBase const* const>(rRequest.pData.get());
				nSpecificAudioRequest = static_cast<TATLEnumFlagsType>(pRequestDataBase->eType);

				break;
			}
		default:
			{
				CryFatalError("Unknown request type during CAudioTranslationLayer::NotifyListener!");

				break;
			}
		}

		SAudioRequestInfo const oResult(ConvertToRequestResult(rRequest.eStatus), rRequest.pOwner, rRequest.pData->eRequestType, nSpecificAudioRequest);
		m_oAudioEventListenerMgr.NotifyListener(&oResult);
	}
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioTranslationLayer::ProcessAudioManagerRequest(CAudioRequestInternal const& rRequest)
{
	EAudioRequestStatus eResult = eARS_FAILURE;

	if (rRequest.pData)
	{
		SAudioManagerRequestDataInternalBase const* const pRequestDataBase = static_cast<SAudioManagerRequestDataInternalBase const*>(rRequest.pData.get());

		switch (pRequestDataBase->eType)
		{
		case eAMRT_RESERVE_AUDIO_OBJECT_ID:
			{
				SAudioManagerRequestDataInternal<eAMRT_RESERVE_AUDIO_OBJECT_ID> const* const pRequestData = static_cast<SAudioManagerRequestDataInternal<eAMRT_RESERVE_AUDIO_OBJECT_ID> const*>(rRequest.pData.get());
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
				eResult = BoolToARS(ReserveAudioObjectID(*pRequestData->pObjectID, pRequestData->sObjectName.c_str()));
#else
				eResult = BoolToARS(ReserveAudioObjectID(*pRequestData->pObjectID));
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
				break;
			}
		case eAMRT_ADD_REQUEST_LISTENER:
			{
				SAudioManagerRequestDataInternal<eAMRT_ADD_REQUEST_LISTENER> const* const pRequestData = static_cast<SAudioManagerRequestDataInternal<eAMRT_ADD_REQUEST_LISTENER> const*>(rRequest.pData.get());
				eResult = m_oAudioEventListenerMgr.AddRequestListener(pRequestData->func, pRequestData->pRequestOwner);

				break;
			}
		case eAMRT_REMOVE_REQUEST_LISTENER:
			{
				SAudioManagerRequestDataInternal<eAMRT_REMOVE_REQUEST_LISTENER> const* const pRequestData = static_cast<SAudioManagerRequestDataInternal<eAMRT_REMOVE_REQUEST_LISTENER> const*>(rRequest.pData.get());
				eResult = m_oAudioEventListenerMgr.RemoveRequestListener(pRequestData->func, pRequestData->pRequestOwner);

				break;
			}
		case eAMRT_INIT_AUDIO_SYSTEM:
			{
				eResult = m_pImpl->Init();

				break;
			}
		case eAMRT_SHUTDOWN_AUDIO_SYSTEM:
			{
				eResult = m_pImpl->ShutDown();

				break;
			}
		case eAMRT_REFRESH_AUDIO_SYSTEM:
			{
				SAudioManagerRequestDataInternal<eAMRT_REFRESH_AUDIO_SYSTEM> const* const pRequestData = static_cast<SAudioManagerRequestDataInternal<eAMRT_REFRESH_AUDIO_SYSTEM> const*>(rRequest.pData.get());
				eResult = RefreshAudioSystem(pRequestData->sLevelName);

				break;
			}
		case eAMRT_LOSE_FOCUS:
			{
				if (
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
						(g_SoundCVars.m_nIgnoreWindowFocus == 0) && 
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
						!m_bIsMuted)
				{
					CATLTrigger const* const pTrigger = stl::find_in_map(m_cTriggers, SATLInternalControlIDs::nLoseFocusTriggerID, NPTR);

					if (pTrigger != NPTR)
					{
						ActivateTrigger(m_pGlobalAudioObject, pTrigger, 0.0f);
					}
					else
					{
						assert(false);
					}

					eResult = m_pImpl->OnLoseFocus();
				}

				break;
			}
		case eAMRT_GET_FOCUS:
			{
				if (
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
					(g_SoundCVars.m_nIgnoreWindowFocus == 0) && 
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
					!m_bIsMuted)
				{
					eResult = m_pImpl->OnGetFocus();

					CATLTrigger const* const pTrigger = stl::find_in_map(m_cTriggers, SATLInternalControlIDs::nGetFocusTriggerID, NPTR);

					if (pTrigger != NPTR)
					{
						ActivateTrigger(m_pGlobalAudioObject, pTrigger, 0.0f);
					}
					else
					{
						assert(false);
					}
				}

				break;
			}
		case eAMRT_MUTE_ALL:
			{
				CATLTrigger const* const pTrigger = stl::find_in_map(m_cTriggers, SATLInternalControlIDs::nMuteAllTriggerID, NPTR);

				if (pTrigger != NPTR)
				{
					ActivateTrigger(m_pGlobalAudioObject, pTrigger, 0.0f);
				}
				else
				{
					assert(false);
				}

				eResult = m_pImpl->MuteAll();
				m_bIsMuted = (eResult == eARS_SUCCESS);

				break;
			}
		case eAMRT_UNMUTE_ALL:
			{
				eResult = m_pImpl->UnmuteAll();
				m_bIsMuted = (eResult != eARS_SUCCESS);

				CATLTrigger const* const pTrigger = stl::find_in_map(m_cTriggers, SATLInternalControlIDs::nUnmuteAllTriggerID, NPTR);

				if (pTrigger != NPTR)
				{
					ActivateTrigger(m_pGlobalAudioObject, pTrigger, 0.0f);
				}
				else
				{
					assert(false);
				}

				break;
			}
		case eAMRT_STOP_ALL_SOUNDS:
			{
				eResult = m_pImpl->StopAllSounds();

				break;
			}
		case eAMRT_PARSE_CONTROLS_DATA:
			{
				SAudioManagerRequestDataInternal<eAMRT_PARSE_CONTROLS_DATA> const* const pRequestData = static_cast<SAudioManagerRequestDataInternal<eAMRT_PARSE_CONTROLS_DATA> const*>(rRequest.pData.get());
				eResult = ParseControlsData(pRequestData->sConfigPath.c_str(), pRequestData->eDataScope);
				
				break;
			}
		case eAMRT_PARSE_PRELOADS_DATA:
			{
				SAudioManagerRequestDataInternal<eAMRT_PARSE_PRELOADS_DATA> const* const pRequestData = static_cast<SAudioManagerRequestDataInternal<eAMRT_PARSE_PRELOADS_DATA> const*>(rRequest.pData.get());
				eResult = ParsePreloadsData(pRequestData->sConfigPath.c_str(), pRequestData->eDataScope);

				break;
			}
		case eAMRT_CLEAR_CONTROLS_DATA:
			{
				SAudioManagerRequestDataInternal<eAMRT_CLEAR_CONTROLS_DATA> const* const pRequestData = static_cast<SAudioManagerRequestDataInternal<eAMRT_CLEAR_CONTROLS_DATA> const*>(rRequest.pData.get());
				eResult = ClearControlsData(pRequestData->eDataScope);
				
				break;
			}
		case eAMRT_CLEAR_PRELOADS_DATA:
			{
				SAudioManagerRequestDataInternal<eAMRT_CLEAR_PRELOADS_DATA> const* const pRequestData = static_cast<SAudioManagerRequestDataInternal<eAMRT_CLEAR_PRELOADS_DATA> const*>(rRequest.pData.get());
				eResult = ClearPreloadsData(pRequestData->eDataScope);

				break;
			}
		case eAMRT_PRELOAD_SINGLE_REQUEST:
			{
				SAudioManagerRequestDataInternal<eAMRT_PRELOAD_SINGLE_REQUEST> const* const pRequestData = static_cast<SAudioManagerRequestDataInternal<eAMRT_PRELOAD_SINGLE_REQUEST> const*>(rRequest.pData.get());
				eResult = m_oFileCacheMgr.TryLoadRequest(pRequestData->nPreloadRequest, ((rRequest.nFlags & eARF_EXECUTE_BLOCKING) != 0), pRequestData->bAutoLoadOnly);

				break;
			}
		case eAMRT_UNLOAD_SINGLE_REQUEST:
			{
				SAudioManagerRequestDataInternal<eAMRT_UNLOAD_SINGLE_REQUEST> const* const pRequestData = static_cast<SAudioManagerRequestDataInternal<eAMRT_UNLOAD_SINGLE_REQUEST> const*>(rRequest.pData.get());
				eResult = m_oFileCacheMgr.TryUnloadRequest(pRequestData->nPreloadRequest);

				break;
			}
		case eAMRT_UNLOAD_AFCM_DATA_BY_SCOPE:
			{
				SAudioManagerRequestDataInternal<eAMRT_UNLOAD_AFCM_DATA_BY_SCOPE> const* const pRequestData = static_cast<SAudioManagerRequestDataInternal<eAMRT_UNLOAD_AFCM_DATA_BY_SCOPE> const*>(rRequest.pData.get());
				eResult = m_oFileCacheMgr.UnloadDataByScope(pRequestData->eDataScope);

				break;
			}
		case eAMRT_CHANGE_LANGUAGE:
			{
				SetImplLanguage();

				m_oFileCacheMgr.UpdateLocalizedFileCacheEntries();

				break;
			}
		case eAMRT_DRAW_DEBUG_INFO:
			{
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
				DrawAudioSystemDebugInfo();
				eResult = eARS_SUCCESS;
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

				break;
			}
		case eAMRT_NONE:
			{
				eResult = eARS_SUCCESS;

				break;
			}
		default:
			{
				eResult = eARS_FAILURE_INVLAID_REQUEST;
				g_AudioLogger.Log(eALT_WARNING, "ATL received an unknown AudioManager request: %u", pRequestDataBase->eType);

				break;
			}
		}
	}

	return eResult;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioTranslationLayer::ProcessAudioCallbackManagerRequest(SAudioRequestDataInternal const* pPassedRequestData)
{
	EAudioRequestStatus eResult = eARS_FAILURE;

	if (pPassedRequestData != NPTR)
	{
		SAudioCallbackManagerRequestDataInternalBase const* const pRequestDataBase = static_cast<SAudioCallbackManagerRequestDataInternalBase const*>(pPassedRequestData);

		switch (pRequestDataBase->eType)
		{
		case eACMRT_REPORT_FINISHED_EVENT:
			{
				SAudioCallbackManagerRequestDataInternal<eACMRT_REPORT_FINISHED_EVENT> const* const pRequestData = static_cast<SAudioCallbackManagerRequestDataInternal<eACMRT_REPORT_FINISHED_EVENT> const*>(pPassedRequestData);
				CATLEvent* const pEvent = m_oAudioEventMgr.LookupID(pRequestData->nEventID);

				if (pEvent != NPTR)
				{
					if (pEvent->m_nObjectID != m_nGloabalAudioObjectID)
					{
						m_oAudioObjectMgr.ReportFinishedEvent(pEvent, pRequestData->bSuccess);
					}
					else
					{
						m_pGlobalAudioObject->ReportFinishedEvent(pEvent, pRequestData->bSuccess);
					}

					m_oAudioEventMgr.ReleaseEvent(pEvent);
				}

				eResult = eARS_SUCCESS;

				break;
			}
		case eACMRT_REPORT_PROCESSED_OBSTRUCTION_RAY:
			{
				SAudioCallbackManagerRequestDataInternal<eACMRT_REPORT_PROCESSED_OBSTRUCTION_RAY> const* const pRequestData = 
					static_cast<SAudioCallbackManagerRequestDataInternal<eACMRT_REPORT_PROCESSED_OBSTRUCTION_RAY> const* const>(pPassedRequestData);

				m_oAudioObjectMgr.ReportObstructionRay(pRequestData->nObjectID, pRequestData->nRayID);

				eResult = eARS_SUCCESS;

				break;
			}
		case eACMRT_NONE:
			{
				eResult = eARS_SUCCESS;

				break;
			}
		default:
			{
				eResult = eARS_FAILURE_INVLAID_REQUEST;
				g_AudioLogger.Log(eALT_WARNING, "ATL received an unknown AudioCallbackManager request: %u", pRequestDataBase->eType);

				break;
			}
		}
	}

	return eResult;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioTranslationLayer::ProcessGlobalObjectRequest(SAudioRequestDataInternal const* pPassedRequestData)
{
	return ProcessAudioObjectRequest(m_pGlobalAudioObject, pPassedRequestData);
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioTranslationLayer::ProcessAudioObjectRequest(CATLAudioObjectBase* const pObject, SAudioRequestDataInternal const* const pPassedRequestData)
{
	EAudioRequestStatus eResult = eARS_FAILURE;

	if (pPassedRequestData != NPTR)
	{
		SAudioObjectRequestDataInternalBase const* const pBaseRequestData = static_cast<SAudioObjectRequestDataInternalBase const*>(pPassedRequestData);

		switch (pBaseRequestData->eType)
		{
		case eAORT_PREPARE_TRIGGER:
			{
				SAudioObjectRequestDataInternal<eAORT_PREPARE_TRIGGER> const* const pRequestData = 
					static_cast<SAudioObjectRequestDataInternal<eAORT_PREPARE_TRIGGER> const* const>(pPassedRequestData);

				CATLTrigger const* const pTrigger = stl::find_in_map(m_cTriggers, pRequestData->nTriggerID, NPTR);
				
				if (pTrigger != NPTR)
				{
					eResult = PrepUnprepTriggerAsync(pObject, pTrigger, true);
				}
				else
				{
					eResult = eARS_FAILURE_INVLAID_CONTROL_ID;
				}
				
				break;
			}
		case eAORT_UNPREPARE_TRIGGER:
			{
				SAudioObjectRequestDataInternal<eAORT_UNPREPARE_TRIGGER> const* const pRequestData = 
					static_cast<SAudioObjectRequestDataInternal<eAORT_UNPREPARE_TRIGGER> const* const>(pPassedRequestData);

				CATLTrigger const* const pTrigger = stl::find_in_map(m_cTriggers, pRequestData->nTriggerID, NPTR);

				if (pTrigger != NPTR)
				{
					eResult = PrepUnprepTriggerAsync(pObject, pTrigger, false);
				}
				else
				{
					eResult = eARS_FAILURE_INVLAID_CONTROL_ID;
				}

				break;
			}
		case eAORT_EXECUTE_TRIGGER:
			{
				SAudioObjectRequestDataInternal<eAORT_EXECUTE_TRIGGER> const* const pRequestData = 
					static_cast<SAudioObjectRequestDataInternal<eAORT_EXECUTE_TRIGGER> const* const>(pPassedRequestData);

				CATLTrigger const* const pTrigger = stl::find_in_map(m_cTriggers, pRequestData->nTriggerID, NPTR);
				
				if (pTrigger != NPTR)
				{
					eResult = ActivateTrigger(
						pObject,
						pTrigger,
						pRequestData->fTimeUntilRemovalInMS,
						pRequestData->pCallback,
						pRequestData->pCallbackCookie,
						pRequestData->nFlags);
				}
				else
				{
					eResult = eARS_FAILURE_INVLAID_CONTROL_ID;
				}
				
				break;
			}
		case eAORT_STOP_TRIGGER:
			{
				SAudioObjectRequestDataInternal<eAORT_STOP_TRIGGER> const* const pRequestData = 
					static_cast<SAudioObjectRequestDataInternal<eAORT_STOP_TRIGGER> const* const>(pPassedRequestData);

				CATLTrigger const* const pTrigger = stl::find_in_map(m_cTriggers, pRequestData->nTriggerID, NPTR);

				if (pTrigger != NPTR)
				{
					eResult = StopTrigger(pObject, pTrigger);
				}
				else
				{
					eResult = eARS_FAILURE_INVLAID_CONTROL_ID;
				}

				break;
			}
		case eAORT_STOP_ALL_TRIGGERS:
			{
				StopAllTriggers(pObject);

				eResult = eARS_SUCCESS;

				break;
			}
		case eAORT_SET_POSITION:
			{
				if (pObject->HasPosition())
				{
					SAudioObjectRequestDataInternal<eAORT_SET_POSITION> const* const pRequestData = 
						static_cast<SAudioObjectRequestDataInternal<eAORT_SET_POSITION> const* const>(pPassedRequestData);

					CATLAudioObject* const pPositionedObject = static_cast<CATLAudioObject*>(pObject);

					eResult = m_pImpl->SetPosition(pPositionedObject->GetImplDataPtr(), pRequestData->oPosition);

					if (eResult == eARS_SUCCESS)
					{
						pPositionedObject->SetPosition(pRequestData->oPosition);
					}
				}
				else
				{
					g_AudioLogger.Log(eALT_WARNING, "ATL received a request to set a position on a global object");
				}

				break;
			}
		case eAORT_SET_RTPC_VALUE:
			{
				eResult = eARS_FAILURE_INVLAID_CONTROL_ID;

				SAudioObjectRequestDataInternal<eAORT_SET_RTPC_VALUE> const* const pRequestData = 
					static_cast<SAudioObjectRequestDataInternal<eAORT_SET_RTPC_VALUE> const* const>(pPassedRequestData);

				CATLRtpc const* const pRtpc = stl::find_in_map(m_cRtpcs, pRequestData->nControlID, NPTR);
				
				if (pRtpc != NPTR)
				{
					eResult = SetRtpc(pObject, pRtpc, pRequestData->fValue);
				}

				break;
			}
		case eAORT_SET_SWITCH_STATE:
			{
				eResult = eARS_FAILURE_INVLAID_CONTROL_ID;
				SAudioObjectRequestDataInternal<eAORT_SET_SWITCH_STATE> const* const pRequestData = 
					static_cast<SAudioObjectRequestDataInternal<eAORT_SET_SWITCH_STATE> const* const>(pPassedRequestData);

				CATLSwitch const* const pSwitch = stl::find_in_map(m_cSwitches, pRequestData->nSwitchID, NPTR);
				
				if (pSwitch != NPTR)
				{
					CATLSwitchState const* const pState = stl::find_in_map(pSwitch->cStates, pRequestData->nStateID, NPTR);
					
					if (pState != NPTR)
					{
						eResult = SetSwitchState(pObject, pState);
					}
				}
				
				break;
			}
		case eAORT_SET_VOLUME:
			{
				eResult = eARS_FAILURE_INVLAID_CONTROL_ID;
				//TODO

				break;
			}
		case eAORT_SET_ENVIRONMENT_AMOUNT:
			{
				if (pObject->HasPosition())
				{
					eResult = eARS_FAILURE_INVLAID_CONTROL_ID;
					SAudioObjectRequestDataInternal<eAORT_SET_ENVIRONMENT_AMOUNT> const* const pRequestData = 
						static_cast<SAudioObjectRequestDataInternal<eAORT_SET_ENVIRONMENT_AMOUNT> const* const>(pPassedRequestData);

					CATLAudioEnvironment const* const pEnvironment = stl::find_in_map(m_cEnvironments, pRequestData->nEnvironmentID, NPTR);

					if (pEnvironment != NPTR)
					{
						eResult = SetEnvironment(pObject, pEnvironment, pRequestData->fAmount);
					}
				}
				else
				{
					g_AudioLogger.Log(eALT_WARNING, "ATL received a request to set an environment on a global object");
				}

				break;
			}
		case eAORT_RESET_ENVIRONMENTS:
			{
				eResult = ResetEnvironments(pObject);
				break;
			}
		case eAORT_RELEASE_OBJECT:
			{
				eResult = eARS_FAILURE;

				TAudioObjectID const nObjectID = pObject->GetID();

				if (nObjectID != m_nGloabalAudioObjectID)
				{
					if (ReleaseAudioObjectID(nObjectID))
					{	
						eResult = eARS_SUCCESS;
					}
				}
				else
				{
					g_AudioLogger.Log(eALT_WARNING, "ATL received a request to release the GlobalAudioObject");
				}

				break;
			}
		case eAORT_NONE:
			{
				eResult = eARS_SUCCESS;
				
				break;
			}
		default:
			{
				eResult = eARS_FAILURE_INVLAID_REQUEST;
				g_AudioLogger.Log(eALT_WARNING, "ATL received an unknown AudioObject request type: %u", pBaseRequestData->eType);
				
				break;
			}
		}
	}

	return eResult;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioTranslationLayer::ProcessAudioListenertRequest(CATLListenerObject* pListener, SAudioRequestDataInternal const* pPassedRequestData)
{
	EAudioRequestStatus eResult = eARS_FAILURE;

	if (pPassedRequestData != NPTR)
	{
		SAudioListenerRequestDataInternalBase const * const pBaseRequestData = 
			static_cast<SAudioListenerRequestDataInternalBase const * const>(pPassedRequestData);

		switch (pBaseRequestData->eType)
		{
		case eALRT_SET_POSITION:
			{
				SAudioListenerRequestDataInternal<eALRT_SET_POSITION> const* const pRequestData =
					static_cast<SAudioListenerRequestDataInternal<eALRT_SET_POSITION> const* const>(pPassedRequestData);

				eResult = m_pImpl->SetListenerPosition(pListener->m_pImplData, pRequestData->oNewPosition);

				if (eResult == eARS_SUCCESS)
				{
					pListener->oPosition = pRequestData->oNewPosition;
				}
				break;
			}
		case eALRT_NONE:
			{
				eResult = eARS_SUCCESS;

				break;
			}
		default:
			{
				eResult = eARS_FAILURE_INVLAID_REQUEST;

				g_AudioLogger.Log(eALT_WARNING, "ATL received an unknown AudioListener request type: %u", pBaseRequestData->eType);

				break;
			}
		}
	}

	return eResult;
}

///////////////////////////////////////////////////////////////////////////
void CAudioTranslationLayer::SetpImpl(IAudioSystemImplementation* const pImplNew)
{
	m_oAudioObjectMgr.Init(pImplNew);
	m_oAudioEventMgr.Init(pImplNew);
	m_oAudioListenerMgr.Init(pImplNew);
	m_oXMLProcessor.Init(pImplNew);
	m_oFileCacheMgr.Init(pImplNew);

	IATLAudioObjectData* const pGlobalObjectData = pImplNew->NewGlobalAudioObjectData(m_nGloabalAudioObjectID);
	POOL_NEW(CATLGlobalAudioObject, m_pGlobalAudioObject)(m_nGloabalAudioObjectID, pGlobalObjectData);

	m_pImpl = pImplNew;

	SetImplLanguage();

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	m_sImplementationNameString = pImplNew->GetImplementationNameString();
#endif //INCLUDE_AUDIO_PRODUCTION_CODE
}

///////////////////////////////////////////////////////////////////////////
void CAudioTranslationLayer::ReleasepImpl()
{
	if (m_pImpl != NPTR)
	{	
		m_oXMLProcessor.ClearControlsData(eADS_ALL);
		m_oXMLProcessor.ClearPreloadsData(eADS_ALL);

		m_pImpl->DeleteAudioObjectData(m_pGlobalAudioObject->GetImplDataPtr());
		POOL_FREE(m_pGlobalAudioObject);

		m_oAudioObjectMgr.Release();
		m_oAudioListenerMgr.Release();
		m_oAudioEventMgr.Release();
		m_oFileCacheMgr.Release();

		m_pImpl->ShutDown();// Shut down the middleware.
		m_pImpl->Release();	// Release the Engine-specific data. 
		m_pImpl = NPTR;
	}
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioTranslationLayer::PrepUnprepTriggerAsync(
	CATLAudioObjectBase* const pAudioObject, 
	CATLTrigger const* const pTrigger,
	bool const bPrepare)
{
	EAudioRequestStatus eResult = eARS_FAILURE;

	TAudioObjectID const nATLObjectID = pAudioObject->GetID();
	TAudioControlID const nATLTriggerID = pTrigger->GetID();

	CATLTrigger::TImplPtrVec::const_iterator iTriggerImpl = pTrigger->m_cImplPtrs.begin();
	CATLTrigger::TImplPtrVec::const_iterator const iTriggerImplEnd = pTrigger->m_cImplPtrs.end();

	TObjectTriggerImplStates const& rTriggerImplStates = pAudioObject->GetTriggerImpls();

	for (; iTriggerImpl != iTriggerImplEnd; ++iTriggerImpl)
	{
		CATLTriggerImpl const* const pTriggerImpl = *iTriggerImpl;
		
		TATLEnumFlagsType nTriggerImplFlags = 0;
		TObjectTriggerImplStates::const_iterator iPlace= rTriggerImplStates.end();
		if (FindPlaceConst(rTriggerImplStates, pTriggerImpl->m_nATLID, iPlace))
		{
			nTriggerImplFlags = iPlace->second.nFlags;
		}

		EATLSubsystem const eReceiver = pTriggerImpl->GetReceiver();
		CATLEvent* pEvent = m_oAudioEventMgr.GetEvent(eReceiver);

		EAudioRequestStatus ePrepUnprepResult = eARS_FAILURE;

		switch (eReceiver)
		{
		case eAS_AUDIO_SYSTEM_IMPLEMENTATION:
			{
				if (bPrepare)
				{
					if (((nTriggerImplFlags & eATS_PREPARED) == 0) && ((nTriggerImplFlags & eATS_LOADING) == 0))
					{
						ePrepUnprepResult = m_pImpl->PrepareTriggerAsync(
							pAudioObject->GetImplDataPtr(),
							pTriggerImpl->m_pImplData,
							pEvent->m_pImplData);
					}
				}
				else
				{
					if (((nTriggerImplFlags & eATS_PREPARED) != 0) && ((nTriggerImplFlags & eATS_UNLOADING) == 0))
					{
						ePrepUnprepResult = m_pImpl->UnprepareTriggerAsync(
							pAudioObject->GetImplDataPtr(),
							pTriggerImpl->m_pImplData,
							pEvent->m_pImplData);
					}
				}

				if (ePrepUnprepResult ==  eARS_SUCCESS)
				{
					pEvent->m_nObjectID = nATLObjectID;
					pEvent->m_nTriggerID = pTriggerImpl->m_nATLID;
					pEvent->m_nTriggerImplID = pTriggerImpl->m_nATLID;

					pEvent->m_eType = bPrepare ? eAET_PREPARE : eAET_UNPREPARE;
				}

				break;
			}
		case eAS_ATL_INTERNAL:
			{
				//TODO: handle this

				break;
			}
		default:
			{
				assert(false);//unknown ATLRecipient

				break;
			}
		}

		if (ePrepUnprepResult == eARS_SUCCESS)
		{
			pEvent->SetDataScope(pTrigger->GetDataScope());
			pAudioObject->ReportStartedEvent(pEvent);
			pAudioObject->IncrementRefCount();
			eResult = eARS_SUCCESS;// if at least one event fires, it is a success
		}
		else
		{
			m_oAudioEventMgr.ReleaseEvent(pEvent);
		}
	}

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	if (eResult != eARS_SUCCESS)
	{
		// No TriggerImpl produced an active event.
		g_AudioLogger.Log(eALT_WARNING, "PrepUnprepTriggerAsync failed on AudioObject \"%s\" (ID: %u)", m_oDebugNameStore.LookupAudioObjectName(nATLObjectID), nATLObjectID);
	}
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

	return eResult;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioTranslationLayer::ActivateTrigger(
	CATLAudioObjectBase* const pAudioObject, 
	CATLTrigger const* const pTrigger, 
	float const fTimeUntilRemovalMS,
	TTriggerFinishedCallback const pCallback /* = NPTR */,
	void* const pCallbackCookie /* = NPTR */,
	TATLEnumFlagsType const nFlags /* = 0 */)
{
	EAudioRequestStatus eResult = eARS_FAILURE;
	TAudioObjectID const nATLObjectID = pAudioObject->GetID();
	TAudioControlID const nATLTriggerID = pTrigger->GetID();

	if (pAudioObject->HasPosition())
	{
		// If the AudioObject uses Obstruction/Occlusion then set the values before activating the trigger.
		CATLAudioObject* const pPositionedAudioObject = static_cast<CATLAudioObject*>(pAudioObject);

		if (pPositionedAudioObject->CanRunObstructionOcclusion() && !pPositionedAudioObject->HasActiveEvents())
		{
			pPositionedAudioObject->ResetObstructionOcclusion(m_oSharedData.m_oActiveListenerPosition);
		}
	}

	TAudioTriggerInstanceID nTriggerInstanceID = m_nTriggerInstanceIDCounter;

	CATLTrigger::TImplPtrVec::const_iterator iTriggerImpl = pTrigger->m_cImplPtrs.begin();
	CATLTrigger::TImplPtrVec::const_iterator const iTriggerImplEnd = pTrigger->m_cImplPtrs.end();

	for (; iTriggerImpl != iTriggerImplEnd; ++iTriggerImpl)
	{
		CATLTriggerImpl const* const pTriggerImpl = *iTriggerImpl;

		EATLSubsystem const eReceiver = (*iTriggerImpl)->GetReceiver();
		CATLEvent* pEvent = m_oAudioEventMgr.GetEvent(eReceiver);

		EAudioRequestStatus eActivateResult = eARS_FAILURE;

		switch (eReceiver)
		{
		case eAS_AUDIO_SYSTEM_IMPLEMENTATION:
			{
				
				eActivateResult = m_pImpl->ActivateTrigger(
					pAudioObject->GetImplDataPtr(),
					pTriggerImpl->m_pImplData,
					pEvent->m_pImplData);
				
				break;
			}
		case eAS_ATL_INTERNAL:
			{
				eActivateResult = ActivateInternalTrigger(pAudioObject,
					pTriggerImpl->m_pImplData,
					pEvent->m_pImplData);
				
				break;
			}
		default:
			{
				assert(false);//unknown ATLRecipient
				
				break;
			}
		}

		if (eActivateResult == eARS_SUCCESS)
		{
			pEvent->m_nObjectID = nATLObjectID;
			pEvent->m_nTriggerID = nATLTriggerID;
			pEvent->m_nTriggerImplID = pTriggerImpl->m_nATLID;
			pEvent->m_nTriggerInstanceID = nTriggerInstanceID;
			pEvent->m_eType = eAET_PLAY;
			pEvent->SetDataScope(pTrigger->GetDataScope());

			pAudioObject->ReportStartedEvent(pEvent);	// Sets eATS_STARTING on this TriggerInstance to avoid reporting TriggerFinished while
																								// the events are being started.
			pAudioObject->IncrementRefCount();
			eResult = eARS_SUCCESS;// If at least one event fires, it is a success: the trigger has been activated.
		}
		else
		{
			m_oAudioEventMgr.ReleaseEvent(pEvent);
		}
	}

	if (eResult == eARS_SUCCESS)
	{
		++m_nTriggerInstanceIDCounter;
		pAudioObject->ReportStartedTriggerInstance(nTriggerInstanceID, pCallback, pCallbackCookie, nFlags);// Unsets eATS_STARTING on this TriggerInstance.
	}
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	else
	{
		// No TriggerImpl generated an active event.
		g_AudioLogger.Log(eALT_WARNING, "Trigger \"%s\" failed on AudioObject \"%s\" (ID: %u)", m_oDebugNameStore.LookupAudioTriggerName(nATLTriggerID), m_oDebugNameStore.LookupAudioObjectName(nATLObjectID), nATLObjectID);
	}
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
	
	return eResult;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioTranslationLayer::StopTrigger(
	CATLAudioObjectBase* const pAudioObject,
	CATLTrigger const* const pTrigger)
{
	EAudioRequestStatus eResult = eARS_FAILURE;

	TAudioObjectID const nATLObjectID = pAudioObject->GetID();
	TAudioControlID const nATLTriggerID = pTrigger->GetID();

	TObjectEventSet rEvents = pAudioObject->GetActiveEvents();
	TObjectEventSet::const_iterator iEvent = rEvents.begin();
	TObjectEventSet::const_iterator const iEventEnd = rEvents.end();

	for (; iEvent != iEventEnd; ++iEvent)
	{
		CATLEvent const* const pEvent = m_oAudioEventMgr.LookupID(*iEvent);

		if ((pEvent != NPTR) && (pEvent->m_eType == eAET_PLAY) && (pEvent->m_nTriggerID == nATLTriggerID))
		{
			switch (pEvent->m_eSender)
			{
			case eAS_AUDIO_SYSTEM_IMPLEMENTATION:
				{
					eResult = m_pImpl->StopEvent(pAudioObject->GetImplDataPtr(), pEvent->m_pImplData);

					break;
				}
			case eAS_ATL_INTERNAL:
				{
					eResult = StopInternalEvent(pAudioObject, pEvent->m_pImplData);

					break;
				}
			default:
				{
					assert(false);//unknown ATLRecipient

					break;
				}
			}
		}
	}

	return eResult;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioTranslationLayer::StopAllTriggers(CATLAudioObjectBase* const pAudioObject)
{
	return m_pImpl->StopAllEvents(pAudioObject->GetImplDataPtr());
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioTranslationLayer::SetSwitchState(
	CATLAudioObjectBase* const pAudioObject, 
	CATLSwitchState const* const pState)
{
	EAudioRequestStatus eResult = eARS_FAILURE;

	CATLSwitchState::TImplPtrVec::const_iterator iStateImpl = pState->m_cImplPtrs.begin();
	CATLSwitchState::TImplPtrVec::const_iterator const iStateImplEnd = pState->m_cImplPtrs.end();

	for (; iStateImpl != iStateImplEnd; ++iStateImpl)
	{
		EATLSubsystem const eReceiver = (*iStateImpl)->GetReceiver();
		EAudioRequestStatus eSetStateResult = eARS_FAILURE;

		switch (eReceiver)
		{
		case eAS_AUDIO_SYSTEM_IMPLEMENTATION:
			{
				eSetStateResult = m_pImpl->SetSwitchState(pAudioObject->GetImplDataPtr(), (*iStateImpl)->m_pImplData);
				
				break;
			}
		case eAS_ATL_INTERNAL:
			{
				eSetStateResult = SetInternalSwitchState(pAudioObject, (*iStateImpl)->m_pImplData);
				
				break;
			}
		default:
			{
				assert(false);//unknown ATLRecipient
				
				break;
			}
		}
		
		if (eSetStateResult == eARS_SUCCESS) 
		{
			eResult = eARS_SUCCESS;// if at least one of the implementations is set successfully, it is a success
		}
	}

	if (eResult == eARS_SUCCESS)
	{
		pAudioObject->SetSwitchState(pState->GetParentID(), pState->GetID());
	}
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	else
	{
		char const* const sSwitchName				= m_oDebugNameStore.LookupAudioObjectName(pState->GetParentID());
		char const* const sSwitchStateName	= m_oDebugNameStore.LookupAudioObjectName(pState->GetID());
		char const* const sAudioObjectName	= m_oDebugNameStore.LookupAudioObjectName(pAudioObject->GetID());
		g_AudioLogger.Log(eALT_WARNING, "Failed to set the ATLSwitch \"%s\" to ATLSwitchState \"%s\" on AudioObject \"%s\" (ID: %u)", sSwitchName, sSwitchStateName, sAudioObjectName, pAudioObject->GetID());
	}
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

	return eResult;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioTranslationLayer::SetRtpc(
	CATLAudioObjectBase* const pAudioObject, 
	CATLRtpc const* const pRtpc,
	float const fValue)
{
	EAudioRequestStatus eResult = eARS_FAILURE;

	CATLRtpc::TImplPtrVec::const_iterator iRtpcImpl = pRtpc->m_cImplPtrs.begin();
	CATLRtpc::TImplPtrVec::const_iterator const iRtpcImplEnd = pRtpc->m_cImplPtrs.end();

	for (; iRtpcImpl != iRtpcImplEnd; ++iRtpcImpl)
	{
		EATLSubsystem const eReceiver = (*iRtpcImpl)->GetReceiver();
		EAudioRequestStatus eSetRtpcResult = eARS_FAILURE;

		switch (eReceiver)
		{
		case eAS_AUDIO_SYSTEM_IMPLEMENTATION:
			{
				eSetRtpcResult = m_pImpl->SetRtpc(pAudioObject->GetImplDataPtr(), (*iRtpcImpl)->m_pImplData, fValue);

				break;
			}
		case eAS_ATL_INTERNAL:
			{
				eSetRtpcResult = SetInternalRtpc(pAudioObject, (*iRtpcImpl)->m_pImplData, fValue);

				break;
			}
		default:
			{
				assert(false);//unknown ATLRecipient

				break;
			}
		}

		if (eSetRtpcResult == eARS_SUCCESS) 
		{
			eResult = eARS_SUCCESS;// if at least one of the implementations is set successfully, it is a success
		}
	}

	if (eResult == eARS_SUCCESS)
	{
		pAudioObject->SetRtpc(pRtpc->GetID(), fValue);
	}
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	else
	{
		char const* const sRtpcName					= m_oDebugNameStore.LookupAudioObjectName(pRtpc->GetID());
		char const* const sAudioObjectName	= m_oDebugNameStore.LookupAudioObjectName(pAudioObject->GetID());
		g_AudioLogger.Log(eALT_WARNING, "Failed to set the ATLRtpc \"%s\" to %f on AudioObject \"%s\" (ID: %u)", sRtpcName, fValue, sAudioObjectName, pAudioObject->GetID());
	}
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

	return eResult;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioTranslationLayer::SetEnvironment(
	CATLAudioObjectBase* const pAudioObject,
	CATLAudioEnvironment const* const pEnvironment,
	float const fAmount)
{
	EAudioRequestStatus eResult = eARS_FAILURE;

	CATLAudioEnvironment::TImplPtrVec::const_iterator iEnvImpl = pEnvironment->m_cImplPtrs.begin();
	CATLAudioEnvironment::TImplPtrVec::const_iterator const iEnvImplEnd = pEnvironment->m_cImplPtrs.end();

	for (; iEnvImpl != iEnvImplEnd; ++iEnvImpl)
	{
		EATLSubsystem const eReceiver = (*iEnvImpl)->GetReceiver();
		EAudioRequestStatus eSetEnvResult = eARS_FAILURE;

		switch (eReceiver)
		{
		case eAS_AUDIO_SYSTEM_IMPLEMENTATION:
			{
				eSetEnvResult = m_pImpl->SetEnvironment(pAudioObject->GetImplDataPtr(), (*iEnvImpl)->m_pImplData, fAmount);

				break;
			}
		case eAS_ATL_INTERNAL:
			{
				eSetEnvResult = SetInternalEnvironment(pAudioObject, (*iEnvImpl)->m_pImplData, fAmount);

				break;
			}
		default:
			{
				assert(false);//unknown ATLRecipient

				break;
			}
		}

		if (eSetEnvResult == eARS_SUCCESS) 
		{
			eResult = eARS_SUCCESS;// if at least one of the implementations is set successfully, it is a success
		}
	}

	if (eResult == eARS_SUCCESS)
	{
		pAudioObject->SetEnvironmentAmount(pEnvironment->GetID(), fAmount);
	}
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	else
	{
		char const* const sEnvironmentName	= m_oDebugNameStore.LookupAudioObjectName(pEnvironment->GetID());
		char const* const sAudioObjectName	= m_oDebugNameStore.LookupAudioObjectName(pAudioObject->GetID());
		g_AudioLogger.Log(eALT_WARNING, "Failed to set the ATLAudioEnvironment \"%s\" to %f on AudioObject \"%s\" (ID: %u)", sEnvironmentName, fAmount, sAudioObjectName, pAudioObject->GetID());
	}
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

	return eResult;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioTranslationLayer::ResetEnvironments(CATLAudioObjectBase* const pAudioObject)
{
	TObjectEnvironmentMap const rEnvironments			= pAudioObject->GetEnvironments();
	TObjectEnvironmentMap::const_iterator iEnv		= rEnvironments.begin();
	TObjectEnvironmentMap::const_iterator iEnvEnd	= rEnvironments.end();

	EAudioRequestStatus eResult = eARS_SUCCESS;

	for (; iEnv != iEnvEnd; ++iEnv)
	{
		CATLAudioEnvironment const* const pEnvironment = stl::find_in_map(m_cEnvironments, iEnv->first, NPTR);

		if (pEnvironment != NPTR)
		{
			EAudioRequestStatus const eSetEnvResult = SetEnvironment(pAudioObject, pEnvironment, 0.0f);

			if (eSetEnvResult != eARS_SUCCESS)
			{
				// If setting at least one Environment fails, we consider this a failure.
				eResult = eARS_FAILURE;
			}
		}
	}

	if (eResult == eARS_SUCCESS)
	{
		pAudioObject->ClearEnvironments();
	}
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	else
	{
		TAudioObjectID const nObjectID = pAudioObject->GetID();

		g_AudioLogger.Log(
			eALT_WARNING,
			"Failed to Reset AudioEnvironments on AudioObject \"%s\" (ID: %u)",
			m_oDebugNameStore.LookupAudioObjectName(nObjectID),
			nObjectID);
	}
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

	return eResult;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioTranslationLayer::ActivateInternalTrigger(
	CATLAudioObjectBase* const pAudioObject,
	IATLTriggerImplData const* const pTriggerData,
	IATLEventData* const pEventData)
{
	//TODO implement
	return eARS_FAILURE;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioTranslationLayer::StopInternalEvent(
	CATLAudioObjectBase* const pAudioObject,
	IATLEventData const* const pEventData)
{
	//TODO implement
	return eARS_FAILURE;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioTranslationLayer::StopAllInternalEvents(CATLAudioObjectBase* const pAudioObject)
{
	//TODO implement
	return eARS_FAILURE;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioTranslationLayer::SetInternalRtpc(
	CATLAudioObjectBase* const pAudioObject,
	IATLRtpcImplData const* const pRtpcData,
	float const fValue)
{
	//TODO implement
	return eARS_FAILURE;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioTranslationLayer::SetInternalSwitchState(
	CATLAudioObjectBase* const pAudioObject,
	IATLSwitchStateImplData const* const pSwitchStateData)
{
	SATLSwitchStateImplData_internal const* const pInternalStateData = static_cast<SATLSwitchStateImplData_internal const*>(pSwitchStateData);

	//TODO: once there is more than one internal switch, a more sensible approach needs to be developed
	if (pInternalStateData->nATLInternalSwitchID == SATLInternalControlIDs::nObstructionOcclusionCalcSwitchID)
	{
		if (pAudioObject->HasPosition())
		{
			CATLAudioObject* const pPositionedAudioObject = static_cast<CATLAudioObject*>(pAudioObject);

			if (pInternalStateData->nATLInternalStateID == SATLInternalControlIDs::nOOCIgnoreStateID)
			{
				pPositionedAudioObject->SetObstructionOcclusionCalc(eAOOCT_IGNORE);
				SATLSoundPropagationData oPropagationData;
				pPositionedAudioObject->GetPropagationData(oPropagationData);
				m_pImpl->SetObstructionOcclusion(pPositionedAudioObject->GetImplDataPtr(), oPropagationData.fObstruction, oPropagationData.fOcclusion);
			}
			else if (pInternalStateData->nATLInternalStateID == SATLInternalControlIDs::nOOCSingleRayStateID)
			{
				pPositionedAudioObject->SetObstructionOcclusionCalc(eAOOCT_SINGLE_RAY);
			}		
			else if (pInternalStateData->nATLInternalStateID == SATLInternalControlIDs::nOOCMultiRayStateID)
			{
				pPositionedAudioObject->SetObstructionOcclusionCalc(eAOOCT_MULTI_RAY);
			}
			else
			{
				assert(false);// setting ignore obstruction to an unknown value
			}
		}
	}

	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioTranslationLayer::SetInternalEnvironment(
	CATLAudioObjectBase* const pAudioObject,
	IATLEnvironmentImplData const* const pEnvironmentImplData,
	float const fAmount)
{
	// TODO: implement 
	return eARS_FAILURE;
}

///////////////////////////////////////////////////////////////////////////
void CAudioTranslationLayer::UpdateSharedData()
{
	m_oAudioListenerMgr.GetDefaultListenerPosition(m_oSharedData.m_oActiveListenerPosition);
}

//////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioTranslationLayer::RefreshAudioSystem(char const* const sLevelName)
{
	g_AudioLogger.Log(eALT_WARNING, "Beginning to refresh the AudioSystem!");

	EAudioRequestStatus eResult = m_pImpl->StopAllSounds();
	assert(eResult == eARS_SUCCESS);

	eResult = m_oFileCacheMgr.UnloadDataByScope(eADS_LEVEL_SPECIFIC);
	assert(eResult == eARS_SUCCESS);

	eResult = m_oFileCacheMgr.UnloadDataByScope(eADS_GLOBAL);
	assert(eResult == eARS_SUCCESS);

	eResult = ClearControlsData(eADS_ALL);
	assert(eResult == eARS_SUCCESS);

	eResult = ClearPreloadsData(eADS_ALL);
	assert(eResult == eARS_SUCCESS);

	m_pImpl->OnAudioSystemRefresh();

	SetImplLanguage();

	CryFixedStringT<MAX_AUDIO_FILE_PATH_LENGTH> sConfigPath(gEnv->pAudioSystem->GetConfigPath());
	eResult = ParseControlsData(sConfigPath.c_str(), eADS_GLOBAL);
	assert(eResult == eARS_SUCCESS);

	eResult = ParsePreloadsData(sConfigPath.c_str(), eADS_GLOBAL);
	assert(eResult == eARS_SUCCESS);

	eResult = m_oFileCacheMgr.TryLoadRequest(SATLInternalControlIDs::nGlobalPreloadRequestID, true, true);
	assert(eResult == eARS_SUCCESS);

	if (sLevelName != NPTR && sLevelName[0] != '\0')
	{
		sConfigPath.append("levels/");
		sConfigPath.append(sLevelName);
		eResult = ParseControlsData(sConfigPath.c_str(), eADS_LEVEL_SPECIFIC);
		assert(eResult == eARS_SUCCESS);

		eResult = ParsePreloadsData(sConfigPath.c_str(), eADS_LEVEL_SPECIFIC);
		assert(eResult == eARS_SUCCESS);

		TAudioPreloadRequestID nPreloadRequestID = INVALID_AUDIO_PRELOAD_REQUEST_ID;

		if (gEnv->pAudioSystem->GetAudioPreloadRequestID(sLevelName, nPreloadRequestID))
		{
			eResult = m_oFileCacheMgr.TryLoadRequest(nPreloadRequestID, true, true);
			assert(eResult == eARS_SUCCESS);
		}
	}

	g_AudioLogger.Log(eALT_WARNING, "Done refreshing the AudioSystem!");

	return eARS_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
void CAudioTranslationLayer::SetImplLanguage()
{
	if (ICVar* pCVar = gEnv->pConsole->GetCVar("g_languageAudio"))
	{
		m_pImpl->SetLanguage(pCVar->GetString());
	}
}

//////////////////////////////////////////////////////////////////////////
void CAudioTranslationLayer::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
{
	switch (event)
	{
	case ESYSTEM_EVENT_LEVEL_UNLOAD:
		{
			CATLAudioObject::CPropagationProcessor::s_bCanIssueRWIs = false;

			break;
		}
	case ESYSTEM_EVENT_LEVEL_GAMEPLAY_START:
	case ESYSTEM_EVENT_LEVEL_PRECACHE_START:
		{
			m_oAudioObjectMgr.ReleasePendingRays();
			CATLAudioObject::CPropagationProcessor::s_bCanIssueRWIs = true;

			break;
		}
	case ESYSTEM_EVENT_LEVEL_POST_UNLOAD:
		{
			m_oAudioObjectMgr.ReleasePendingRays();

			break;
		}
	default:
		{
			break;
		}
	}
}

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
///////////////////////////////////////////////////////////////////////////
bool CAudioTranslationLayer::ReserveAudioObjectID(TAudioObjectID& rAudioObjectID, char const* const sAudioObjectName)
{
	bool const bSuccess = m_oAudioObjectMgr.ReserveID(rAudioObjectID, sAudioObjectName);

	if (bSuccess)
	{
		m_oDebugNameStore.AddAudioObject(rAudioObjectID, sAudioObjectName);
	}

	return bSuccess;
}

//////////////////////////////////////////////////////////////////////////
void CAudioTranslationLayer::DrawAudioSystemDebugInfo()
{
	FUNCTION_PROFILER_ALWAYS(gEnv->pSystem, PROFILE_AUDIO);

	IRenderer* const pRenderer = gEnv->pRenderer;

	if ((g_SoundCVars.m_nDrawAudioDebug > 0) && (pRenderer != NPTR))
	{
		DrawAudioObjectDebugInfo(pRenderer); // needs to be called first so that the rest of the labels are printed
		// on top (Draw2dLabel doesn't provide a way set which labels are printed on top)

		size_t const nBucketAllocatorPoolUsedSize		= g_MemoryPoolPrimary.GetSmallAllocsSize();
		size_t const nBucketAllocatorPoolAllocation	= g_MemoryPoolPrimary.GetSmallAllocsCount();
		size_t const nPrimaryPoolSize								= g_MemoryPoolPrimary.MemSize();
		size_t const nPrimaryPoolFreeSize						= g_MemoryPoolPrimary.MemFree();
		size_t const nPrimaryPoolUsedSize						= nPrimaryPoolSize - nPrimaryPoolFreeSize;
		size_t const nPrimaryPoolAllocation					= g_MemoryPoolPrimary.FragmentCount();

		float fPosX = 0.0f;
		float fPosY = 4.0f;

		float const fColor[4]				= {1.0f, 1.0f, 1.0f, 0.9f};
		float const fColorRed[4]		= {1.0f, 0.0f, 0.0f, 0.7f};
		float const fColorGreen[4]	= {0.0f, 1.0f, 0.0f, 0.7f};
		float const fColorBlue[4]		= {0.4f, 0.4f, 1.0f, 1.0f};

		pRenderer->Draw2dLabel(fPosX, fPosY, 1.6f, fColorBlue, false, "AudioTranslationLayer with %s", m_sImplementationNameString.c_str());

		fPosX += 20.0f;
		fPosY += 17.0f;

		pRenderer->Draw2dLabel(fPosX, fPosY, 1.35f, fColor, false, 
			"ATL Memory: Bucket: %d / ? KiB NumAllocs: %d Primary: %.2f / %.2f MiB NumAllocs: %d", 
			static_cast<int>(nBucketAllocatorPoolUsedSize / 1024), 
			static_cast<int>(nBucketAllocatorPoolAllocation),
			(nPrimaryPoolUsedSize/1024)/1024.0f,
			(nPrimaryPoolSize/1024)/1024.0f,
			static_cast<int>(nPrimaryPoolAllocation));

		float const fLineHeight = 13.0f;

		if (m_pImpl != NPTR)
		{
			SAudioImplMemoryInfo oMemoryInfo;
			m_pImpl->GetMemoryInfo(oMemoryInfo);

			fPosY += fLineHeight;
			pRenderer->Draw2dLabel(fPosX, fPosY, 1.35f, fColor, false,
				"Impl Memory: Bucket: %d / ? KiB NumAllocs: %d Primary: %.2f / %.2f MiB NumAllocs: %d Secondary: %.2f / %.2f MiB NumAllocs: %d",
				static_cast<int>(oMemoryInfo.nBucketUsedSize / 1024),
				static_cast<int>(oMemoryInfo.nBucketAllocations),
				(oMemoryInfo.nPrimaryPoolUsedSize/1024)/1024.0f,
				(oMemoryInfo.nPrimaryPoolSize/1024)/1024.0f,
				static_cast<int>(oMemoryInfo.nPrimaryPoolAllocations),
				(oMemoryInfo.nSecondaryPoolUsedSize/1024)/1024.0f,
				(oMemoryInfo.nSecondaryPoolSize/1024)/1024.0f,
				static_cast<int>(oMemoryInfo.nSecondaryPoolAllocations));
		}

		static float const SMOOTHING_ALPHA = 0.2f;
		static float fSyncRays = 0;
		static float fAsyncRays = 0;

		Vec3 const vPos											= m_oSharedData.m_oActiveListenerPosition.GetPositionVec();
		Vec3 const vFwd											= m_oSharedData.m_oActiveListenerPosition.GetForwardVec();
		size_t const nNumAudioObjects				= m_oAudioObjectMgr.GetNumAudioObjects();
		size_t const nNumActiveAudioObjects	= m_oAudioObjectMgr.GetNumActiveAudioObjects();
		size_t const nEvents								= m_oAudioEventMgr.GetNumActive();
		size_t const nListeners							= m_oAudioListenerMgr.GetNumActive();
		fSyncRays		+= (CATLAudioObject::CPropagationProcessor::s_nTotalSyncPhysRays - fSyncRays) * SMOOTHING_ALPHA;
		fAsyncRays	+= (CATLAudioObject::CPropagationProcessor::s_nTotalAsyncPhysRays - fAsyncRays) * SMOOTHING_ALPHA * 0.1f;

		bool const bActive = true;
		float const fColorListener[4] =
		{bActive?fColorGreen[0]:fColorRed[0],
		bActive?fColorGreen[1]:fColorRed[1],
		bActive?fColorGreen[2]:fColorRed[2],
		1.0f};

		float const* fColorNumbers = fColorBlue;

		fPosY += fLineHeight;
		pRenderer->Draw2dLabel(fPosX, fPosY, 1.35f, fColorListener, false, "Listener <%d> PosXYZ: %.2f %.2f %.2f FwdXYZ: %.2f %.2f %.2f", 0, vPos.x, vPos.y, vPos.z, vFwd.x, vFwd.y, vFwd.z);

		fPosY += fLineHeight;
		pRenderer->Draw2dLabel(fPosX, fPosY, 1.35f, fColorNumbers, false, 
			"Objects: %3" PRISIZE_T "/%3" PRISIZE_T " Events: %3" PRISIZE_T " Listeners: %" PRISIZE_T " | SyncRays: %3.1f AsyncRays: %3.1f",
			nNumActiveAudioObjects, nNumAudioObjects, nEvents, nListeners, fSyncRays, fAsyncRays);

		fPosY += fLineHeight;
		DrawATLComponentDebugInfo(pRenderer, fPosX, fPosY);
	}
}

///////////////////////////////////////////////////////////////////////////
void CAudioTranslationLayer::DrawATLComponentDebugInfo(IRenderer* const pRenderer, float fPosX, float const fPosY)
{
	if (m_bIsRunning)
	{
		m_oFileCacheMgr.DrawDebugInfo(pRenderer, fPosX, fPosY);

		if ((g_SoundCVars.m_nDrawAudioDebug & eADDF_SHOW_ACTIVE_OBJECTS) != 0)
		{			
			m_oAudioObjectMgr.DrawDebugInfo(pRenderer, fPosX, fPosY);
			fPosX += 300;
		}

		if ((g_SoundCVars.m_nDrawAudioDebug & eADDF_SHOW_ACTIVE_EVENTS) != 0)
		{		
			m_oAudioEventMgr.DrawDebugInfo(pRenderer, fPosX, fPosY);
		}
	}
}

///////////////////////////////////////////////////////////////////////////
void CAudioTranslationLayer::DrawAudioObjectDebugInfo(IRenderer* const pRenderer)
{
	if (m_bIsRunning)
	{
		SATLWorldPosition oListenerPosition;
		m_oAudioListenerMgr.GetDefaultListenerPosition(oListenerPosition);

		m_oAudioObjectMgr.DrawPerObjectDebugInfo(pRenderer, oListenerPosition.GetPositionVec());
	}
}
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
