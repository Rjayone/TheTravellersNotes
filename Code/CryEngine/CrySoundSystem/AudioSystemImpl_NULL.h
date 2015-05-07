// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#ifndef AUDIOSYSTEMIMPL_NULL_H_INCLUDED
#define AUDIOSYSTEMIMPL_NULL_H_INCLUDED

#include <IAudioSystemImplementation.h>

class CAudioSystemImpl_NULL : public IAudioSystemImplementation
{
public:

	CAudioSystemImpl_NULL();
	~CAudioSystemImpl_NULL();

	//IAudioSystemImplementation
	VIRTUAL void														Update(float const fUpdateIntervalMS);

	VIRTUAL EAudioRequestStatus							Init();
	VIRTUAL EAudioRequestStatus							ShutDown();
	VIRTUAL EAudioRequestStatus							Release();
	VIRTUAL EAudioRequestStatus							OnLoseFocus();
	VIRTUAL EAudioRequestStatus							OnGetFocus();
	VIRTUAL EAudioRequestStatus							MuteAll();
	VIRTUAL EAudioRequestStatus							UnmuteAll();
	VIRTUAL EAudioRequestStatus							StopAllSounds();

	VIRTUAL EAudioRequestStatus							RegisterAudioObject(
																						IATLAudioObjectData* const pObjectData, 
																						char const* const sObjectName);
	VIRTUAL EAudioRequestStatus							RegisterAudioObject(IATLAudioObjectData* const pObjectData);
	VIRTUAL EAudioRequestStatus							UnregisterAudioObject(IATLAudioObjectData* const pObjectData);
	VIRTUAL EAudioRequestStatus							ResetAudioObject(IATLAudioObjectData* const pObjectData);
	VIRTUAL EAudioRequestStatus							UpdateAudioObject(IATLAudioObjectData* const pObjectData);

	VIRTUAL EAudioRequestStatus							PrepareTriggerSync(
																						IATLAudioObjectData* const pAudioObjectData, 
																						IATLTriggerImplData const* const pTriggerDataData);
	VIRTUAL EAudioRequestStatus							UnprepareTriggerSync(
																						IATLAudioObjectData* const pAudioObjectData, 
																						IATLTriggerImplData const* const pTriggerData);
	VIRTUAL EAudioRequestStatus							PrepareTriggerAsync(
																						IATLAudioObjectData* const pAudioObjectData, 
																						IATLTriggerImplData const* const pTriggerData,
																						IATLEventData* const pEventData);
	VIRTUAL EAudioRequestStatus							UnprepareTriggerAsync(
																						IATLAudioObjectData* const pAudioObjectData, 
																						IATLTriggerImplData const* const pTriggerData,
																						IATLEventData* const pEventData);
	VIRTUAL EAudioRequestStatus							ActivateTrigger(
																						IATLAudioObjectData* const pAudioObjectData,
																						IATLTriggerImplData const* const pTriggerData,
																						IATLEventData* const pEventData);
	VIRTUAL EAudioRequestStatus							StopEvent(
																						IATLAudioObjectData* const pAudioObjectData,
																						IATLEventData const* const pEventData);
	VIRTUAL EAudioRequestStatus							StopAllEvents(IATLAudioObjectData* const pAudioObjectData);
	VIRTUAL EAudioRequestStatus							SetPosition(
																						IATLAudioObjectData* const pAudioObjectData,
																						SATLWorldPosition const& sWorldPosition);
	VIRTUAL EAudioRequestStatus							SetEnvironment(
																						IATLAudioObjectData* const pAudioObjectData,
																						IATLEnvironmentImplData const* const pEnvironmentImplData,
																						float const fAmount);
	VIRTUAL EAudioRequestStatus							SetRtpc(
																						IATLAudioObjectData* const pAudioObjectData, 
																						IATLRtpcImplData const* const pRtpcData, 
																						float const fValue);
	VIRTUAL EAudioRequestStatus							SetSwitchState(
																						IATLAudioObjectData* const pAudioObjectData, 
																						IATLSwitchStateImplData const* const pSwitchStateData);
	VIRTUAL EAudioRequestStatus							SetObstructionOcclusion(
																						IATLAudioObjectData* const pAudioObjectData,
																						float const fObstruction,
																						float const fOcclusion);

	VIRTUAL EAudioRequestStatus							SetListenerPosition(
																						IATLListenerData* const pListenerData,
																						SATLWorldPosition const& oNewPosition);

	VIRTUAL EAudioRequestStatus							RegisterInMemoryFile(SATLAudioFileEntryInfo* const pAudioFileEntry);
	VIRTUAL EAudioRequestStatus							UnregisterInMemoryFile(SATLAudioFileEntryInfo* const pAudioFileEntry);
	
	VIRTUAL EAudioRequestStatus							ParseAudioFileEntry(XmlNodeRef const pAudioFileEntryNode, SATLAudioFileEntryInfo* const pFileEntryInfo);
	VIRTUAL void														DeleteAudioFileEntryData(IATLAudioFileEntryData* const pOldAudioFileEntryData);
	VIRTUAL char const* const								GetAudioFileLocation(SATLAudioFileEntryInfo* const pFileEntryInfo);

	VIRTUAL IATLTriggerImplData const*			NewAudioTriggerImplData(XmlNodeRef const pAudioTriggerNode);
	VIRTUAL void														DeleteAudioTriggerImplData(IATLTriggerImplData const* const pOldTriggerImplData);
	
	VIRTUAL IATLRtpcImplData const*					NewAudioRtpcImplData(XmlNodeRef const pAudioRtpcNode);
	VIRTUAL void														DeleteAudioRtpcImplData(IATLRtpcImplData const* const pOldRtpcImplData);

	VIRTUAL IATLSwitchStateImplData const*	NewAudioSwitchStateImplData(XmlNodeRef const pAudioSwitchStateImplNode);
	VIRTUAL void														DeleteAudioSwitchStateImplData(IATLSwitchStateImplData const* const pOldAudioSwitchStateImplNode);
	
	VIRTUAL IATLEnvironmentImplData const*	NewAudioEnvironmentImplData(XmlNodeRef const pAudioEnvironmentNode);
	VIRTUAL void														DeleteAudioEnvironmentImplData(IATLEnvironmentImplData const* const pOldEnvironmentImplData);

	VIRTUAL IATLAudioObjectData*						NewGlobalAudioObjectData(TAudioObjectID const nObjectID);
	VIRTUAL IATLAudioObjectData*						NewAudioObjectData(TAudioObjectID const nObjectID);
	VIRTUAL void														DeleteAudioObjectData(IATLAudioObjectData* const pOldObjectData);

	VIRTUAL IATLListenerData*								NewDefaultAudioListenerObjectData();
	VIRTUAL IATLListenerData*								NewAudioListenerObjectData(uint const nIndex);
	VIRTUAL void														DeleteAudioListenerObjectData(IATLListenerData* const pOldListenerData);

	VIRTUAL IATLEventData*									NewAudioEventData(TAudioEventID const nEventID);
	VIRTUAL void														DeleteAudioEventData(IATLEventData* const pOldEventData);
	VIRTUAL void														ResetAudioEventData(IATLEventData* const pEventData);

	VIRTUAL void														OnAudioSystemRefresh();
	VIRTUAL void														SetLanguage(char const* const sLanguage);

	// Below data is only used when INCLUDE_AUDIO_PRODUCTION_CODE is defined!
	VIRTUAL char const* const								GetImplementationNameString() const {return "NULL System";}
	VIRTUAL void														GetMemoryInfo(SAudioImplMemoryInfo& oMemoryInfo) const;
};
#endif // AUDIOSYSTEMIMPL_NULL_H_INCLUDED
