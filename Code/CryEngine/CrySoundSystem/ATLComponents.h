// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#ifndef ATLCOMPONENTS_H_INCLUDED
#define ATLCOMPONENTS_H_INCLUDED

#include "FileCacheManager.h"
#include "ATLUtils.h"
#include "ATLEntities.h"
#include "ATLAudioObject.h"
#include <IAudioSystem.h>

//---------------- ATL implementation internal classes ------------------------
class CAudioEventManager
{
public:

	CAudioEventManager();
	virtual ~CAudioEventManager();

	void				Init(IAudioSystemImplementation* const pImpl);
	void				Release();
	void				Update(float const fUpdateIntervalMS);

	CATLEvent*	GetEvent(EATLSubsystem const eSender);
	CATLEvent*	LookupID(TAudioEventID const nID) const;
	void				ReleaseEvent(CATLEvent* const pEvent);

	size_t			GetNumActive() const;

private:

	CAudioEventManager(CAudioEventManager const&);						// not defined; calls will fail at compile time
	CAudioEventManager& operator=(CAudioEventManager const&);	// not defined; calls will fail at compile time

	CATLEvent*	GetImplInstance();
	void				ReleaseImplInstance(CATLEvent* const pOldEvent);
	CATLEvent*	GetInternalInstance();
	void				ReleaseInternalInstance(CATLEvent* const pOldEvent);

	typedef std::map<TAudioEventID, CATLEvent*, std::less<TAudioEventID>, STLSoundAllocator<std::pair<TAudioEventID, CATLEvent*> > >
		TActiveEventMap;

	TActiveEventMap															m_cActiveAudioEvents;
	CInstanceManager<CATLEvent, TAudioEventID>	m_oAudioEventPool;
	IAudioSystemImplementation*									m_pASImpl;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
public:

	void							SetDebugNameStore(CATLDebugNameStore const* const pDebugNameStore);
	void							DrawDebugInfo(IRenderer* const pRenderer, float fPosX, float fPosY) const;

private:

	CATLDebugNameStore const*										m_pDebugNameStore;
#endif //INCLUDE_AUDIO_PRODUCTION_CODE
};

class CAudioObjectManager
{
public:

	CAudioObjectManager();
	virtual ~CAudioObjectManager();

	void							Init(IAudioSystemImplementation* const pImpl);
	void							Release();
	void							Update(float const fUpdateIntervalMS, SATLWorldPosition const& rListenerPosition);

	bool							ReserveID(TAudioObjectID& rAudioObjectID);
	bool							ReserveThisID(TAudioObjectID const nAudioObjectID);
	bool							ReleaseID(TAudioObjectID const nAudioObjectID);	
	CATLAudioObject*	LookupID(TAudioObjectID const nAudioObjectID) const;

	void							ReportFinishedEvent(CATLEvent const* const pEvent, bool const bSuccess);
	void							ReportObstructionRay(TAudioObjectID const nAudioObjectID, size_t const nRayID);

	void							ReleasePendingRays();

private:

	CAudioObjectManager(CAudioObjectManager const&);						// not defined; calls will fail at compile time
	CAudioObjectManager& operator=(CAudioObjectManager const&);	// not defined; calls will fail at compile time

	CATLAudioObject*	GetInstance();
	bool							ReleaseInstance(CATLAudioObject* const pOldObject);

	typedef std::map<TAudioObjectID, CATLAudioObject*, std::less<TAudioObjectID>, STLSoundAllocator<std::pair<TAudioObjectID, CATLAudioObject*> > >
		TActiveObjectMap;

	TActiveObjectMap																	m_cAudioObjects;
	CInstanceManager<CATLAudioObject, TAudioObjectID>	m_cObjectPool;
	IAudioSystemImplementation*												m_pASImpl;

	static TAudioObjectID const												s_nMinAudioObjectID;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
public:

	bool							ReserveID(TAudioObjectID& rAudioObjectID, char const* const sAudioObjectName);
	void							SetDebugNameStore(CATLDebugNameStore* const pDebugNameStore);
	size_t						GetNumAudioObjects() const;
	size_t						GetNumActiveAudioObjects() const;
	void							DrawPerObjectDebugInfo(IRenderer* const pRenderer, Vec3 const& vListenerPos) const;
	void							DrawDebugInfo(IRenderer* const pRenderer, float fPosX, float fPosY) const;

private:

	CATLDebugNameStore*																m_pDebugNameStore;
#endif //INCLUDE_AUDIO_PRODUCTION_CODE
};

class CAudioListenerManager
{
public:

	CAudioListenerManager();
	~CAudioListenerManager();

	void								Init(IAudioSystemImplementation* const pImpl);
	void								Release();
	void								Update(float const fUpdateIntervalMS);

	bool								ReserveID(TAudioObjectID& rAudioObjectID);
	bool								ReleaseID(TAudioObjectID const nAudioObjectID);	
	CATLListenerObject*	LookupID(TAudioObjectID const nAudioObjectID) const;

	size_t							GetNumActive() const;
	void								GetDefaultListenerPosition(SATLWorldPosition& oPosition);
	TAudioObjectID			GetDefaultListenerID() const {return m_nDefaultListenerID;}

private:

	typedef std::vector<CATLListenerObject*, STLSoundAllocator<CATLListenerObject*> > TListenerPtrContainer;
	typedef std::map<TAudioObjectID, CATLListenerObject*, std::less<TAudioObjectID>, STLSoundAllocator<std::pair<TAudioObjectID, CATLListenerObject*> > >
		TActiveListenerMap;

	CAudioListenerManager(CAudioListenerManager const&);						// not defined; calls will fail at compile time
	CAudioListenerManager& operator=(CAudioListenerManager const&);	// not defined; calls will fail at compile time

	TActiveListenerMap					m_cActiveListeners;
	TListenerPtrContainer				m_cListenerPool;
	CATLListenerObject*					m_pDefaultListenerObject;
	TAudioObjectID const				m_nDefaultListenerID;
	size_t											m_nMaxNumberListeners;
	IAudioSystemImplementation*	m_pASImpl;
};

class CAudioEventListenerManager
{
public:

	CAudioEventListenerManager();
	~CAudioEventListenerManager();

	EAudioRequestStatus	AddRequestListener(void (*func)(SAudioRequestInfo const* const), void const* const pRequestOwner);
	EAudioRequestStatus	RemoveRequestListener(void (*func)(SAudioRequestInfo const* const), void const* const pRequestOwner);
	void								NotifyListener(SAudioRequestInfo const* const pRequestInfo);

private:

	CAudioEventListenerManager(CAudioEventListenerManager const& other);						// Copy protection
	CAudioEventListenerManager& operator=(CAudioEventListenerManager const& other); // Copy protection

	typedef std::vector<SAudioEventListener, STLSoundAllocator<SAudioEventListener> > TListenerArray;
	TListenerArray	m_cListeners;
};

class CATLXMLProcessor
{
public:

	CATLXMLProcessor(
		TATLTriggerLookup& rTriggers,
		TATLRtpcLookup& rRtpcs,
		TATLSwitchLookup& rSwitches,
		TATLEnvironmentLookup& rEnvironments,
		TATLPreloadRequestLookup&	rPreloadRequests,
		CFileCacheManager& rFileCacheMgr);
	
	~CATLXMLProcessor();

	void Init(IAudioSystemImplementation* const pImpl);
	void Release();

	void	ParseControlsData(char const* const sFolderPath, EATLDataScope const eDataScope);
	void	ClearControlsData(EATLDataScope const eDataScope);
	void	ParsePreloadsData(char const* const sFolderPath, EATLDataScope const eDataScope);
	void	ClearPreloadsData(EATLDataScope const eDataScope);

private:
	
	void	ParseAudioTriggers(XmlNodeRef const pXMLTriggerRoot, EATLDataScope const eDataScope);
	void	ParseAudioSwitches(XmlNodeRef const pXMLSwitchRoot, EATLDataScope const eDataScope);
	void	ParseAudioRtpcs(XmlNodeRef const pXMLRtpcRoot, EATLDataScope const eDataScope);
	void	ParseAudioPreloads(XmlNodeRef const pPreloadDataRoot, EATLDataScope const eDataScope, char const* const sFolderName);
	void	ParseAudioEnvironments(XmlNodeRef const pAudioEnvironmentRoot, EATLDataScope const eDataScope);
	
	IATLTriggerImplData const*			NewAudioTriggerImplDataInternal(XmlNodeRef const pXMLTriggerRoot);
	IATLRtpcImplData const*					NewAudioRtpcImplDataInternal(XmlNodeRef const pXMLRtpcRoot);
	IATLSwitchStateImplData const*	NewAudioSwitchStateImplDataInternal(XmlNodeRef const pXMLSwitchRoot);
	IATLEnvironmentImplData const*	NewAudioEnvironmentImplDataInternal(XmlNodeRef const pXMLEnvironmentRoot);

	void														DeleteAudioTrigger(CATLTrigger const* const pOldTrigger);
	void														DeleteAudioRtpc(CATLRtpc const* const pOldRtpc);
	void														DeleteAudioSwitch(CATLSwitch const* const pOldSwitch);
	void														DeleteAudioPreloadRequest(CATLPreloadRequest const* const pOldPreloadRequest);
	void														DeleteAudioEnvironment(CATLAudioEnvironment const* const pOldEnvironment);

	TATLTriggerLookup&							m_rTriggers;
	TATLRtpcLookup&									m_rRtpcs;
	TATLSwitchLookup&								m_rSwitches;
	TATLEnvironmentLookup&					m_rEnvironments;
	TATLPreloadRequestLookup&				m_rPreloadRequests;

	TAudioTriggerImplID							m_nTriggerImplIDCounter;

	CFileCacheManager&							m_rFileCacheMgr;

	IAudioSystemImplementation*			m_pImpl;
		
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
public:

	void SetDebugNameStore(CATLDebugNameStore* const pDebugNameStore);

private:

	CATLDebugNameStore*							m_pDebugNameStore; 
#endif //INCLUDE_AUDIO_PRODUCTION_CODE
};

struct SATLSharedData
{
	SATLSharedData();
	~SATLSharedData();

	SATLWorldPosition m_oActiveListenerPosition;
};
#endif // ATLCOMPONENTS_H_INCLUDED
