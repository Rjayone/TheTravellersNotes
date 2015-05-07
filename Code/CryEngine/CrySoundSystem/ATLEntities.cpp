// CryEngine Source File.
// Copyright (C), Crytek, 1999-2014.

#include "stdafx.h"
#include "ATLEntities.h"

#if defined(WIN32) || defined(WIN64)
char const* const SATLXMLTags::sPlatform									= "pc";
#elif defined(DURANGO)
char const* const SATLXMLTags::sPlatform									= "xboxone";
#elif defined(ORBIS)
char const* const SATLXMLTags::sPlatform									= "ps4";
#elif defined(MAC)
char const* const SATLXMLTags::sPlatform									= "mac";
#elif defined(LINUX)
char const* const SATLXMLTags::sPlatform									= "linux";
#elif defined(CAFE)
char const* const SATLXMLTags::sPlatform									= "wiiu";
#elif defined(IOS)
char const* const SATLXMLTags::sPlatform									= "ios";
#elif defined(ANDROID)
char const* const SATLXMLTags::sPlatform									= "android";
#else
#error "Undefined platform."
#endif

char const* const SATLXMLTags::sRootNodeTag								= "ATLConfig";
char const* const SATLXMLTags::sTriggersNodeTag						= "AudioTriggers";
char const* const SATLXMLTags::sRtpcsNodeTag							= "AudioRtpcs";
char const* const SATLXMLTags::sSwitchesNodeTag						= "AudioSwitches";
char const* const SATLXMLTags::sPreloadsNodeTag						= "AudioPreloads";
char const* const SATLXMLTags::sEnvironmentsNodeTag				= "AudioEnvironments";

char const* const SATLXMLTags::sATLTriggerTag							= "ATLTrigger";
char const* const SATLXMLTags::sATLSwitchTag							= "ATLSwitch";
char const* const SATLXMLTags::sATLRtpcTag								= "ATLRtpc";
char const* const SATLXMLTags::sATLSwitchStateTag					= "ATLSwitchState";
char const* const SATLXMLTags::sATLEnvironmentTag					= "ATLEnvironment";
char const* const SATLXMLTags::sATLPlatformsTag						= "ATLPlatforms";
char const* const SATLXMLTags::sATLConfigGroupTag					= "ATLConfigGroup";

char const* const SATLXMLTags::sATLTriggerRequestTag			= "ATLTriggerRequest";
char const* const SATLXMLTags::sATLSwitchRequestTag				= "ATLSwitchRequest";
char const* const SATLXMLTags::sATLValueTag								= "ATLValue";
char const* const SATLXMLTags::sATLRtpcRequestTag					= "ATLRtpcRequest";
char const* const SATLXMLTags::sATLPreloadRequestTag			= "ATLPreloadRequest";
char const* const SATLXMLTags::sATLEnvironmentRequestTag	= "ATLEnvironmentRequest";

char const* const SATLXMLTags::sATLNameAttribute					= "atl_name";
char const* const SATLXMLTags::sATLInternalNameAttribute	= "atl_internal_name";
char const* const SATLXMLTags::sATLTypeAttribute					=	"atl_type";
char const* const SATLXMLTags::sATLConfigGroupAttribute		=	"atl_config_group_name";

char const* const SATLXMLTags::sATLDataLoadType						= "AutoLoad";

TAudioControlID SATLInternalControlIDs::nObstructionOcclusionCalcSwitchID	= INVALID_AUDIO_CONTROL_ID;
TAudioControlID SATLInternalControlIDs::nLoseFocusTriggerID								= INVALID_AUDIO_CONTROL_ID;
TAudioControlID SATLInternalControlIDs::nGetFocusTriggerID								= INVALID_AUDIO_CONTROL_ID;
TAudioControlID SATLInternalControlIDs::nMuteAllTriggerID									= INVALID_AUDIO_CONTROL_ID;
TAudioControlID SATLInternalControlIDs::nUnmuteAllTriggerID								= INVALID_AUDIO_CONTROL_ID;
TAudioSwitchStateID SATLInternalControlIDs::nOOCIgnoreStateID							= INVALID_AUDIO_SWITCH_STATE_ID;
TAudioSwitchStateID SATLInternalControlIDs::nOOCSingleRayStateID					= INVALID_AUDIO_SWITCH_STATE_ID;
TAudioSwitchStateID SATLInternalControlIDs::nOOCMultiRayStateID						= INVALID_AUDIO_SWITCH_STATE_ID;
TAudioPreloadRequestID SATLInternalControlIDs::nGlobalPreloadRequestID		= INVALID_AUDIO_PRELOAD_REQUEST_ID;


///////////////////////////////////////////////////////////////////////////
void InitATLControlIDs()
{
	SATLInternalControlIDs::nObstructionOcclusionCalcSwitchID	=	AudioStringToID("ObstructionOcclusionCalculationType");
	SATLInternalControlIDs::nLoseFocusTriggerID								= AudioStringToID("lose_focus");
	SATLInternalControlIDs::nGetFocusTriggerID								= AudioStringToID("get_focus");
	SATLInternalControlIDs::nMuteAllTriggerID									= AudioStringToID("mute_all");
	SATLInternalControlIDs::nUnmuteAllTriggerID								= AudioStringToID("unmute_all");
	SATLInternalControlIDs::nOOCIgnoreStateID									=	AudioStringToID("ignore");
	SATLInternalControlIDs::nOOCSingleRayStateID							=	AudioStringToID("single_ray");
	SATLInternalControlIDs::nOOCMultiRayStateID								=	AudioStringToID("multi_ray");
	SATLInternalControlIDs::nGlobalPreloadRequestID						= AudioStringToID("global_atl_preloads");
}

///////////////////////////////////////////////////////////////////////////
void CAudioTriggerCallbackQueue::Push(SAudioTriggerCallbackData const& rCallbackData)
{
	CryAutoLock<CryCriticalSection> autoLock(m_oCriticalSection);

	m_aTriggerCallbacks.push_back(rCallbackData);
}

///////////////////////////////////////////////////////////////////////////
void CAudioTriggerCallbackQueue::ExecuteAll()
{
	CryAutoLock<CryCriticalSection> autoLock(m_oCriticalSection);

	if (!m_aTriggerCallbacks.empty())
	{
		TTriggerCallbacks::const_iterator iCallbackData = m_aTriggerCallbacks.begin();
		TTriggerCallbacks::const_iterator iCallbackDataEnd = m_aTriggerCallbacks.end();

		for (;iCallbackData != iCallbackDataEnd; ++iCallbackData)
		{
			(*iCallbackData->pCallback)(iCallbackData->nObjectID, iCallbackData->nTriggerID, iCallbackData->pCallbackCookie);
		}

		m_aTriggerCallbacks.clear();
	}
}

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
///////////////////////////////////////////////////////////////////////////
CATLDebugNameStore::CATLDebugNameStore()
	: m_bATLObjectsChanged(false)
	,	m_bATLTriggersChanged(false)
	,	m_bATLRtpcsChanged(false)
	,	m_bATLSwitchesChanged(false)
	,	m_bATLPreloadsChanged(false)
	,	m_bATLEnvironmentsChanged(false)
{
}

///////////////////////////////////////////////////////////////////////////
CATLDebugNameStore::~CATLDebugNameStore()
{
	// the containers only hold numbers and strings, no ATL specific objects, 
	// so there is no need to call the implementation to do the cleanup
	stl::free_container(m_cATLObjectNames);
	stl::free_container(m_cATLTriggerNames);
	stl::free_container(m_cATLSwitchNames);
	stl::free_container(m_cATLRtpcNames);
	stl::free_container(m_cATLPreloadRequestNames);
	stl::free_container(m_cATLEnvironmentNames);
}

///////////////////////////////////////////////////////////////////////////
void CATLDebugNameStore::SyncChanges(CATLDebugNameStore const& rOtherNameStore)
{
	if (rOtherNameStore.m_bATLObjectsChanged)
	{
		m_cATLObjectNames.clear();
		m_cATLObjectNames.insert(rOtherNameStore.m_cATLObjectNames.begin(), rOtherNameStore.m_cATLObjectNames.end());
		rOtherNameStore.m_bATLObjectsChanged = false;
	}

	if (rOtherNameStore.m_bATLTriggersChanged)
	{
		m_cATLTriggerNames.clear();
		m_cATLTriggerNames.insert(rOtherNameStore.m_cATLTriggerNames.begin(), rOtherNameStore.m_cATLTriggerNames.end());
		rOtherNameStore.m_bATLTriggersChanged = false;
	}

	if (rOtherNameStore.m_bATLRtpcsChanged)
	{
		m_cATLRtpcNames.clear();
		m_cATLRtpcNames.insert(rOtherNameStore.m_cATLRtpcNames.begin(), rOtherNameStore.m_cATLRtpcNames.end());
		rOtherNameStore.m_bATLRtpcsChanged = false;
	}

	if (rOtherNameStore.m_bATLSwitchesChanged)
	{
		m_cATLSwitchNames.clear();
		m_cATLSwitchNames.insert(rOtherNameStore.m_cATLSwitchNames.begin(), rOtherNameStore.m_cATLSwitchNames.end());
		rOtherNameStore.m_bATLSwitchesChanged = false;
	}

	if (rOtherNameStore.m_bATLPreloadsChanged)
	{
		m_cATLPreloadRequestNames.clear();
		m_cATLPreloadRequestNames.insert(rOtherNameStore.m_cATLPreloadRequestNames.begin(), rOtherNameStore.m_cATLPreloadRequestNames.end());
		rOtherNameStore.m_bATLPreloadsChanged = false;
	}

	if (rOtherNameStore.m_bATLEnvironmentsChanged)
	{
		m_cATLEnvironmentNames.clear();
		m_cATLEnvironmentNames.insert(rOtherNameStore.m_cATLEnvironmentNames.begin(), rOtherNameStore.m_cATLEnvironmentNames.end());
		rOtherNameStore.m_bATLEnvironmentsChanged = false;
	}
}

///////////////////////////////////////////////////////////////////////////
void CATLDebugNameStore::AddAudioObject(TAudioObjectID const nObjectID, char const* const sName)
{
	m_cATLObjectNames[nObjectID] = sName;
	m_bATLObjectsChanged = true;
}

///////////////////////////////////////////////////////////////////////////
void CATLDebugNameStore::AddAudioTrigger(TAudioControlID const nTriggerID, char const* const sName)
{	
	m_cATLTriggerNames[nTriggerID] = sName;
	m_bATLTriggersChanged = true;
}

///////////////////////////////////////////////////////////////////////////
void CATLDebugNameStore::AddAudioRtpc(TAudioControlID const nRtpcID, char const* const sName)
{
	m_cATLRtpcNames[nRtpcID] = sName;
	m_bATLRtpcsChanged = true;
}

///////////////////////////////////////////////////////////////////////////
void CATLDebugNameStore::AddAudioSwitch(TAudioControlID const nSwitchID, char const* const sName)
{
	m_cATLSwitchNames[nSwitchID] = std::make_pair(sName, TAudioSwitchStateMap());
	m_bATLSwitchesChanged = true;
}

///////////////////////////////////////////////////////////////////////////
void CATLDebugNameStore::AddAudioSwitchState(TAudioControlID const nSwitchID, TAudioSwitchStateID const nStateID, char const* const sName)
{
	m_cATLSwitchNames[nSwitchID].second[nStateID] = sName;
	m_bATLSwitchesChanged = true;
}

//////////////////////////////////////////////////////////////////////////
void CATLDebugNameStore::AddAudioPreloadRequest(TAudioPreloadRequestID const nRequestID, char const* const sName)
{
	m_cATLPreloadRequestNames[nRequestID] = sName;
	m_bATLPreloadsChanged = true;
}

///////////////////////////////////////////////////////////////////////////
void CATLDebugNameStore::AddAudioEnvironment(TAudioEnvironmentID const nEnvironmentID, char const* const sName)
{
	m_cATLEnvironmentNames[nEnvironmentID] = sName;
	m_bATLEnvironmentsChanged = true;
}

///////////////////////////////////////////////////////////////////////////
void CATLDebugNameStore::RemoveAudioObject(TAudioObjectID const nObjectID)
{
	m_cATLObjectNames.erase(nObjectID);
	m_bATLObjectsChanged = true;
}

///////////////////////////////////////////////////////////////////////////
void CATLDebugNameStore::RemoveAudioTrigger(TAudioControlID const nTriggerID)
{
	m_cATLTriggerNames.erase(nTriggerID);
	m_bATLTriggersChanged = true;
}

///////////////////////////////////////////////////////////////////////////
void CATLDebugNameStore::RemoveAudioRtpc(TAudioControlID const nRtpcID)
{
	m_cATLRtpcNames.erase(nRtpcID);
	m_bATLRtpcsChanged = true;
}

///////////////////////////////////////////////////////////////////////////
void CATLDebugNameStore::RemoveAudioSwitch(TAudioControlID const nSwitchID)
{
	m_cATLSwitchNames.erase(nSwitchID);
	m_bATLSwitchesChanged = true;
}

//////////////////////////////////////////////////////////////////////////
void CATLDebugNameStore::RemoveAudioPreloadRequest(TAudioPreloadRequestID const nRequestID)
{
	m_cATLPreloadRequestNames.erase(nRequestID);
	m_bATLPreloadsChanged = true;
}

//////////////////////////////////////////////////////////////////////////
void CATLDebugNameStore::RemoveAudioEnvironment(TAudioEnvironmentID const nEnvironmentID)
{
	m_cATLEnvironmentNames.erase(nEnvironmentID);
	m_bATLEnvironmentsChanged = true;
}

///////////////////////////////////////////////////////////////////////////
char const* CATLDebugNameStore::LookupAudioObjectName(TAudioObjectID const nObjectID) const
{
	TAudioObjectMap::const_iterator iPlace= m_cATLObjectNames.begin();
	char const* sResult = NPTR;

	if (FindPlaceConst(m_cATLObjectNames, nObjectID, iPlace))
	{
		sResult = iPlace->second.c_str();
	}

	return sResult;
}

///////////////////////////////////////////////////////////////////////////
char const* CATLDebugNameStore::LookupAudioTriggerName(TAudioControlID const nTriggerID) const
{
	TAudioControlMap::const_iterator iPlace= m_cATLTriggerNames.begin();
	char const* sResult = NPTR;

	if (FindPlaceConst(m_cATLTriggerNames, nTriggerID, iPlace))
	{
		sResult = iPlace->second.c_str();
	}

	return sResult;
}

///////////////////////////////////////////////////////////////////////////
char const* CATLDebugNameStore::LookupAudioRtpcName(TAudioControlID const nRtpcID) const
{
	TAudioControlMap::const_iterator iPlace= m_cATLRtpcNames.begin();
	char const* sResult = NPTR;

	if (FindPlaceConst(m_cATLRtpcNames, nRtpcID, iPlace))
	{
		sResult = iPlace->second.c_str();
	}

	return sResult;
}

///////////////////////////////////////////////////////////////////////////
char const* CATLDebugNameStore::LookupAudioSwitchName(TAudioControlID const nSwitchID) const
{
	TAudioSwitchMap::const_iterator iPlace= m_cATLSwitchNames.begin();
	char const* sResult = NPTR;

	if (FindPlaceConst(m_cATLSwitchNames, nSwitchID, iPlace))
	{
		sResult = iPlace->second.first.c_str();
	}

	return sResult;
}

///////////////////////////////////////////////////////////////////////////
char const* CATLDebugNameStore::LookupAudioSwitchStateName(TAudioControlID const nSwitchID, TAudioSwitchStateID const nStateID) const
{
	TAudioSwitchMap::const_iterator iSwitchPlace= m_cATLSwitchNames.begin();
	char const* sResult = NPTR;

	if (FindPlaceConst(m_cATLSwitchNames, nSwitchID, iSwitchPlace))
	{
		TAudioSwitchStateMap const cStates = iSwitchPlace->second.second;
		TAudioSwitchStateMap::const_iterator iStatePlace = cStates.begin();

		if (FindPlaceConst(cStates, nStateID, iStatePlace))
		{
			sResult = iStatePlace->second.c_str();
		}
	}

	return sResult;
}

//////////////////////////////////////////////////////////////////////////
char const* CATLDebugNameStore::LookupAudioPreloadRequestName(TAudioPreloadRequestID const nRequestID) const
{
	TAudioPreloadRequestsMap::const_iterator Iter(m_cATLPreloadRequestNames.begin());
	char const* sResult = NPTR;

	if (FindPlaceConst(m_cATLPreloadRequestNames, nRequestID, Iter))
	{
		sResult = Iter->second.c_str();
	}

	return sResult;
}

///////////////////////////////////////////////////////////////////////////
char const* CATLDebugNameStore::LookupAudioEnvironmentName(TAudioEnvironmentID const nEnvironmentID) const
{
	TAudioEnvironmentMap::const_iterator Iter(m_cATLEnvironmentNames.begin());
	char const* sResult = NPTR;

	if (FindPlaceConst(m_cATLEnvironmentNames, nEnvironmentID, Iter))
	{
		sResult = Iter->second.c_str();
	}

	return sResult;
}
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
