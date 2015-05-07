// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#ifndef ATL_H_INCLUDED
#define ATL_H_INCLUDED

#include "ATLEntities.h"
#include "ATLComponents.h"
#include "FileCacheManager.h"

class CAudioTranslationLayer : public ISystemEventListener
{
public:

	CAudioTranslationLayer();
	virtual ~CAudioTranslationLayer();

	// ISystemEventListener
	VIRTUAL void					OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam);
	// ~ISystemEventListener

	bool									Initialize(IAudioSystemImplementation* pImpl);
	bool									ShutDown();

	void									ProcessRequest(CAudioRequestInternal& rRequest);
	void									Update(float const fUpdateIntervalMS);

	bool									GetAudioTriggerID(char const* const sAudioTriggerName, TAudioControlID& rAudioTriggerID) const;
	bool									GetAudioRtpcID(char const* const sAudioRtpcName, TAudioControlID& rAudioRtpcID) const;
	bool									GetAudioSwitchID(char const* const sAudioStateName, TAudioControlID& rAudioSwitchID) const;
	bool									GetAudioSwitchStateID(TAudioControlID const nSwitchID, char const* const sAudioSwitchStateName, TAudioSwitchStateID& rAudioStateID) const;
	bool									GetAudioPreloadRequestID(char const* const sAudioPreloadRequestName, TAudioPreloadRequestID& rAudioPreloadRequestID) const;
	bool									GetAudioEnvironmentID(char const* const sAudioEnvironmentName, TAudioEnvironmentID& rAudioEnvironmentID) const;

	bool									ReserveAudioObjectID(TAudioObjectID& rAudioObjectID); 
	bool									ReleaseAudioObjectID(TAudioObjectID const nAudioObjectID);

	bool									ReserveAudioListenerID(TAudioObjectID& rAudioObjectID);
	bool									ReleaseAudioListenerID(TAudioObjectID const nAudioObjectID);

	EAudioRequestStatus		ParseControlsData(char const* const pFolderPath, EATLDataScope const eDataScope);
	EAudioRequestStatus		ClearControlsData(EATLDataScope const eDataScope);
	EAudioRequestStatus		ParsePreloadsData(char const* const pFolderPath, EATLDataScope const eDataScope);
	EAudioRequestStatus		ClearPreloadsData(EATLDataScope const eDataScope);

	void									NotifyListener(CAudioRequestInternal const& rRequest);

private:

	CAudioTranslationLayer(CAudioTranslationLayer const& rOther);						//copy protection
	CAudioTranslationLayer& operator=(CAudioTranslationLayer const& rOther);//copy protection

	EAudioRequestStatus		ProcessAudioManagerRequest(CAudioRequestInternal const& rRequest);
	EAudioRequestStatus		ProcessAudioCallbackManagerRequest(SAudioRequestDataInternal const* const pPassedRequestData);
	EAudioRequestStatus		ProcessGlobalObjectRequest(SAudioRequestDataInternal const* const pPassedRequestData);
	EAudioRequestStatus		ProcessAudioObjectRequest(
													CATLAudioObjectBase* const pObject, 
													SAudioRequestDataInternal const* const pPassedRequestData);
	EAudioRequestStatus		ProcessAudioListenertRequest(
													CATLListenerObject* const pListener, 
													SAudioRequestDataInternal const* const pPassedRequestData);

	void									SetpImpl(IAudioSystemImplementation* const pImplNew);
	void									ReleasepImpl();

	EAudioRequestStatus		PrepUnprepTriggerAsync(
													CATLAudioObjectBase* const pAudioObject, 
													CATLTrigger const* const pTrigger,
													bool const bPrepare);
	EAudioRequestStatus		ActivateTrigger(
													CATLAudioObjectBase* const pAudioObject, 
													CATLTrigger const* const pTrigger, 
													float const fTimeUntilRemovalMS,
													TTriggerFinishedCallback const pCallback = NPTR,
													void* const pCallbackCookie = NPTR,
													TATLEnumFlagsType const nFlags = 0);
	EAudioRequestStatus		StopTrigger(
													CATLAudioObjectBase* const pAudioObject, 
													CATLTrigger const* const pTrigger);
	EAudioRequestStatus		StopAllTriggers(CATLAudioObjectBase* const pAudioObject);
	EAudioRequestStatus		SetSwitchState(
													CATLAudioObjectBase* const pAudioObject, 
													CATLSwitchState const* const pState);
	EAudioRequestStatus		SetRtpc(
													CATLAudioObjectBase* const pAudioObject, 
													CATLRtpc const* const pRtpc, 
													float const fValue);
	EAudioRequestStatus		SetEnvironment(
													CATLAudioObjectBase* const pAudioObject, 
													CATLAudioEnvironment const* const pEnvironment, 
													float const fAmount);
	EAudioRequestStatus		ResetEnvironments(CATLAudioObjectBase* const pAudioObject);

	EAudioRequestStatus		ActivateInternalTrigger(
													CATLAudioObjectBase* const pAudioObject, 
													IATLTriggerImplData const* const pTriggerData,
													IATLEventData* const pEventData);
	EAudioRequestStatus		StopInternalEvent(
													CATLAudioObjectBase* const pAudioObject,
													IATLEventData const* const pEventData);
	EAudioRequestStatus		StopAllInternalEvents(CATLAudioObjectBase* const pAudioObject);
	EAudioRequestStatus		SetInternalRtpc(
													CATLAudioObjectBase* const pAudioObject,
													IATLRtpcImplData const* const pRtpcData, 
													float const fValue);
	EAudioRequestStatus		SetInternalSwitchState(
													CATLAudioObjectBase* const pAudioObject,
													IATLSwitchStateImplData const* const pSwitchStateData);
	EAudioRequestStatus		SetInternalEnvironment(
													CATLAudioObjectBase* const pAudioObject,
													IATLEnvironmentImplData const* const pEnvironmentImplData,
													float const fAmount);

	void									UpdateSharedData();
	EAudioRequestStatus		RefreshAudioSystem(char const* const sLevelName);
	void									SetImplLanguage();

	// Regularly updated common data to be referenced by all components.
	SATLSharedData												m_oSharedData;

	// ATLObject containers
	TATLTriggerLookup											m_cTriggers;
	TATLRtpcLookup												m_cRtpcs;
	TATLSwitchLookup											m_cSwitches;
	TATLPreloadRequestLookup							m_cPreloadRequests;
	TATLEnvironmentLookup									m_cEnvironments;

	CATLGlobalAudioObject*								m_pGlobalAudioObject;
	TAudioObjectID const									m_nGloabalAudioObjectID;

	TAudioTriggerInstanceID								m_nTriggerInstanceIDCounter;

	// Components
	CAudioEventManager										m_oAudioEventMgr;
	CAudioObjectManager										m_oAudioObjectMgr;
	CAudioListenerManager									m_oAudioListenerMgr;
	CFileCacheManager											m_oFileCacheMgr;
	CAudioEventListenerManager						m_oAudioEventListenerMgr;
	CATLXMLProcessor											m_oXMLProcessor;

	// Utility members
	bool																	m_bIsRunning;
	bool																	m_bIsMuted;
	IAudioSystemImplementation*						m_pImpl;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
public:

	bool	ReserveAudioObjectID(TAudioObjectID& rAudioObjectID, char const* const sAudioObjectName);
	void	DrawAudioSystemDebugInfo();
	CATLDebugNameStore const& GetDebugStore() const {return m_oDebugNameStore;}

private:

	void	DrawAudioObjectDebugInfo(IRenderer* const pRenderer);
	void	DrawATLComponentDebugInfo(IRenderer* const pRenderer, float fPosX, float const fPosY);

	CATLDebugNameStore										m_oDebugNameStore;
	CryFixedStringT<ATL_NAME_MAX_LENGTH>	m_sImplementationNameString;
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
};

#endif // ATL_H_INCLUDED
