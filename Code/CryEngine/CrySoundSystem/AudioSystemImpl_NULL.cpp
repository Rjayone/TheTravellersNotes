// CryEngine Source File.
// Copyright (C), Crytek, 1999-2014.

#include "stdafx.h"
#include "AudioSystemImpl_NULL.h"

///////////////////////////////////////////////////////////////////////////
CAudioSystemImpl_NULL::CAudioSystemImpl_NULL()
{
}

///////////////////////////////////////////////////////////////////////////
CAudioSystemImpl_NULL::~CAudioSystemImpl_NULL()
{
}

///////////////////////////////////////////////////////////////////////////
void CAudioSystemImpl_NULL::Update(float const fUpdateIntervalMS)
{
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::Init()
{
	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::ShutDown()
{
	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::Release()
{
	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::OnLoseFocus()
{
	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::OnGetFocus()
{
	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::MuteAll()
{
	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::UnmuteAll()
{
	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::StopAllSounds()
{
	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::RegisterAudioObject(IATLAudioObjectData* const pObject)
{
	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::RegisterAudioObject(IATLAudioObjectData* const pObject, char const* const sObjectName)
{
	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::UnregisterAudioObject(IATLAudioObjectData* const pObject)
{
	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::ResetAudioObject(IATLAudioObjectData* const pObject)
{
	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::UpdateAudioObject(IATLAudioObjectData* const pObject)
{
	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::PrepareTriggerSync(IATLAudioObjectData* const pAudioObject, IATLTriggerImplData const* const pTrigger)
{
	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::UnprepareTriggerSync(IATLAudioObjectData* const pAudioObject, IATLTriggerImplData const* const pTrigger)
{
	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::PrepareTriggerAsync(IATLAudioObjectData* const pAudioObject, IATLTriggerImplData const* const pTrigger, IATLEventData* const pEvent)
{
	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::UnprepareTriggerAsync(IATLAudioObjectData* const pAudioObject, IATLTriggerImplData const* const pTrigger, IATLEventData* const pEvent)
{
	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::ActivateTrigger(IATLAudioObjectData* const pAudioObject, IATLTriggerImplData const* const pTrigger, IATLEventData* const pEvent)
{
	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::StopEvent(IATLAudioObjectData* const pAudioObject, IATLEventData const* const pEvent)
{
	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::StopAllEvents(IATLAudioObjectData* const pAudioObject)
{
	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::SetPosition(IATLAudioObjectData* const pAudioObject, SATLWorldPosition const& sWorldPosition)
{
	return eARS_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::SetEnvironment(IATLAudioObjectData* const pAudioObject, IATLEnvironmentImplData const* const pEnvironmentImpl, float const fValue)
{
	return eARS_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::SetRtpc(IATLAudioObjectData* const pAudioObject, IATLRtpcImplData const* const pRtpc, float const fValue)
{
	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::SetSwitchState(IATLAudioObjectData* const pAudioObject, IATLSwitchStateImplData const* const pState)
{
	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::SetObstructionOcclusion(IATLAudioObjectData* const pAudioObject, float const fObstruction, float const fOcclusion)
{
	return eARS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::SetListenerPosition(IATLListenerData* const pListener, SATLWorldPosition const& oNewPosition)
{
	return eARS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::RegisterInMemoryFile(SATLAudioFileEntryInfo* const pAudioFileEntry)
{
	return eARS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::UnregisterInMemoryFile(SATLAudioFileEntryInfo* const pAudioFileEntry)
{
	return eARS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioSystemImpl_NULL::ParseAudioFileEntry(XmlNodeRef const pAudioFileEntryNode, SATLAudioFileEntryInfo* const pFileEntryInfo)
{
	pFileEntryInfo->nMemoryBlockAlignment = 0;
	pFileEntryInfo->nSize = 0;
	pFileEntryInfo->bLocalized = false;
	pFileEntryInfo->pFileData = NPTR;
	pFileEntryInfo->pImplData = NPTR;
	pFileEntryInfo->sFileName = NPTR;
	return eARS_FAILURE; // This is the correct behavior: the NULL implementation does not recognize any EntryNodes.
}

//////////////////////////////////////////////////////////////////////////
void CAudioSystemImpl_NULL::DeleteAudioFileEntryData(IATLAudioFileEntryData* const pOldAudioFileEntry)
{
}

//////////////////////////////////////////////////////////////////////////
char const* const CAudioSystemImpl_NULL::GetAudioFileLocation(SATLAudioFileEntryInfo* const pFileEntryInfo)
{
	return NPTR;
}

///////////////////////////////////////////////////////////////////////////
IATLAudioObjectData* CAudioSystemImpl_NULL::NewGlobalAudioObjectData(TAudioObjectID const nObjectID)
{
	POOL_NEW_CREATE(IATLAudioObjectData, pNewObject)();
	return pNewObject;
}

///////////////////////////////////////////////////////////////////////////
IATLAudioObjectData* CAudioSystemImpl_NULL::NewAudioObjectData(TAudioObjectID const nObjectID)
{
	POOL_NEW_CREATE(IATLAudioObjectData, pNewObject)();
	return pNewObject;
}

///////////////////////////////////////////////////////////////////////////
void CAudioSystemImpl_NULL::DeleteAudioObjectData(IATLAudioObjectData* const pOldObject)
{
	POOL_FREE(pOldObject);
}

///////////////////////////////////////////////////////////////////////////
IATLListenerData* CAudioSystemImpl_NULL::NewAudioListenerObjectData(uint const nIndex)
{
	POOL_NEW_CREATE(IATLListenerData, pNewObject)();
	return pNewObject;
}

///////////////////////////////////////////////////////////////////////////
void CAudioSystemImpl_NULL::DeleteAudioListenerObjectData(IATLListenerData* const pOldListener)
{
	POOL_FREE(pOldListener);
}

///////////////////////////////////////////////////////////////////////////
IATLListenerData* CAudioSystemImpl_NULL::NewDefaultAudioListenerObjectData()
{
	POOL_NEW_CREATE(IATLListenerData, pNewObject)();
	return pNewObject;
}

///////////////////////////////////////////////////////////////////////////
IATLEventData* CAudioSystemImpl_NULL::NewAudioEventData(TAudioEventID const nEventID)
{
	POOL_NEW_CREATE(IATLEventData, pNewEvent)();
	return pNewEvent;
}

///////////////////////////////////////////////////////////////////////////
void CAudioSystemImpl_NULL::DeleteAudioEventData(IATLEventData* const pOldEventImpl)
{
	POOL_FREE(pOldEventImpl);
}

///////////////////////////////////////////////////////////////////////////
void CAudioSystemImpl_NULL::ResetAudioEventData(IATLEventData* const pEventData)
{
}

///////////////////////////////////////////////////////////////////////////
IATLTriggerImplData const* CAudioSystemImpl_NULL::NewAudioTriggerImplData(XmlNodeRef const pAudioTriggerNode)
{
	return NPTR;
}

///////////////////////////////////////////////////////////////////////////
void CAudioSystemImpl_NULL::DeleteAudioTriggerImplData(IATLTriggerImplData const* const pOldTriggerImpl)
{
}

///////////////////////////////////////////////////////////////////////////
IATLRtpcImplData const* CAudioSystemImpl_NULL::NewAudioRtpcImplData(XmlNodeRef const pAudioRtpcNode)
{
	return NPTR;
}

///////////////////////////////////////////////////////////////////////////
void CAudioSystemImpl_NULL::DeleteAudioRtpcImplData(IATLRtpcImplData const* const pOldRtpcImpl)
{
}

///////////////////////////////////////////////////////////////////////////
IATLSwitchStateImplData const* CAudioSystemImpl_NULL::NewAudioSwitchStateImplData(XmlNodeRef const pAudioSwitchNode)
{
	return NPTR;
}

///////////////////////////////////////////////////////////////////////////
void CAudioSystemImpl_NULL::DeleteAudioSwitchStateImplData(IATLSwitchStateImplData const* const pOldSwitchState)
{
}

///////////////////////////////////////////////////////////////////////////
IATLEnvironmentImplData const* CAudioSystemImpl_NULL::NewAudioEnvironmentImplData(XmlNodeRef const pAudioEnvironmentNode)
{
	return NPTR;
}

///////////////////////////////////////////////////////////////////////////
void CAudioSystemImpl_NULL::DeleteAudioEnvironmentImplData(IATLEnvironmentImplData const* const pOldEnvironmentImpl)
{
}

///////////////////////////////////////////////////////////////////////////
void CAudioSystemImpl_NULL::GetMemoryInfo(SAudioImplMemoryInfo& oMemoryInfo) const
{
	oMemoryInfo.nPrimaryPoolSize = 0;
	oMemoryInfo.nPrimaryPoolUsedSize = 0;
	oMemoryInfo.nPrimaryPoolAllocations = 0;
	oMemoryInfo.nSecondaryPoolSize = 0;
	oMemoryInfo.nSecondaryPoolUsedSize = 0;
	oMemoryInfo.nSecondaryPoolAllocations = 0;
	oMemoryInfo.nBucketUsedSize = 0;
	oMemoryInfo.nBucketAllocations = 0;
}

//////////////////////////////////////////////////////////////////////////
void CAudioSystemImpl_NULL::OnAudioSystemRefresh()
{
}

//////////////////////////////////////////////////////////////////////////
void CAudioSystemImpl_NULL::SetLanguage(char const* const sLanguage)
{
}
