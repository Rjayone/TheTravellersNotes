// CryEngine Source File.
// Copyright (C), Crytek, 1999-2014.

#include "stdafx.h"
#include "AudioProxy.h"
#include "SoundCVars.h"
#include <AudioLogger.h>
#include <ISystem.h>
#include <IEntitySystem.h>

TAudioControlID CAudioProxy::s_nObstructionCalcSwitchID = INVALID_AUDIO_CONTROL_ID;
TAudioSwitchStateID CAudioProxy::s_aObstructionCalcStateIDs[eAOOCT_COUNT] = {
	INVALID_AUDIO_SWITCH_STATE_ID,
	INVALID_AUDIO_SWITCH_STATE_ID,
	INVALID_AUDIO_SWITCH_STATE_ID,
	INVALID_AUDIO_SWITCH_STATE_ID
};

//////////////////////////////////////////////////////////////////////////
struct AreaInfoCompare
{
	bool operator() (SAudioAreaInfo const& oAreaInfo1, SAudioAreaInfo const& oAreaInfo2)
	{
		bool bResult = false;
		int const nGroup1 = oAreaInfo1.pArea->GetGroup();
		int const nGroup2 = oAreaInfo2.pArea->GetGroup();

		if (nGroup1 == nGroup2)
		{
			bResult = (oAreaInfo1.pArea->GetPriority() > oAreaInfo2.pArea->GetPriority());
		}
		else
		{
			bResult = (nGroup1 > nGroup2);
		}

		return bResult;
	}
};

///////////////////////////////////////////////////////////////////////////
CAudioProxy::CAudioProxy()
	:	m_nAudioObjectID(INVALID_AUDIO_OBJECT_ID)
	,	m_nFlags(eAPF_NONE)
	, m_eCurrentLipSyncMethod(eLSM_None)
	, m_nCurrentLipSyncID(INVALID_AUDIO_CONTROL_ID)
{
	gEnv->pAudioSystem->AddRequestListener(&CAudioProxy::OnAudioEvent, this);
}

///////////////////////////////////////////////////////////////////////////
CAudioProxy::~CAudioProxy()
{
	assert(m_nAudioObjectID == INVALID_AUDIO_OBJECT_ID);
	stl::free_container(m_aQueuedAudioCommands);
	gEnv->pAudioSystem->RemoveRequestListener(&CAudioProxy::OnAudioEvent, this);
}

///////////////////////////////////////////////////////////////////////////
void CAudioProxy::Initialize(char const* const sObjectName, bool const bInitAsync /* = true */)
{
	if ((bInitAsync && g_SoundCVars.m_nAudioProxiesInitType == 0) || g_SoundCVars.m_nAudioProxiesInitType == 2)
	{
		if ((m_nFlags & eAPF_WAITING_FOR_ID) == 0)
		{
			m_nFlags |= eAPF_WAITING_FOR_ID;

			SAudioRequest oRequest;
			SAudioManagerRequestData<eAMRT_RESERVE_AUDIO_OBJECT_ID> oRequestData(&m_nAudioObjectID, sObjectName);
			oRequest.nFlags = eARF_PRIORITY_HIGH | eARF_SYNC_CALLBACK;
			oRequest.pOwner	= this;
			oRequest.pData = &oRequestData;

			gEnv->pAudioSystem->PushRequest(oRequest);
		}
		else
		{
			SQueuedAudioCommand oQueuedCommand = SQueuedAudioCommand(eQACT_INITIALIZE);
			oQueuedCommand.sValue = sObjectName;
			TryAddQueuedCommand(oQueuedCommand);
		}
	}
	else
	{
		SAudioRequest oRequest;
		SAudioManagerRequestData<eAMRT_RESERVE_AUDIO_OBJECT_ID> oRequestData(&m_nAudioObjectID, sObjectName);
		oRequest.nFlags = eARF_PRIORITY_HIGH | eARF_EXECUTE_BLOCKING;
		oRequest.pData = &oRequestData;

		gEnv->pAudioSystem->PushRequest(oRequest);

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
		if (m_nAudioObjectID == INVALID_AUDIO_OBJECT_ID)
		{
			CryFatalError("<Audio> Failed to reserve audio object ID on AudioProxy (%s)!", sObjectName);
		}
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
	}
}

///////////////////////////////////////////////////////////////////////////
void CAudioProxy::ExecuteTrigger(TAudioControlID const nTriggerID, ELipSyncMethod const eLipSyncMethod)
{
	ExecuteTriggerInternal(nTriggerID, eLipSyncMethod);
}

///////////////////////////////////////////////////////////////////////////
void CAudioProxy::ExecuteTrigger(
	TAudioControlID const nTriggerID,
	ELipSyncMethod const eLipSyncMethod,
	TTriggerFinishedCallback const pCallback,
	void* const pCallbackCookie)
{
	ExecuteTriggerInternal(nTriggerID, eLipSyncMethod, pCallback, pCallbackCookie);
}

///////////////////////////////////////////////////////////////////////////
void CAudioProxy::StopTrigger(TAudioControlID const nTriggerID)
{
	if ((m_nFlags & eAPF_WAITING_FOR_ID) == 0)
	{
		SAudioRequest oRequest;
		SAudioObjectRequestData<eAORT_STOP_TRIGGER> oRequestData(nTriggerID);
		oRequest.nAudioObjectID = m_nAudioObjectID;
		oRequest.nFlags = eARF_PRIORITY_NORMAL;
		oRequest.pData = &oRequestData;

		gEnv->pAudioSystem->PushRequest(oRequest);
	}
	else
	{
		SQueuedAudioCommand oQueuedCommand = SQueuedAudioCommand(eQACT_STOP_TRIGGER);
		oQueuedCommand.nTriggerID = nTriggerID;
		TryAddQueuedCommand(oQueuedCommand);
	}
}

///////////////////////////////////////////////////////////////////////////
void CAudioProxy::SetSwitchState(TAudioControlID const nSwitchID, TAudioSwitchStateID const nStateID)
{
	if ((m_nFlags & eAPF_WAITING_FOR_ID) == 0)
	{
		SAudioRequest oRequest;
		SAudioObjectRequestData<eAORT_SET_SWITCH_STATE> oRequestData(nSwitchID, nStateID);
		oRequest.nAudioObjectID = m_nAudioObjectID;
		oRequest.nFlags = eARF_PRIORITY_NORMAL;
		oRequest.pData = &oRequestData;

		gEnv->pAudioSystem->PushRequest(oRequest);
	}
	else
	{
		SQueuedAudioCommand oQueuedCommand = SQueuedAudioCommand(eQACT_SET_SWITCH_STATE);
		oQueuedCommand.nSwitchID	= nSwitchID;
		oQueuedCommand.nStateID		= nStateID;
		TryAddQueuedCommand(oQueuedCommand);
	}
}

///////////////////////////////////////////////////////////////////////////
void CAudioProxy::SetRtpcValue(TAudioControlID const nRtpcID, float const fValue)
{
	if ((m_nFlags & eAPF_WAITING_FOR_ID) == 0)
	{
		SAudioRequest oRequest;
		SAudioObjectRequestData<eAORT_SET_RTPC_VALUE> oRequestData(nRtpcID, fValue);
		oRequest.nAudioObjectID = m_nAudioObjectID;
		oRequest.nFlags = eARF_PRIORITY_NORMAL;
		oRequest.pData = &oRequestData;

		gEnv->pAudioSystem->PushRequest(oRequest);
	}
	else
	{
		SQueuedAudioCommand oQueuedCommand = SQueuedAudioCommand(eQACT_SET_RTPC_VALUE);
		oQueuedCommand.nRtpcID	= nRtpcID;
		oQueuedCommand.fValue		= fValue;
		TryAddQueuedCommand(oQueuedCommand);
	}
}

///////////////////////////////////////////////////////////////////////////
void CAudioProxy::SetObstructionCalcType(EAudioObjectObstructionCalcType const eObstructionType)
{
	TATLEnumFlagsType const nObstractionCalcIndex = static_cast<TATLEnumFlagsType>(eObstructionType);

	if (nObstractionCalcIndex < eAOOCT_COUNT)
	{
		SetSwitchState(s_nObstructionCalcSwitchID, s_aObstructionCalcStateIDs[eObstructionType]);
	}
}

///////////////////////////////////////////////////////////////////////////
void CAudioProxy::SetPosition(SATLWorldPosition const& rPosition)
{
	if ((m_nFlags & eAPF_WAITING_FOR_ID) == 0)
	{
		m_oPosition = rPosition;

		SAudioRequest oRequest;
		SAudioObjectRequestData<eAORT_SET_POSITION> oRequestData(m_oPosition);
		oRequest.nAudioObjectID = m_nAudioObjectID;
		oRequest.nFlags = eARF_PRIORITY_NORMAL;
		oRequest.pData = &oRequestData;

		gEnv->pAudioSystem->PushRequest(oRequest);
	}
	else
	{
		SQueuedAudioCommand oQueuedCommand = SQueuedAudioCommand(eQACT_SET_POSITION);
		oQueuedCommand.oPosition = rPosition;
		TryAddQueuedCommand(oQueuedCommand);
	}
}

///////////////////////////////////////////////////////////////////////////
void CAudioProxy::SetPosition(Vec3 const& rPosition)
{
	// Update position only if the delta exceeds a given value.
	// Ideally this should be done on the caller's side!
	if (g_SoundCVars.m_fPositionUpdateThreshold == 0.0f || !rPosition.IsEquivalent(m_oPosition.mPosition.GetTranslation(), g_SoundCVars.m_fPositionUpdateThreshold))
	{
		Matrix34 oPositionMatrix(ZERO);

		oPositionMatrix.SetTranslationMat(rPosition);
		SATLWorldPosition oWorldPosition(oPositionMatrix, Vec3(ZERO));

		SetPosition(oWorldPosition);
	}
}

///////////////////////////////////////////////////////////////////////////
void CAudioProxy::SetEnvironmentAmount(TAudioEnvironmentID const nEnvironmentID, float const fValue)
{
	if ((m_nFlags & eAPF_WAITING_FOR_ID) == 0)
	{
		SAudioRequest oRequest;
		SAudioObjectRequestData<eAORT_SET_ENVIRONMENT_AMOUNT> oRequestData(nEnvironmentID, fValue);
		oRequest.nAudioObjectID = m_nAudioObjectID;
		oRequest.nFlags = eARF_PRIORITY_NORMAL;
		oRequest.pData = &oRequestData;

		gEnv->pAudioSystem->PushRequest(oRequest);
	}
	else
	{
		SQueuedAudioCommand oQueuedCommand = SQueuedAudioCommand(eQACT_SET_ENVIRONMENT_AMOUNT);
		oQueuedCommand.nEnvironmentID	= nEnvironmentID;
		oQueuedCommand.fValue					= fValue;
		TryAddQueuedCommand(oQueuedCommand);
	}
}

///////////////////////////////////////////////////////////////////////////
void CAudioProxy::SetCurrentEnvironments(EntityId const nEntityToIgnore)
{
	if ((m_nFlags & eAPF_WAITING_FOR_ID) == 0)
	{
		IAreaManager* const pAreaManager = gEnv->pEntitySystem->GetAreaManager();

		SAudioAreaInfo m_aAreaQueries[sMaxAreas];
		size_t nAreaCount = 0;

		ClearEnvironments();

		if (!pAreaManager->QueryAudioAreas(m_oPosition.GetPositionVec(), m_aAreaQueries, sMaxAreas, nAreaCount))
		{
			gEnv->pSystem->Warning(VALIDATOR_MODULE_AUDIO, VALIDATOR_WARNING, VALIDATOR_FLAG_AUDIO, NULL, "<Audio>: QueryAreas failed for AudioObjectID %u", m_nAudioObjectID);
		}

		if (nAreaCount > 0)
		{
			std::sort(m_aAreaQueries, m_aAreaQueries+nAreaCount, AreaInfoCompare());

			int nLastGroupID = -1;// default value for the CArea::m_AreaGroupID
			int nLastPriority = 0;// default value for the CArea::m_nPriority
			bool bIgnoreThisGroup = false;
			float fCurrentAmount = 0.0f;

			for (size_t nArea = 0; nArea < nAreaCount; ++nArea)
			{
				SAudioAreaInfo const& rAreaInfo = m_aAreaQueries[nArea];
				int const nCurrentGroupID = rAreaInfo.pArea->GetGroup();
				int const nCurrentPriority = rAreaInfo.pArea->GetPriority();

				if (nEntityToIgnore == INVALID_ENTITYID || nEntityToIgnore != rAreaInfo.nEnvProvidingEntityID)
				{
					if (nCurrentGroupID != nLastGroupID)
					{
						// new group, highest priority
						fCurrentAmount = rAreaInfo.fEnvironmentAmount;
						nLastGroupID = nCurrentGroupID;
						nLastPriority = nCurrentPriority;

						SetEnvironmentAmount(rAreaInfo.nEnvironmentID, rAreaInfo.fEnvironmentAmount);
					}
					else if (nCurrentPriority != nLastPriority)
					{
						// same group, lower priority
						if (fCurrentAmount < 1.0f)
						{
							fCurrentAmount = rAreaInfo.fEnvironmentAmount;
							nLastPriority = nCurrentPriority;

							SetEnvironmentAmount(rAreaInfo.nEnvironmentID, fCurrentAmount);
						}
					}
					else
					{
						// same group, same priority
						// this assumes that all areas of the same priority within a group use the same Environment
						float const fAmount = rAreaInfo.fEnvironmentAmount;

						if (fAmount > fCurrentAmount)
						{
							SetEnvironmentAmount(rAreaInfo.nEnvironmentID, fAmount);
							fCurrentAmount = fAmount;
						}
					}
				}
			}
		}
	}
	else
	{
		SQueuedAudioCommand oQueuedCommand = SQueuedAudioCommand(eQACT_SET_CURRENT_ENVIRONMENTS);
		oQueuedCommand.nEntityID = nEntityToIgnore;
		TryAddQueuedCommand(oQueuedCommand);
	}
}

///////////////////////////////////////////////////////////////////////////
void CAudioProxy::SetLipSyncProvider(ILipSyncProvider* const pILipSyncProvider)
{
	if (pILipSyncProvider == m_oLipSyncProvider.get())
		return;

	m_oLipSyncProvider.reset(pILipSyncProvider);
	m_nCurrentLipSyncID = INVALID_AUDIO_CONTROL_ID;
	m_eCurrentLipSyncMethod = eLSM_None;
}

///////////////////////////////////////////////////////////////////////////
void CAudioProxy::ClearEnvironments()
{
	if ((m_nFlags & eAPF_WAITING_FOR_ID) == 0)
	{
		SAudioRequest oRequest;
		SAudioObjectRequestData<eAORT_RESET_ENVIRONMENTS> oRequestData;
		oRequest.nAudioObjectID = m_nAudioObjectID;
		oRequest.nFlags = eARF_PRIORITY_NORMAL;
		oRequest.pData = &oRequestData;

		gEnv->pAudioSystem->PushRequest(oRequest);
	}
	else
	{
		TryAddQueuedCommand(SQueuedAudioCommand(eQACT_CLEAR_ENVIRONMENTS));
	}
}

///////////////////////////////////////////////////////////////////////////
void CAudioProxy::Release()
{
	if ((m_nFlags & eAPF_WAITING_FOR_ID) == 0)
	{
		Reset();
		gEnv->pAudioSystem->FreeAudioProxy(this);
	}
	else
	{
		TryAddQueuedCommand(SQueuedAudioCommand(eQACT_RELEASE));
	}
}

//////////////////////////////////////////////////////////////////////////
void CAudioProxy::Reset()
{
	if ((m_nFlags & eAPF_WAITING_FOR_ID) == 0)
	{
		if (m_nAudioObjectID != INVALID_AUDIO_OBJECT_ID)
		{
			// Request must be asynchronous and lowest priority!
			SAudioRequest oRequest;
			SAudioObjectRequestData<eAORT_RELEASE_OBJECT> oRequestData;
			oRequest.nAudioObjectID = m_nAudioObjectID;
			oRequest.pData = &oRequestData;

			gEnv->pAudioSystem->PushRequest(oRequest);

			m_nAudioObjectID = INVALID_AUDIO_OBJECT_ID;
		}

		m_oPosition = SATLWorldPosition();
		m_eCurrentLipSyncMethod = eLSM_None;
		m_nCurrentLipSyncID = INVALID_AUDIO_CONTROL_ID;
	}
	else
	{
		TryAddQueuedCommand(SQueuedAudioCommand(eQACT_RESET));
	}
}

REINST(need a way to periodically update the LipSyncProvider)
//////////////////////////////////////////////////////////////////////////
//void CAudioProxy::Update(SEntityUpdateContext &ctx)
//{
//	if (m_currentLipSyncId != INVALID_AUDIO_CONTROL_ID)
//	{
//		if (m_pLipSyncProvider)
//		{
//			m_pLipSyncProvider->UpdateLipSync(this, m_currentLipSyncId, m_currentLipSyncMethod);
//		}
//	}
//}

//////////////////////////////////////////////////////////////////////////
void CAudioProxy::ExecuteQueuedCommands()
{
	m_nFlags &= ~eAPF_WAITING_FOR_ID;

	if (!m_aQueuedAudioCommands.empty())
	{
		TQueuedAudioCommands::iterator Iter(m_aQueuedAudioCommands.begin());
		TQueuedAudioCommands::const_iterator const IterEnd(m_aQueuedAudioCommands.end());

		for (; Iter != IterEnd; ++Iter)
		{
			SQueuedAudioCommand const& rCommand = (*Iter);

			switch (rCommand.eType)
			{
			case eQACT_EXECUTE_TRIGGER:
				{
					ExecuteTriggerInternal(rCommand.nTriggerID, rCommand.eLipSyncMethod, rCommand.pCallback, rCommand.pCallbackCookie);

					break;
				}
			case eQACT_STOP_TRIGGER:
				{
					StopTrigger(rCommand.nTriggerID);

					break;
				}
			case eQACT_SET_SWITCH_STATE:
				{
					SetSwitchState(rCommand.nSwitchID, rCommand.nStateID);

					break;
				}
			case eQACT_SET_RTPC_VALUE:
				{
					SetRtpcValue(rCommand.nRtpcID, rCommand.fValue);

					break;
				}
			case eQACT_SET_ENVIRONMENT_AMOUNT:
				{
					SetEnvironmentAmount(rCommand.nEnvironmentID, rCommand.fValue);

					break;
				}
			case eQACT_SET_CURRENT_ENVIRONMENTS:
				{
					SetCurrentEnvironments(rCommand.nEntityID);

					break;
				}
			case eQACT_CLEAR_ENVIRONMENTS:
				{
					ClearEnvironments();

					break;
				}
			case eQACT_SET_POSITION:
				{
					SetPosition(rCommand.oPosition);

					break;
				}
			case eQACT_RESET:
				{
					Reset();

					break;
				}
			case eQACT_RELEASE:
				{
					Release();

					break;
				}
			case eQACT_INITIALIZE:
				{
					Initialize(rCommand.sValue.c_str(), true);

					break;
				}
			default:
				{
					CryFatalError("Unknown command type in CAudioProxy::ExecuteQueuedCommands!");

					break;
				}
			}

			if ((m_nFlags & eAPF_WAITING_FOR_ID) != 0)
			{
				// An Initialize command was queued up.
				// Here we need to keep all commands after the Initialize.
				break;
			}
		}

		if ((m_nFlags & eAPF_WAITING_FOR_ID) == 0)
		{
			m_aQueuedAudioCommands.clear();
		}
		else
		{
			// An Initialize command was queued up.
			// Here we need to keep queued commands except for Reset and Initialize.
			Iter = m_aQueuedAudioCommands.begin();

			while (Iter != IterEnd)
			{
				SQueuedAudioCommand const& rCommand = (*Iter);

				if (rCommand.eType == eQACT_RESET || rCommand.eType == eQACT_INITIALIZE)
				{
					m_aQueuedAudioCommands.erase(Iter++);
					continue;
				}

				++Iter;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CAudioProxy::TryAddQueuedCommand(SQueuedAudioCommand const& rCommand)
{
	// For now we compare only against the last pushed back command!
	bool bAdd = true;

	switch (rCommand.eType)
	{
	case eQACT_EXECUTE_TRIGGER:
	case eQACT_STOP_TRIGGER:
		{
			// These type of commands get always pushed back!
			break;
		}
	case eQACT_SET_SWITCH_STATE:
		{
			if (!m_aQueuedAudioCommands.empty())
			{
				SQueuedAudioCommand& rLastCommand = m_aQueuedAudioCommands.back();

				if (rLastCommand.eType == rCommand.eType && rLastCommand.nSwitchID == rCommand.nSwitchID)
				{
					// Command for this switch exists, just update the state.
					rLastCommand.nStateID = rCommand.nStateID;
					bAdd = false;
				}
			}

			break;
		}
	case eQACT_SET_RTPC_VALUE:
		{
			if (!m_aQueuedAudioCommands.empty())
			{
				SQueuedAudioCommand& rLastCommand = m_aQueuedAudioCommands.back();

				if (rLastCommand.eType == rCommand.eType && rLastCommand.nRtpcID == rCommand.nRtpcID)
				{
					// Command for this RTPC exists, just update the value.
					rLastCommand.fValue = rCommand.fValue;
					bAdd = false;
				}
			}

			break;
		}
	case eQACT_SET_POSITION:
		{
			if (!m_aQueuedAudioCommands.empty())
			{
				SQueuedAudioCommand& rLastCommand = m_aQueuedAudioCommands.back();

				if (rLastCommand.eType == rCommand.eType)
				{
					// Command exists already, just update the position.
					rLastCommand.oPosition = rCommand.oPosition;
					bAdd = false;
				}
			}

			break;
		}
	case eQACT_SET_ENVIRONMENT_AMOUNT:
		{
			if (!m_aQueuedAudioCommands.empty())
			{
				SQueuedAudioCommand& rLastCommand = m_aQueuedAudioCommands.back();

				if (rLastCommand.eType == rCommand.eType && rLastCommand.nEnvironmentID == rCommand.nEnvironmentID)
				{
					// Command for this Environment exists, just update the value.
					rLastCommand.fValue = rCommand.fValue;
					bAdd = false;
				}
			}

			break;
		}
	case eQACT_SET_CURRENT_ENVIRONMENTS:
	case eQACT_CLEAR_ENVIRONMENTS:
	case eQACT_RELEASE:
		{
			TQueuedAudioCommands::const_iterator Iter(m_aQueuedAudioCommands.begin());
			TQueuedAudioCommands::const_iterator const IterEnd(m_aQueuedAudioCommands.end());

			for (; Iter != IterEnd; ++Iter)
			{
				SQueuedAudioCommand const& rLocalCommand = (*Iter);

				if (rLocalCommand.eType == rCommand.eType)
				{
					// These type of commands don't need another instance!
					bAdd = false;

					break;
				}
			}

			break;
		}
	case eQACT_RESET:
		{
			TQueuedAudioCommands::const_iterator Iter(m_aQueuedAudioCommands.begin());
			TQueuedAudioCommands::const_iterator const IterEnd(m_aQueuedAudioCommands.end());

			for (; Iter != IterEnd; ++Iter)
			{
				SQueuedAudioCommand const& rLocalCommand = (*Iter);

				if (rLocalCommand.eType == eQACT_RELEASE)
				{
					// If eQACT_RELEASE is already queued up then there is no need for adding a eQACT_RESET command.
					bAdd = false;

					break;
				}
			}

			if (!bAdd)
			{
				// If this proxy is resetting then there is no need for any pending commands.
				m_aQueuedAudioCommands.clear();
			}

			break;
		}
	case eQACT_INITIALIZE:
		{
			// There must be only 1 Initialize command be queued up.
			m_aQueuedAudioCommands.clear();

			// Precede the Initialization with a Reset command to release the pending audio object.
			m_aQueuedAudioCommands.push_back(SQueuedAudioCommand(eQACT_RESET));

			break;
		}
	default:
		{
			assert(false);
			g_AudioLogger.Log(eALT_ERROR, "Unknown queued command type in CAudioProxy::TryAddQueuedCommand!");
			bAdd = false;

			break;
		}
	}

	if (bAdd)
	{
		m_aQueuedAudioCommands.push_back(rCommand);
	}
}

///////////////////////////////////////////////////////////////////////////
void CAudioProxy::ExecuteTriggerInternal(
	TAudioControlID const nTriggerID,
	ELipSyncMethod const eLipSyncMethod,
	TTriggerFinishedCallback const pCallback /* = NPTR */, 
	void* const pCallbackCookie /* = NPTR */)
{
	if ((m_nFlags & eAPF_WAITING_FOR_ID) == 0)
	{
		assert(m_nAudioObjectID != INVALID_AUDIO_OBJECT_ID);

		SAudioRequest oRequest;
		oRequest.nAudioObjectID = m_nAudioObjectID;
		oRequest.nFlags = eARF_PRIORITY_NORMAL;

		SAudioTriggerFinishedCallbackData oCallbackData(pCallback, pCallbackCookie);
		SAudioObjectRequestData<eAORT_EXECUTE_TRIGGER> oRequestData(nTriggerID, 0.0f, oCallbackData);
		oRequestData.eLipSyncMethod = eLipSyncMethod;

		oRequest.pData = &oRequestData;

		gEnv->pAudioSystem->PushRequest(oRequest);
	}
	else
	{
		SQueuedAudioCommand oQueuedCommand = SQueuedAudioCommand(eQACT_EXECUTE_TRIGGER);
		oQueuedCommand.nTriggerID = nTriggerID;
		oQueuedCommand.pCallback = pCallback;
		oQueuedCommand.pCallbackCookie = pCallbackCookie;
		oQueuedCommand.eLipSyncMethod = eLipSyncMethod;
		TryAddQueuedCommand(oQueuedCommand);
	}
}
//////////////////////////////////////////////////////////////////////////
void CAudioProxy::OnAudioEvent(SAudioRequestInfo const* const pAudioRequestInfo)
{
	switch (pAudioRequestInfo->eResult)
	{
	case eARR_SUCCESS:
		{
			switch (pAudioRequestInfo->eAudioRequestType)
			{
			case eART_AUDIO_MANAGER_REQUEST:
				{
					EAudioManagerRequestType const eAudioManagerRequestType = static_cast<EAudioManagerRequestType const>(pAudioRequestInfo->nSpecificAudioRequest);

					switch (eAudioManagerRequestType)
					{
					case eAMRT_RESERVE_AUDIO_OBJECT_ID:
						{
							CAudioProxy* const pAudioProxy = static_cast<CAudioProxy* const>(pAudioRequestInfo->pOwner);

							if (pAudioProxy != NPTR)
							{
								pAudioProxy->ExecuteQueuedCommands();
							}

							break;
						}
					default:
						{
							break;
						}
					}

					break;
				}
			default:
				{
					break;
				}
			}

			break;
		}
	default:
		{
			g_AudioLogger.Log(eALT_ERROR, "Unknown result type for AudioProxy request!");

			break;
		}
	}
}
