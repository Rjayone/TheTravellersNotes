// CryEngine Source File.
// Copyright (C), Crytek, 1999-2014.

#include "stdafx.h"

#define REQUEST_CASE_BLOCK(CLASS, ENUM, P_SOURCE, P_RESULT)\
	case ENUM:\
	{\
	POOL_NEW(CLASS ## Internal<ENUM>, P_RESULT)(static_cast<CLASS<ENUM> const* const> (P_SOURCE));\
		\
		break;\
	}

#define AM_REQUEST_BLOCK(ENUM) REQUEST_CASE_BLOCK(SAudioManagerRequestData, ENUM, pExternalData, pResult)
#define ACM_REQUEST_BLOCK(ENUM) REQUEST_CASE_BLOCK(SAudioCallbackManagerRequestData, ENUM, pExternalData, pResult)
#define AO_REQUEST_BLOCK(ENUM) REQUEST_CASE_BLOCK(SAudioObjectRequestData, ENUM, pExternalData, pResult)
#define AL_REQUEST_BLOCK(ENUM) REQUEST_CASE_BLOCK(SAudioListenerRequestData, ENUM, pExternalData, pResult)

///////////////////////////////////////////////////////////////////////////
SAudioRequestDataInternal* ConvertToInternal(SAudioRequestDataBase const* const pExternalData)
{
	EAudioRequestType const eRequestType	= pExternalData->eRequestType;
	SAudioRequestDataInternal* pResult		= NPTR;

	switch (eRequestType)
	{
	case eART_AUDIO_MANAGER_REQUEST:
		{
			SAudioManagerRequestDataBase const* const pBase = static_cast<SAudioManagerRequestDataBase const* const>(pExternalData);

			switch (pBase->eType)
			{
			AM_REQUEST_BLOCK(eAMRT_RESERVE_AUDIO_OBJECT_ID)
			AM_REQUEST_BLOCK(eAMRT_ADD_REQUEST_LISTENER)
			AM_REQUEST_BLOCK(eAMRT_REMOVE_REQUEST_LISTENER)
			AM_REQUEST_BLOCK(eAMRT_PARSE_CONTROLS_DATA)
			AM_REQUEST_BLOCK(eAMRT_PARSE_PRELOADS_DATA)
			AM_REQUEST_BLOCK(eAMRT_CLEAR_CONTROLS_DATA)
			AM_REQUEST_BLOCK(eAMRT_CLEAR_PRELOADS_DATA)
			AM_REQUEST_BLOCK(eAMRT_PRELOAD_SINGLE_REQUEST)
			AM_REQUEST_BLOCK(eAMRT_UNLOAD_SINGLE_REQUEST)
			AM_REQUEST_BLOCK(eAMRT_UNLOAD_AFCM_DATA_BY_SCOPE)
			AM_REQUEST_BLOCK(eAMRT_INIT_AUDIO_SYSTEM)
			AM_REQUEST_BLOCK(eAMRT_SHUTDOWN_AUDIO_SYSTEM)
			AM_REQUEST_BLOCK(eAMRT_REFRESH_AUDIO_SYSTEM)
			AM_REQUEST_BLOCK(eAMRT_LOSE_FOCUS)
			AM_REQUEST_BLOCK(eAMRT_GET_FOCUS)
			AM_REQUEST_BLOCK(eAMRT_MUTE_ALL)
			AM_REQUEST_BLOCK(eAMRT_UNMUTE_ALL)
			AM_REQUEST_BLOCK(eAMRT_STOP_ALL_SOUNDS)
			AM_REQUEST_BLOCK(eAMRT_DRAW_DEBUG_INFO)
			AM_REQUEST_BLOCK(eAMRT_CHANGE_LANGUAGE)
			default:
				{
					g_AudioLogger.Log(eALT_ERROR, "Unknown audio manager request type (%d)", pBase->eType);
					assert(false);

					break;
				}
			}

			break;
		}
	case eART_AUDIO_CALLBACK_MANAGER_REQUEST:
		{
			SAudioCallbackManagerRequestDataBase const* const pBase = static_cast<SAudioCallbackManagerRequestDataBase const* const>(pExternalData);

			switch (pBase->eType)
			{
			ACM_REQUEST_BLOCK(eACMRT_REPORT_FINISHED_EVENT)
			ACM_REQUEST_BLOCK(eACMRT_REPORT_PROCESSED_OBSTRUCTION_RAY)
			default:
				{
					g_AudioLogger.Log(eALT_ERROR, "Unknown audio callback manager request type (%d)", pBase->eType);
					assert(false);

					break;
				}
			}

			break;
		}
	case eART_AUDIO_OBJECT_REQUEST:
		{
			SAudioObjectRequestDataBase const* const pBase = static_cast<SAudioObjectRequestDataBase const* const>(pExternalData);

			switch (pBase->eType)
			{
			AO_REQUEST_BLOCK(eAORT_EXECUTE_TRIGGER)
			AO_REQUEST_BLOCK(eAORT_PREPARE_TRIGGER)
			AO_REQUEST_BLOCK(eAORT_UNPREPARE_TRIGGER)
			AO_REQUEST_BLOCK(eAORT_STOP_TRIGGER)
			AO_REQUEST_BLOCK(eAORT_STOP_ALL_TRIGGERS)
			AO_REQUEST_BLOCK(eAORT_SET_POSITION)
			AO_REQUEST_BLOCK(eAORT_SET_SWITCH_STATE)
			AO_REQUEST_BLOCK(eAORT_SET_RTPC_VALUE)
			AO_REQUEST_BLOCK(eAORT_SET_VOLUME)
			AO_REQUEST_BLOCK(eAORT_SET_ENVIRONMENT_AMOUNT)
			AO_REQUEST_BLOCK(eAORT_RESET_ENVIRONMENTS)
			AO_REQUEST_BLOCK(eAORT_RELEASE_OBJECT)
			default:
				{
					g_AudioLogger.Log(eALT_ERROR, "Unknown audio object request type (%d)", pBase->eType);
					assert(false);

					break;
				}
			}

			break;
		}
	case eART_AUDIO_LISTENER_REQUEST:
		{
			SAudioListenerRequestDataBase const* const pBase = static_cast<SAudioListenerRequestDataBase const* const>(pExternalData);

			switch(pBase->eType)
			{
			AL_REQUEST_BLOCK(eALRT_SET_POSITION)
			default:
				{
					g_AudioLogger.Log(eALT_ERROR, "Unknown audio listener request type (%d)", pBase->eType);
					assert(false);

					break;
				}
			}

			break;
		}
	default:
		{
			g_AudioLogger.Log(eALT_ERROR, "Unknown audio request type (%d)", eRequestType);
			assert(false);

			break;
		}
	}

	return pResult;
}

//////////////////////////////////////////////////////////////////////////
void SAudioRequestDataInternal::Release()
{
	int const nCount = CryInterlockedDecrement(&m_nRefCounter);
	assert(nCount >= 0);

	if (nCount == 0)
	{
		POOL_FREE(this);
	}
	else if (nCount < 0)
	{
		CryFatalError("Deleting Reference Counted Object Twice");
	}
}
