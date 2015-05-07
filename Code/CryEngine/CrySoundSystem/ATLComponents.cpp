// CryEngine Source File.
// Copyright (C), Crytek, 1999-2014.

#include "stdafx.h"
#include "ATLComponents.h"
#include "SoundCVars.h"
#include <IAudioSystemImplementation.h>
#include <IPhysics.h>
#include <ISurfaceType.h>
#include <I3DEngine.h>

///////////////////////////////////////////////////////////////////////////
CAudioEventManager::CAudioEventManager()
	: m_oAudioEventPool(g_SoundCVars.m_nAudioEventPoolSize, 1)
	, m_pASImpl(NPTR)
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	, m_pDebugNameStore(NPTR)
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
{
}

///////////////////////////////////////////////////////////////////////////
CAudioEventManager::~CAudioEventManager()
{
	if (m_pASImpl != NPTR)
	{	
		Release();
	}

	stl::free_container(m_oAudioEventPool.m_cReserved);
}

///////////////////////////////////////////////////////////////////////////
void CAudioEventManager::Init(IAudioSystemImplementation* const pImpl)
{
	m_pASImpl = pImpl;

	for (size_t i = 0; i < m_oAudioEventPool.m_nReserveSize; ++i)
	{
		TAudioEventID const nEventID = m_oAudioEventPool.GetNextID();
		IATLEventData* pNewEventData = m_pASImpl->NewAudioEventData(nEventID);
		POOL_NEW_CREATE(CATLEvent, pNewEvent)(nEventID, eAS_AUDIO_SYSTEM_IMPLEMENTATION, pNewEventData);
		m_oAudioEventPool.m_cReserved.push_back(pNewEvent);
	}
}

///////////////////////////////////////////////////////////////////////////
void CAudioEventManager::Release()
{
	CInstanceManager<CATLEvent, TAudioEventID>::TPointerContainer::iterator Iter(m_oAudioEventPool.m_cReserved.begin());
	CInstanceManager<CATLEvent, TAudioEventID>::TPointerContainer::const_iterator const IterEnd(m_oAudioEventPool.m_cReserved.end());

	for (;Iter != IterEnd; ++Iter)
	{
		m_pASImpl->DeleteAudioEventData((*Iter)->m_pImplData);
		POOL_FREE(*Iter);
	}

	m_pASImpl = NPTR;
	m_oAudioEventPool.m_cReserved.clear();
}

///////////////////////////////////////////////////////////////////////////
void CAudioEventManager::Update(float fUpdateIntervalMS)
{
	//TODO: implement 
}

///////////////////////////////////////////////////////////////////////////
CATLEvent* CAudioEventManager::GetEvent(EATLSubsystem const eSender)
{
	CATLEvent* pATLEvent = NPTR;

	switch (eSender)
	{
	case eAS_AUDIO_SYSTEM_IMPLEMENTATION:
		{
			pATLEvent = GetImplInstance();
			
			break;
		}
	case eAS_ATL_INTERNAL:
		{
			pATLEvent = GetInternalInstance();

			break;
		}
	default:
		{
			assert(false); // Unknown sender!
		}
	}

	if (pATLEvent != NPTR)
	{
		m_cActiveAudioEvents[pATLEvent->GetID()] = pATLEvent;
	}

	return pATLEvent;
}

///////////////////////////////////////////////////////////////////////////
CATLEvent* CAudioEventManager::LookupID(TAudioEventID const nID) const
{
	TActiveEventMap::const_iterator iPlace = m_cActiveAudioEvents.begin();
	bool const bLookupResult = 	FindPlaceConst(m_cActiveAudioEvents, nID, iPlace);

	return bLookupResult ? iPlace->second : NPTR;
}

///////////////////////////////////////////////////////////////////////////
void CAudioEventManager::ReleaseEvent(CATLEvent* const pEvent)
{
	if (pEvent != NPTR)
	{
		m_cActiveAudioEvents.erase(pEvent->GetID());

		switch(pEvent->m_eSender)
		{
		case eAS_AUDIO_SYSTEM_IMPLEMENTATION:
			{
				ReleaseImplInstance(pEvent);

				break;
			}
		case eAS_ATL_INTERNAL:
			{
				ReleaseInternalInstance(pEvent);

				break;
			}
		default:
			{
				assert(false);//unknown Sender
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
CATLEvent* CAudioEventManager::GetImplInstance()
{
	// must be called within a block protected by a critical section!

	CATLEvent* pEvent = NPTR;

	if (!m_oAudioEventPool.m_cReserved.empty())
	{
		//have reserved instances
		pEvent = m_oAudioEventPool.m_cReserved.back();

		m_oAudioEventPool.m_cReserved.pop_back();
	}
	else 
	{
		//need to get a new instance
		TAudioEventID const nNewID = m_oAudioEventPool.GetNextID();

		IATLEventData* pNewEventData = m_pASImpl->NewAudioEventData(nNewID);
		POOL_NEW(CATLEvent, pEvent)(nNewID, eAS_AUDIO_SYSTEM_IMPLEMENTATION, pNewEventData);

		if (pEvent == NPTR)
		{
			--m_oAudioEventPool.m_nIDCounter;

			g_AudioLogger.Log(eALT_WARNING, "Failed to get a new instance of an AudioEvent from the implementation");
			//failed to get a new instance from the implementation
		}
	}

	return pEvent;
}

///////////////////////////////////////////////////////////////////////////
void CAudioEventManager::ReleaseImplInstance(CATLEvent* const pOldEvent)
{
	// must be called within a block protected by a critical section!

	if (pOldEvent != NPTR)
	{
		pOldEvent->Clear();

		if (m_oAudioEventPool.m_cReserved.size() < m_oAudioEventPool.m_nReserveSize)
		{
			// can return the instance to the reserved pool
			m_pASImpl->ResetAudioEventData(pOldEvent->m_pImplData);

			m_oAudioEventPool.m_cReserved.push_back(pOldEvent);
		}
		else
		{
			// the reserve pool is full, can return the instance to the implementation to dispose
			m_pASImpl->DeleteAudioEventData(pOldEvent->m_pImplData);
			
			POOL_FREE(pOldEvent);
		}
	}
}

///////////////////////////////////////////////////////////////////////////
CATLEvent* CAudioEventManager::GetInternalInstance()
{
	// must be called within a block protected by a critical section!

	assert(false);// implement when it is needed
	return NPTR;
}

//////////////////////////////////////////////////////////////////////////
void CAudioEventManager::ReleaseInternalInstance(CATLEvent* const pOldEvent)
{
	// must be called within a block protected by a critical section!

	assert(false);// implement when it is needed
}

///////////////////////////////////////////////////////////////////////////
size_t CAudioEventManager::GetNumActive() const
{
	return  m_cActiveAudioEvents.size();
}

// IDs below that number are reserved for the various unique
// objects inside the AudioSystem a user may want to address
// (e.g. an AudioManager, a MusicSystem, AudioListeners?...)
TAudioObjectID const CAudioObjectManager::s_nMinAudioObjectID = 100;

///////////////////////////////////////////////////////////////////////////
CAudioObjectManager::CAudioObjectManager()
	: m_cObjectPool(g_SoundCVars.m_nAudioObjectPoolSize, s_nMinAudioObjectID)
	, m_pASImpl(NPTR)
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	, m_pDebugNameStore(NPTR)
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
{
}

///////////////////////////////////////////////////////////////////////////
CAudioObjectManager::~CAudioObjectManager()
{
	if (m_pASImpl != NPTR)
	{	
		Release();
	}

	stl::free_container(m_cObjectPool.m_cReserved);
}

///////////////////////////////////////////////////////////////////////////
void CAudioObjectManager::Update(float const fUpdateIntervalMS, SATLWorldPosition const& rListenerPosition)
{
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	//reset the ray counts
	CATLAudioObject::CPropagationProcessor::s_nTotalAsyncPhysRays = 0;
	CATLAudioObject::CPropagationProcessor::s_nTotalSyncPhysRays = 0;
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

	TActiveObjectMap::iterator iObjectPair = m_cAudioObjects.begin();
	TActiveObjectMap::const_iterator const iObjectEnd = m_cAudioObjects.end();

	for (; iObjectPair != iObjectEnd; ++iObjectPair)
	{
		CATLAudioObject* const pObject = iObjectPair->second;
		
		if (pObject->HasActiveEvents())
		{
			pObject->Update(fUpdateIntervalMS, rListenerPosition);

			if (pObject->CanRunObstructionOcclusion())
			{
				SATLSoundPropagationData oPropagationData;
				pObject->GetPropagationData(oPropagationData);

				m_pASImpl->SetObstructionOcclusion(pObject->GetImplDataPtr(), oPropagationData.fObstruction, oPropagationData.fOcclusion);
			}

			m_pASImpl->UpdateAudioObject(pObject->GetImplDataPtr());
		}
	}
}

///////////////////////////////////////////////////////////////////////////
bool CAudioObjectManager::ReserveID(TAudioObjectID& rAudioObjectID)
{
	CATLAudioObject* const pNewObject  = GetInstance();

	bool bSuccess = false;
	rAudioObjectID = INVALID_AUDIO_OBJECT_ID;

	if (pNewObject != NPTR) 
	{
		EAudioRequestStatus const eImplResult = (m_pASImpl->RegisterAudioObject(pNewObject->GetImplDataPtr()));

		if (eImplResult == eARS_SUCCESS) 
		{
			pNewObject->IncrementRefCount();
			rAudioObjectID = pNewObject->GetID();
			m_cAudioObjects.insert(std::make_pair(rAudioObjectID, pNewObject));
			bSuccess = true;
		}
		else
		{
			ReleaseInstance(pNewObject);
			bSuccess = false;
		}
	}

	return bSuccess;
}

//////////////////////////////////////////////////////////////////////////
class CObjectIDPredicate
{
public:

	CObjectIDPredicate(TAudioObjectID const nID)
		: m_nID(nID)
	{}
	
	~CObjectIDPredicate() {}

	bool operator()(CATLAudioObject const* const pObject)
	{
		return pObject->GetID() == m_nID;
	}

private:

	TAudioObjectID const m_nID;
};

///////////////////////////////////////////////////////////////////////////
bool CAudioObjectManager::ReserveThisID(TAudioObjectID const nAudioObjectID)
{
	bool bSuccess = false;
	CATLAudioObject* pObject = LookupID(nAudioObjectID);

	if (pObject == NPTR)
	{
		//no active object uses nAudioObjectID, so we can create one
		CInstanceManager<CATLAudioObject>::TPointerContainer::iterator ipObject = m_cObjectPool.m_cReserved.begin();
		// not using const for .end() iterator, because find_if seems to expect two iterators of the same type
		CInstanceManager<CATLAudioObject>::TPointerContainer::iterator const ipObjectEnd = m_cObjectPool.m_cReserved.end();

		ipObject = std::find_if(ipObject, ipObjectEnd, CObjectIDPredicate(nAudioObjectID));

		if (ipObject != ipObjectEnd)
		{
			// there is a reserved instance with the required ID
			std::swap(*ipObject, m_cObjectPool.m_cReserved.back());
			pObject = m_cObjectPool.m_cReserved.back();
			m_cObjectPool.m_cReserved.pop_back();
		}
		else
		{
			// none of the reserved instances have the required ID
			IATLAudioObjectData* const pObjectData = m_pASImpl->NewAudioObjectData(nAudioObjectID);
			POOL_NEW(CATLAudioObject, pObject)(nAudioObjectID, pObjectData);
		}

		m_cAudioObjects.insert(std::make_pair(nAudioObjectID, pObject));

		bSuccess = true;		
	}

	return bSuccess;
}

///////////////////////////////////////////////////////////////////////////
bool CAudioObjectManager::ReleaseID(TAudioObjectID const nAudioObjectID)
{
	bool bSuccess = false;
	CATLAudioObject* const pOldObject = LookupID(nAudioObjectID);

	if (pOldObject != NPTR)
	{
		if (pOldObject->GetRefCount() < 2)
		{
			bSuccess = ReleaseInstance(pOldObject);
		}
		else
		{
			pOldObject->DecrementRefCount();
		}
	}

	return bSuccess;
}

///////////////////////////////////////////////////////////////////////////
CATLAudioObject* CAudioObjectManager::LookupID(TAudioObjectID const nID) const
{
	TActiveObjectMap::const_iterator iPlace;
	CATLAudioObject* pResult = NPTR;

	if (FindPlaceConst(m_cAudioObjects, nID, iPlace))
	{
		pResult = iPlace->second;
	}

	return pResult;
}

///////////////////////////////////////////////////////////////////////////
void CAudioObjectManager::ReportFinishedEvent(CATLEvent const* const pEvent, bool const bSuccess)
{
	if (pEvent != NPTR)
	{
		CATLAudioObject* const pObject = LookupID(pEvent->m_nObjectID);

		if (pObject != NPTR)
		{
			pObject->ReportFinishedEvent(pEvent, bSuccess);

			if (pObject->GetRefCount() == 0)
			{
				ReleaseInstance(pObject);
			}
		}
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
		else
		{
			g_AudioLogger.Log(
				eALT_WARNING,
				"Removing Event %u from Object %s: Object no longer exists!",
				pEvent->GetID(),
				m_pDebugNameStore->LookupAudioObjectName(pEvent->m_nObjectID));
		}
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
	}
	else
	{
		g_AudioLogger.Log(eALT_WARNING, "NULL pEvent in CAudioObjectManager::ReportFinishedEvent");
	}
}

///////////////////////////////////////////////////////////////////////////
void CAudioObjectManager::ReportObstructionRay(TAudioObjectID const nAudioObjectID, size_t const nRayID)
{
	CATLAudioObject* const pObject = LookupID(nAudioObjectID);

	if (pObject != NPTR)
	{
		pObject->ReportPhysicsRayProcessed(nRayID);

		if (pObject->GetRefCount() == 0)
		{
			ReleaseInstance(pObject);
		}
	}
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	else
	{
		g_AudioLogger.Log(
			eALT_WARNING, 
			"Reporting Ray %" PRISIZE_T " from Object %s: Object no longer exists!", 
			nRayID, 
			m_pDebugNameStore->LookupAudioObjectName(nAudioObjectID));
	}
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
}

///////////////////////////////////////////////////////////////////////////
CATLAudioObject* CAudioObjectManager::GetInstance()
{
	CATLAudioObject* pObject = NPTR;

	if (!m_cObjectPool.m_cReserved.empty())
	{
		//have reserved instances
		pObject = m_cObjectPool.m_cReserved.back();
		m_cObjectPool.m_cReserved.pop_back();
	}
	else 
	{
		//need to get a new instance
		TAudioObjectID const nNewID = m_cObjectPool.GetNextID();
		IATLAudioObjectData* const pObjectData = m_pASImpl->NewAudioObjectData(nNewID);
		POOL_NEW(CATLAudioObject, pObject)(nNewID, pObjectData);

		if (pObject == NPTR)
		{
			--m_cObjectPool.m_nIDCounter;

			g_AudioLogger.Log(eALT_WARNING, "Failed to get a new instance of an AudioObject from the implementation");
			//failed to get a new instance from the implementation
		}
	}

	return pObject;
}

///////////////////////////////////////////////////////////////////////////
bool CAudioObjectManager::ReleaseInstance(CATLAudioObject* const pOldObject)
{
	bool bSuccess = false;
	if (pOldObject != NPTR)
	{
		TAudioObjectID const nObjectID = pOldObject->GetID();
		m_cAudioObjects.erase(nObjectID);

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
		m_pDebugNameStore->RemoveAudioObject(nObjectID);
		pOldObject->CheckBeforeRemoval(m_pDebugNameStore);
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
		pOldObject->Clear();
		bSuccess = (m_pASImpl->UnregisterAudioObject(pOldObject->GetImplDataPtr()) == eARS_SUCCESS);

		if (m_cObjectPool.m_cReserved.size() < m_cObjectPool.m_nReserveSize)
		{
			// can return the instance to the reserved pool
			m_pASImpl->ResetAudioObject(pOldObject->GetImplDataPtr());
			m_cObjectPool.m_cReserved.push_back(pOldObject);
		}
		else
		{
			// the reserve pool is full, can return the instance to the implementation to dispose
			m_pASImpl->DeleteAudioObjectData(pOldObject->GetImplDataPtr());
			POOL_FREE(pOldObject);
		}
	}

	return bSuccess;
}

///////////////////////////////////////////////////////////////////////////
void CAudioObjectManager::Init(IAudioSystemImplementation* const pImpl)
{
	m_pASImpl = pImpl;

	for (size_t i = 0; i < m_cObjectPool.m_nReserveSize; ++i)
	{
		TAudioObjectID const nObjectID = static_cast<TAudioObjectID>(m_cObjectPool.GetNextID());
		IATLAudioObjectData* const pObjectData = m_pASImpl->NewAudioObjectData(nObjectID);
		POOL_NEW_CREATE(CATLAudioObject, pObject)(nObjectID, pObjectData);
		m_cObjectPool.m_cReserved.push_back(pObject);
	}
}

///////////////////////////////////////////////////////////////////////////
void CAudioObjectManager::Release()
{
	CInstanceManager<CATLAudioObject>::TPointerContainer::iterator iter = m_cObjectPool.m_cReserved.begin();
	CInstanceManager<CATLAudioObject>::TPointerContainer::const_iterator const iEnd = 
		m_cObjectPool.m_cReserved.end();

	for ( ;iter != iEnd; ++iter)
	{
		m_pASImpl->DeleteAudioObjectData((*iter)->GetImplDataPtr());
		POOL_FREE(*iter);
	}

	m_pASImpl = NPTR;
	m_cObjectPool.m_cReserved.clear();
}

//////////////////////////////////////////////////////////////////////////
void CAudioObjectManager::ReleasePendingRays()
{
	if (!m_cAudioObjects.empty())
	{
		CInstanceManager<CATLAudioObject>::TPointerContainer aObjectsToRelease;
		TActiveObjectMap::const_iterator Iter(m_cAudioObjects.begin());
		TActiveObjectMap::const_iterator const IterEnd(m_cAudioObjects.end());

		for (; Iter != IterEnd; ++Iter)
		{
			CATLAudioObject* const pObject = Iter->second;

			if (pObject != NPTR)
			{
				pObject->ReleasePendingRays();

				if (pObject->GetRefCount() == 0)
				{
					aObjectsToRelease.push_back(pObject);
				}
			}
		}

		if (!aObjectsToRelease.empty())
		{
			CInstanceManager<CATLAudioObject>::TPointerContainer::iterator Iter2(aObjectsToRelease.begin());
			CInstanceManager<CATLAudioObject>::TPointerContainer::const_iterator const IterEnd2(aObjectsToRelease.end());

			for (; Iter2 != IterEnd2; ++Iter2)
			{
				ReleaseInstance(*Iter2);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
CAudioListenerManager::CAudioListenerManager()
	: m_nMaxNumberListeners(8)
	, m_pDefaultListenerObject(NPTR)
	,	m_nDefaultListenerID(50) // IDs 50-57 are reserved for the AudioListeners.
	, m_pASImpl(NPTR)
{
	m_cListenerPool.reserve(m_nMaxNumberListeners);
}

///////////////////////////////////////////////////////////////////////////
CAudioListenerManager::~CAudioListenerManager()
{
	if (m_pASImpl != NPTR)
	{	
		Release();
	}

	stl::free_container(m_cListenerPool);
}

///////////////////////////////////////////////////////////////////////////
void CAudioListenerManager::Init(IAudioSystemImplementation* const pImpl)
{
	m_pASImpl = pImpl;
	IATLListenerData* const pNewListenerData = m_pASImpl->NewDefaultAudioListenerObjectData();
	POOL_NEW(CATLListenerObject, m_pDefaultListenerObject)(m_nDefaultListenerID, pNewListenerData);

	if (m_pDefaultListenerObject != NPTR)
	{
		m_cActiveListeners[m_nDefaultListenerID] = m_pDefaultListenerObject;
	}

	for (size_t i = 1; i < m_nMaxNumberListeners; ++ i)
	{
		TAudioObjectID const nListenerID = m_nDefaultListenerID + i;
		IATLListenerData* const pNewListenerData = m_pASImpl->NewAudioListenerObjectData(i);
		POOL_NEW_CREATE(CATLListenerObject, pListenerObject)(m_nDefaultListenerID, pNewListenerData);
		m_cListenerPool.push_back(pListenerObject);
	}
}

///////////////////////////////////////////////////////////////////////////
void CAudioListenerManager::Release()
{
	if (m_pDefaultListenerObject != NPTR) // guard against double deletions
	{
		m_cActiveListeners.erase(m_nDefaultListenerID);

		m_pASImpl->DeleteAudioListenerObjectData(m_pDefaultListenerObject->m_pImplData);
		POOL_FREE(m_pDefaultListenerObject);
		m_pDefaultListenerObject = NPTR;
	}

	TListenerPtrContainer::iterator iter = m_cListenerPool.begin();
	TListenerPtrContainer::const_iterator const iEnd = m_cListenerPool.end();

	for (; iter != iEnd; ++iter)
	{
		m_pASImpl->DeleteAudioListenerObjectData((*iter)->m_pImplData);
		POOL_FREE(*iter);
	}

	m_pASImpl = NPTR;

	m_cListenerPool.clear();
}

///////////////////////////////////////////////////////////////////////////
void CAudioListenerManager::Update(float const fUpdateIntervalMS)
{
	//TODO: implement
}

///////////////////////////////////////////////////////////////////////////
bool CAudioListenerManager::ReserveID(TAudioObjectID& rAudioObjectID)
{
	bool bSuccess = false;

	if (!m_cListenerPool.empty())
	{
		CATLListenerObject* pListener = m_cListenerPool.back();
		m_cListenerPool.pop_back();

		TAudioObjectID const nID = pListener->GetID();
		m_cActiveListeners.insert(std::make_pair(nID, pListener));

		rAudioObjectID = nID;
		bSuccess = true;
	}

	return bSuccess;
}

///////////////////////////////////////////////////////////////////////////
bool CAudioListenerManager::ReleaseID(TAudioObjectID const nAudioObjectID)
{
	bool bSuccess = false;
	CATLListenerObject* pListener = LookupID(nAudioObjectID);

	if (pListener != NPTR)
	{
		m_cActiveListeners.erase(nAudioObjectID);
		m_cListenerPool.push_back(pListener);
		bSuccess = true;
	}

	return bSuccess;
}

///////////////////////////////////////////////////////////////////////////
CATLListenerObject* CAudioListenerManager::LookupID(TAudioObjectID const nID) const
{
	CATLListenerObject* pListenerObject = NPTR;

	TActiveListenerMap::const_iterator iPlace= m_cActiveListeners.begin();

	if (FindPlaceConst(m_cActiveListeners, nID, iPlace))
	{
		pListenerObject = iPlace->second;
	}

	return pListenerObject;
}

///////////////////////////////////////////////////////////////////////////
size_t CAudioListenerManager::GetNumActive() const
{
	return m_cActiveListeners.size();
}

///////////////////////////////////////////////////////////////////////////
void CAudioListenerManager::GetDefaultListenerPosition(SATLWorldPosition& oPosition)
{
	if (m_pDefaultListenerObject != NPTR)
	{
		oPosition = m_pDefaultListenerObject->oPosition;
	}
}

//////////////////////////////////////////////////////////////////////////
CAudioEventListenerManager::CAudioEventListenerManager()
{
}

///////////////////////////////////////////////////////////////////////////
CAudioEventListenerManager::~CAudioEventListenerManager()
{
	stl::free_container(m_cListeners);
}

//////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioEventListenerManager::AddRequestListener(void (*func)(SAudioRequestInfo const* const), void const* const pRequestOwner)
{
	EAudioRequestStatus eResult = eARS_FAILURE;
	TListenerArray::const_iterator Iter(m_cListeners.begin());
	TListenerArray::const_iterator const IterEnd(m_cListeners.end());

	for (; Iter != IterEnd; ++Iter)
	{
		if (Iter->OnEvent == func && Iter->pRequestOwner == pRequestOwner)
		{
			eResult = eARS_SUCCESS;

			break;
		}
	}

	if (eResult == eARS_FAILURE)
	{
		SAudioEventListener oAudioEventListener;
		oAudioEventListener.pRequestOwner	= pRequestOwner;
		oAudioEventListener.OnEvent				= func;
		m_cListeners.push_back(oAudioEventListener);
		eResult = eARS_SUCCESS;
	}

	return eResult;
}

//////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioEventListenerManager::RemoveRequestListener(void (*func)(SAudioRequestInfo const* const), void const* const pRequestOwner)
{
	EAudioRequestStatus eResult = eARS_FAILURE;
	TListenerArray::iterator Iter(m_cListeners.begin());
	TListenerArray::const_iterator const IterEnd(m_cListeners.end());

	for (; Iter != IterEnd; ++Iter)
	{
		if (Iter->OnEvent == func && Iter->pRequestOwner == pRequestOwner)
		{
			if (Iter != IterEnd-1)
			{
				(*Iter) = m_cListeners.back();
			}

			m_cListeners.pop_back();
			eResult = eARS_SUCCESS;

			break;
		}
	}

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	if (eResult == eARS_FAILURE)
	{
		g_AudioLogger.Log(eALT_WARNING, "Failed to remove a request listener!");
	}
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

	return eResult;
}

//////////////////////////////////////////////////////////////////////////
void CAudioEventListenerManager::NotifyListener(SAudioRequestInfo const* const pResultInfo)
{
	FUNCTION_PROFILER_ALWAYS(gEnv->pSystem, PROFILE_AUDIO);

	TListenerArray::const_iterator Iter(m_cListeners.begin());
	TListenerArray::const_iterator const IterEnd(m_cListeners.end());

	for (; Iter != IterEnd; ++Iter)
	{
		if (Iter->pRequestOwner == pResultInfo->pOwner)
		{
			Iter->OnEvent(pResultInfo);
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////
CATLXMLProcessor::CATLXMLProcessor(
	TATLTriggerLookup& rTriggers,
	TATLRtpcLookup& rRtpcs,
	TATLSwitchLookup& rSwitches,
	TATLEnvironmentLookup& rEnvironments,
	TATLPreloadRequestLookup&	rPreloadRequests,
	CFileCacheManager& rFileCacheMgr)
	: m_rTriggers(rTriggers)
	, m_rRtpcs(rRtpcs)
	, m_rSwitches(rSwitches)
	, m_rEnvironments(rEnvironments)
	,	m_rPreloadRequests(rPreloadRequests)
	, m_nTriggerImplIDCounter(AUDIO_TRIGGER_IMPL_ID_NUM_RESERVED)
	, m_rFileCacheMgr(rFileCacheMgr)
	, m_pImpl(NPTR)
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	, m_pDebugNameStore(NPTR)
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
{
}

///////////////////////////////////////////////////////////////////////////
CATLXMLProcessor::~CATLXMLProcessor()
{
}

///////////////////////////////////////////////////////////////////////////
void CATLXMLProcessor::Init(IAudioSystemImplementation* const pImpl)
{
	m_pImpl = pImpl;
}

///////////////////////////////////////////////////////////////////////////
void CATLXMLProcessor::Release()
{
	m_pImpl = NPTR;
}

///////////////////////////////////////////////////////////////////////////
void CATLXMLProcessor::ParseControlsData(char const* const sFolderPath, EATLDataScope const eDataScope)
{
	CryFixedStringT<MAX_AUDIO_FILE_PATH_LENGTH> sRootFolderPath(sFolderPath);

	if (m_pImpl != NPTR)
	{
		sRootFolderPath.TrimRight("/\\");
		CryFixedStringT<MAX_AUDIO_FILE_PATH_LENGTH + MAX_AUDIO_FILE_NAME_LENGTH> sSearch(sRootFolderPath + "/*.xml");
		_finddata_t fd;
		intptr_t handle = gEnv->pCryPak->FindFirst(sSearch.c_str(), &fd);

		if (handle != -1)
		{
			CryFixedStringT<MAX_AUDIO_FILE_PATH_LENGTH + MAX_AUDIO_FILE_NAME_LENGTH> sFileName;

			do
			{
				sFileName = sRootFolderPath.c_str();
				sFileName += "/" ;
				sFileName += fd.name;

				XmlNodeRef const pATLConfigRoot(GetISystem()->LoadXmlFromFile(sFileName));

				if (pATLConfigRoot)
				{
					if (_stricmp(pATLConfigRoot->getTag(), SATLXMLTags::sRootNodeTag) == 0)
					{
						size_t const nATLConfigChildrenCount = static_cast<size_t>(pATLConfigRoot->getChildCount());

						for (size_t i = 0; i < nATLConfigChildrenCount; ++i)
						{
							XmlNodeRef const pAudioConfigNode(pATLConfigRoot->getChild(i));

							if (pAudioConfigNode)
							{
								char const* const sAudioConfigNodeTag = pAudioConfigNode->getTag();

								if (_stricmp(sAudioConfigNodeTag, SATLXMLTags::sTriggersNodeTag) == 0)
								{
									ParseAudioTriggers(pAudioConfigNode, eDataScope);
								}
								else if (_stricmp(sAudioConfigNodeTag, SATLXMLTags::sRtpcsNodeTag) == 0)
								{
									ParseAudioRtpcs(pAudioConfigNode, eDataScope);
								}
								else if (_stricmp(sAudioConfigNodeTag, SATLXMLTags::sSwitchesNodeTag) == 0)
								{
									ParseAudioSwitches(pAudioConfigNode, eDataScope);
								}
								else if (_stricmp(sAudioConfigNodeTag, SATLXMLTags::sEnvironmentsNodeTag) == 0)
								{
									ParseAudioEnvironments(pAudioConfigNode, eDataScope);
								}
								else if (_stricmp(sAudioConfigNodeTag, SATLXMLTags::sPreloadsNodeTag) == 0)
								{
									// This tag is valid but ignored here.
								}
								else
								{
									g_AudioLogger.Log(eALT_WARNING, "Unknown AudioConfig node: %s", sAudioConfigNodeTag);
									assert(false);
								}
							}
						}
					}
				}
			} while (gEnv->pCryPak->FindNext(handle, &fd) >= 0);

			gEnv->pCryPak->FindClose(handle);
		}
	}
}

///////////////////////////////////////////////////////////////////////////
void CATLXMLProcessor::ParsePreloadsData(char const* const sFolderPath, EATLDataScope const eDataScope)
{
	CryFixedStringT<MAX_AUDIO_FILE_PATH_LENGTH> sRootFolderPath(sFolderPath);

	if (m_pImpl != NPTR)
	{
		sRootFolderPath.TrimRight("/\\");
		CryFixedStringT<MAX_AUDIO_FILE_PATH_LENGTH + MAX_AUDIO_FILE_NAME_LENGTH> sSearch(sRootFolderPath + "/*.xml");
		_finddata_t fd;
		intptr_t handle = gEnv->pCryPak->FindFirst(sSearch.c_str(), &fd);

		if (handle != -1)
		{
			CryFixedStringT<MAX_AUDIO_FILE_PATH_LENGTH + MAX_AUDIO_FILE_NAME_LENGTH> sFileName;

			do
			{
				sFileName = sRootFolderPath.c_str();
				sFileName += "/" ;
				sFileName += fd.name;

				XmlNodeRef const pATLConfigRoot(GetISystem()->LoadXmlFromFile(sFileName));

				if (pATLConfigRoot)
				{
					if (_stricmp(pATLConfigRoot->getTag(), SATLXMLTags::sRootNodeTag) == 0)
					{
						size_t const nATLConfigChildrenCount = static_cast<size_t>(pATLConfigRoot->getChildCount());

						for (size_t i = 0; i < nATLConfigChildrenCount; ++i)
						{
							XmlNodeRef const pAudioConfigNode(pATLConfigRoot->getChild(i));

							if (pAudioConfigNode)
							{
								char const* const sAudioConfigNodeTag = pAudioConfigNode->getTag();

								if (_stricmp(sAudioConfigNodeTag, SATLXMLTags::sPreloadsNodeTag) == 0)
								{
									size_t const nLastSlashIndex = sRootFolderPath.rfind('/');

									if (sRootFolderPath.npos != nLastSlashIndex)
									{
										CryFixedStringT<MAX_AUDIO_FILE_PATH_LENGTH> const sFolderName(sRootFolderPath.substr(nLastSlashIndex + 1, sRootFolderPath.size()));
										ParseAudioPreloads(pAudioConfigNode, eDataScope, sFolderName.c_str());
									}
									else
									{
										ParseAudioPreloads(pAudioConfigNode, eDataScope, NPTR);
									}
								}
								else if (_stricmp(sAudioConfigNodeTag, SATLXMLTags::sTriggersNodeTag) == 0 ||
									_stricmp(sAudioConfigNodeTag, SATLXMLTags::sRtpcsNodeTag) == 0 ||
									_stricmp(sAudioConfigNodeTag, SATLXMLTags::sSwitchesNodeTag) == 0 ||
									_stricmp(sAudioConfigNodeTag, SATLXMLTags::sEnvironmentsNodeTag) == 0
									)
								{
									// These tags are valid but ignored here.
								}
								else
								{
									g_AudioLogger.Log(eALT_WARNING, "Unknown AudioConfig node: %s", sAudioConfigNodeTag);
									assert(false);
								}
							}
						}
					}
				}
			} while (gEnv->pCryPak->FindNext(handle, &fd) >= 0);

			gEnv->pCryPak->FindClose(handle);
		}
	}
}

///////////////////////////////////////////////////////////////////////////
void CATLXMLProcessor::ClearControlsData(EATLDataScope const eDataScope)
{
	if (m_pImpl != NPTR)
	{
		TATLTriggerLookup::iterator iTriggerRemover = m_rTriggers.begin();
		TATLTriggerLookup::const_iterator const iTriggerEnd = m_rTriggers.end();

		while (iTriggerRemover != iTriggerEnd)
		{
			CATLTrigger const* const pTrigger = iTriggerRemover->second;

			if ((pTrigger->GetDataScope() == eDataScope) || eDataScope == eADS_ALL)
			{
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
				m_pDebugNameStore->RemoveAudioTrigger(pTrigger->GetID());
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

				DeleteAudioTrigger(pTrigger);
				m_rTriggers.erase(iTriggerRemover++);
			}
			else
			{
				++iTriggerRemover;
			}
		}

		TATLRtpcLookup::iterator iRtpcRemover = m_rRtpcs.begin();
		TATLRtpcLookup::const_iterator const iRtpcEnd = m_rRtpcs.end();

		while (iRtpcRemover != iRtpcEnd)
		{
			CATLRtpc const* const pRtpc = iRtpcRemover->second;

			if ((pRtpc->GetDataScope() == eDataScope) || eDataScope == eADS_ALL)
			{
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
				m_pDebugNameStore->RemoveAudioRtpc(pRtpc->GetID());
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

				DeleteAudioRtpc(pRtpc);
				m_rRtpcs.erase(iRtpcRemover++);
			}
			else
			{
				++iRtpcRemover;
			}
		}

		TATLSwitchLookup::iterator iSwitchRemover = m_rSwitches.begin();
		TATLSwitchLookup::const_iterator const iSwitchEnd = m_rSwitches.end();

		while (iSwitchRemover != iSwitchEnd)
		{
			CATLSwitch const* const pSwitch = iSwitchRemover->second;

			if ((pSwitch->GetDataScope() == eDataScope) || eDataScope == eADS_ALL)
			{
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
				m_pDebugNameStore->RemoveAudioSwitch(pSwitch->GetID());
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

				DeleteAudioSwitch(pSwitch);
				m_rSwitches.erase(iSwitchRemover++);
			}
			else
			{
				++iSwitchRemover;
			}
		}

		TATLEnvironmentLookup::iterator iRemover = m_rEnvironments.begin();
		TATLEnvironmentLookup::const_iterator const iEnd = m_rEnvironments.end();

		while (iRemover != iEnd)
		{
			CATLAudioEnvironment const* const pEnvironment = iRemover->second;

			if ((pEnvironment->GetDataScope() == eDataScope) || eDataScope == eADS_ALL)
			{
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
				m_pDebugNameStore->RemoveAudioEnvironment(pEnvironment->GetID());
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

				DeleteAudioEnvironment(pEnvironment);
				m_rEnvironments.erase(iRemover++);
			}
			else
			{
				++iRemover;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
void CATLXMLProcessor::ParseAudioPreloads(XmlNodeRef const pPreloadDataRoot, EATLDataScope const eDataScope, char const* const sFolderName)
{
	LOADING_TIME_PROFILE_SECTION;

	size_t const nPreloadRequestCount = static_cast<size_t>(pPreloadDataRoot->getChildCount());

	for (size_t i = 0; i < nPreloadRequestCount; ++i)
	{
		XmlNodeRef const pPreloadRequestNode(pPreloadDataRoot->getChild(i));

		if (pPreloadRequestNode && _stricmp(pPreloadRequestNode->getTag(), SATLXMLTags::sATLPreloadRequestTag) == 0)
		{
			TAudioPreloadRequestID nAudioPreloadRequestID = SATLInternalControlIDs::nGlobalPreloadRequestID;
			char const* sAudioPreloadRequestName = "global_atl_preloads";
			bool const bAutoLoad = (_stricmp(pPreloadRequestNode->getAttr(SATLXMLTags::sATLTypeAttribute), SATLXMLTags::sATLDataLoadType) == 0);

			if (!bAutoLoad)
			{
				sAudioPreloadRequestName = pPreloadRequestNode->getAttr(SATLXMLTags::sATLNameAttribute);
				nAudioPreloadRequestID = static_cast<TAudioPreloadRequestID const>(AudioStringToID(sAudioPreloadRequestName));
			}
			else if (eDataScope == eADS_LEVEL_SPECIFIC)
			{
				sAudioPreloadRequestName = sFolderName;
				nAudioPreloadRequestID = static_cast<TAudioPreloadRequestID const>(AudioStringToID(sAudioPreloadRequestName));
			}

			if (nAudioPreloadRequestID != INVALID_AUDIO_PRELOAD_REQUEST_ID)
			{
				size_t const nPreloadRequestChidrenCount = static_cast<size_t>(pPreloadRequestNode->getChildCount());

				if (nPreloadRequestChidrenCount > 1)
				{
					// We need to have at least two children: ATLPlatforms and at least one ATLConfigGroup
					XmlNodeRef const pPlatformsNode(pPreloadRequestNode->getChild(0));
					char const* sATLConfigGroupName = NPTR;

					if (pPlatformsNode && _stricmp(pPlatformsNode->getTag(), SATLXMLTags::sATLPlatformsTag) == 0)
					{
						size_t const nPlatformCount = pPlatformsNode->getChildCount();

						for (size_t j = 0; j < nPlatformCount; ++j)
						{
							XmlNodeRef const pPlatformNode(pPlatformsNode->getChild(j));

							if (pPlatformNode && _stricmp(pPlatformNode->getAttr(SATLXMLTags::sATLNameAttribute), SATLXMLTags::sPlatform) == 0)
							{
								sATLConfigGroupName = pPlatformNode->getAttr(SATLXMLTags::sATLConfigGroupAttribute);
								break;
							}
						}
					}

					if (sATLConfigGroupName != NPTR)
					{
						for (size_t j = 1; j < nPreloadRequestChidrenCount; ++j)
						{
							XmlNodeRef const pConfigGroupNode(pPreloadRequestNode->getChild(j));

							if (pConfigGroupNode && _stricmp(pConfigGroupNode->getAttr(SATLXMLTags::sATLNameAttribute), sATLConfigGroupName) == 0)
							{
								// Found the config group corresponding to the specified platform.
								size_t const nFileCount = static_cast<size_t>(pConfigGroupNode->getChildCount());

								CATLPreloadRequest::TFileEntryIDs cFileEntryIDs;
								cFileEntryIDs.reserve(nFileCount);

								for (size_t k = 0; k < nFileCount; ++k)
								{
									TAudioFileEntryID const nID = m_rFileCacheMgr.TryAddFileCacheEntry(pConfigGroupNode->getChild(k), eDataScope, bAutoLoad);

									if (nID != INVALID_AUDIO_FILE_ENTRY_ID)
									{
										cFileEntryIDs.push_back(nID);
									}
									else
									{
										g_AudioLogger.Log(eALT_WARNING, "Preload request \"%s\" could not create file entry from tag \"%s\"!", sAudioPreloadRequestName, pConfigGroupNode->getChild(k)->getTag());
									}
								}

								CATLPreloadRequest* pPreloadRequest = stl::find_in_map(m_rPreloadRequests, nAudioPreloadRequestID, NPTR);

								if (pPreloadRequest == NPTR)
								{
									POOL_NEW(CATLPreloadRequest, pPreloadRequest)(nAudioPreloadRequestID, eDataScope, bAutoLoad, cFileEntryIDs);

									if (pPreloadRequest != NPTR)
									{
										m_rPreloadRequests[nAudioPreloadRequestID] = pPreloadRequest;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
										m_pDebugNameStore->AddAudioPreloadRequest(nAudioPreloadRequestID, sAudioPreloadRequestName);
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
									}
									else
									{
										CryFatalError("<Audio>: Failed to allocate CATLPreloadRequest");
									}
								}
								else
								{
									// Add to existing preload request.
									pPreloadRequest->m_cFileEntryIDs.insert(pPreloadRequest->m_cFileEntryIDs.end(), cFileEntryIDs.begin(), cFileEntryIDs.end());
								}

								break;// no need to look through the rest of the ConfigGroups
							}
						}
					}
				}
			}
			else
			{
				g_AudioLogger.Log(eALT_ERROR, "Preload request \"%s\" already exists! Skipping this entry!", sAudioPreloadRequestName);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
void CATLXMLProcessor::ClearPreloadsData(EATLDataScope const eDataScope)
{
	if (m_pImpl != NPTR)
	{
		TATLPreloadRequestLookup::iterator iRemover = m_rPreloadRequests.begin();
		TATLPreloadRequestLookup::const_iterator const iEnd = m_rPreloadRequests.end();

		while (iRemover != iEnd)
		{
			CATLPreloadRequest const* const pRequest = iRemover->second;

			if ((pRequest->GetDataScope() == eDataScope) || eDataScope == eADS_ALL)
			{
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
				m_pDebugNameStore->RemoveAudioPreloadRequest(pRequest->GetID());
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

				DeleteAudioPreloadRequest(pRequest);
				m_rPreloadRequests.erase(iRemover++);
			}
			else
			{
				++iRemover;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
void CATLXMLProcessor::ParseAudioEnvironments(XmlNodeRef const pAudioEnvironmentRoot, EATLDataScope const eDataScope)
{
	size_t const nAudioEnvironmentCount = static_cast<size_t>(pAudioEnvironmentRoot->getChildCount());

	for (size_t i = 0; i < nAudioEnvironmentCount; ++i)
	{
		XmlNodeRef const pAudioEnvironmentNode(pAudioEnvironmentRoot->getChild(i));

		if (pAudioEnvironmentNode && _stricmp(pAudioEnvironmentNode->getTag(), SATLXMLTags::sATLEnvironmentTag) == 0)
		{
			char const* const sATLEnvironmentName = pAudioEnvironmentNode->getAttr(SATLXMLTags::sATLNameAttribute);
			TAudioEnvironmentID const nATLEnvironmentID = static_cast<TAudioEnvironmentID const>(AudioStringToID(sATLEnvironmentName));

			if ((nATLEnvironmentID != INVALID_AUDIO_CONTROL_ID) && (stl::find_in_map(m_rEnvironments, nATLEnvironmentID, NPTR) == NPTR))
			{
				//there is no entry yet with this ID in the container
				size_t const nAudioEnvironmentChildrenCount = static_cast<size_t>(pAudioEnvironmentNode->getChildCount());

				CATLAudioEnvironment::TImplPtrVec cImplPtrs;
				cImplPtrs.reserve(nAudioEnvironmentChildrenCount);

				for (size_t j = 0; j < nAudioEnvironmentChildrenCount; ++j)
				{
					XmlNodeRef const pEnvironmentImplNode(pAudioEnvironmentNode->getChild(j));

					if (pEnvironmentImplNode)
					{
						IATLEnvironmentImplData const* pEnvirnomentImplData = NPTR;
						EATLSubsystem eReceiver = eAS_NONE;

						if (_stricmp(pEnvironmentImplNode->getTag(), SATLXMLTags::sATLEnvironmentRequestTag) == 0)
						{
							pEnvirnomentImplData = NewAudioEnvironmentImplDataInternal(pEnvironmentImplNode);
							eReceiver = eAS_ATL_INTERNAL;
						}
						else
						{
							pEnvirnomentImplData = m_pImpl->NewAudioEnvironmentImplData(pEnvironmentImplNode);
							eReceiver = eAS_AUDIO_SYSTEM_IMPLEMENTATION;
						}

						if (pEnvirnomentImplData != NPTR)
						{
							POOL_NEW_CREATE(CATLEnvironmentImpl, pEnvirnomentImpl)(eReceiver, pEnvirnomentImplData);
							cImplPtrs.push_back(pEnvirnomentImpl);
						}
						else
						{
							g_AudioLogger.Log(eALT_WARNING,"Could not parse an Environment Implementation with XML tag %s", pEnvironmentImplNode->getTag());
						}
					}
				}

				if (!cImplPtrs.empty())
				{
					POOL_NEW_CREATE(CATLAudioEnvironment, pNewEnvironment)(nATLEnvironmentID, eDataScope, cImplPtrs);

					if (pNewEnvironment != NPTR)
					{
						m_rEnvironments[nATLEnvironmentID] = pNewEnvironment;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
						m_pDebugNameStore->AddAudioEnvironment(nATLEnvironmentID, sATLEnvironmentName);
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
					}
				}
			}
			else
			{
				g_AudioLogger.Log(eALT_ERROR, "AudioEnvironment \"%s\" already exists!", sATLEnvironmentName);
				assert(false);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
void CATLXMLProcessor::ParseAudioTriggers(XmlNodeRef const pXMLTriggerRoot, EATLDataScope const eDataScope)
{
	size_t const nAudioTriggersChildrenCount = static_cast<size_t>(pXMLTriggerRoot->getChildCount());

	for (size_t i = 0; i < nAudioTriggersChildrenCount; ++i)
	{
		XmlNodeRef const pAudioTriggerNode(pXMLTriggerRoot->getChild(i));

		if (pAudioTriggerNode && _stricmp(pAudioTriggerNode->getTag(), SATLXMLTags::sATLTriggerTag) == 0)
		{
			char const* const sATLTriggerName = pAudioTriggerNode->getAttr(SATLXMLTags::sATLNameAttribute);
			TAudioControlID const nATLTriggerID = static_cast<TAudioControlID const>(AudioStringToID(sATLTriggerName));

			if ((nATLTriggerID != INVALID_AUDIO_CONTROL_ID) && (stl::find_in_map(m_rTriggers, nATLTriggerID, NPTR) == NPTR))
			{
				//there is no entry yet with this ID in the container
				size_t const nAudioTriggerChildrenCount = static_cast<size_t>(pAudioTriggerNode->getChildCount());

				CATLTrigger::TImplPtrVec cImplPtrs;
				cImplPtrs.reserve(nAudioTriggerChildrenCount);

				for (size_t m = 0; m < nAudioTriggerChildrenCount; ++m)
				{
					XmlNodeRef const pTriggerImplNode(pAudioTriggerNode->getChild(m));

					if (pTriggerImplNode)
					{
						IATLTriggerImplData const* pTriggerImplData = NPTR;
						EATLSubsystem eReceiver = eAS_NONE;

						if (_stricmp(pTriggerImplNode->getTag(), SATLXMLTags::sATLTriggerRequestTag) == 0)
						{
							pTriggerImplData = NewAudioTriggerImplDataInternal(pTriggerImplNode);
							eReceiver = eAS_ATL_INTERNAL;
						}
						else
						{
							pTriggerImplData = m_pImpl->NewAudioTriggerImplData(pTriggerImplNode);
							eReceiver = eAS_AUDIO_SYSTEM_IMPLEMENTATION;
						}

						if (pTriggerImplData != NPTR)
						{
							POOL_NEW_CREATE(CATLTriggerImpl, pTriggerImpl)(++m_nTriggerImplIDCounter, nATLTriggerID, eReceiver, pTriggerImplData);
							cImplPtrs.push_back(pTriggerImpl);
						}
						else
						{
							g_AudioLogger.Log(eALT_WARNING,"Could not parse a Trigger Implementation with XML tag %s", pTriggerImplNode->getTag());
						}
					}
				}

				POOL_NEW_CREATE(CATLTrigger, pNewTrigger)(nATLTriggerID, eDataScope, cImplPtrs);

				if (pNewTrigger != NPTR)
				{
					m_rTriggers[nATLTriggerID] = pNewTrigger;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
					m_pDebugNameStore->AddAudioTrigger(nATLTriggerID, sATLTriggerName);
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
				}
			}
			else
			{
				g_AudioLogger.Log(eALT_ERROR, "trigger \"%s\" already exists!", sATLTriggerName);
				assert(false);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
void CATLXMLProcessor::ParseAudioSwitches(XmlNodeRef const pXMLSwitchRoot, EATLDataScope const eDataScope)
{
	typedef std::vector<CATLSwitchImpl const*, STLSoundAllocator<CATLSwitchImpl const*> > TSwitchImplVec;
	typedef std::map<
		TAudioSwitchStateID, 
		CATLSwitchState::TImplPtrVec, 
		std::less<TAudioSwitchStateID>, 
		STLSoundAllocator< std::pair<TAudioSwitchStateID, CATLSwitchState::TImplPtrVec> > > TSwitchStateImplMap;

	size_t const nAudioSwitchesChildrenCount = static_cast<size_t>(pXMLSwitchRoot->getChildCount());

	for (size_t i = 0; i < nAudioSwitchesChildrenCount; ++i)
	{
		XmlNodeRef const pATLSwitchNode(pXMLSwitchRoot->getChild(i));

		if (pATLSwitchNode && _stricmp(pATLSwitchNode->getTag(), SATLXMLTags::sATLSwitchTag) == 0)
		{
			char const* const sATLSwitchName = pATLSwitchNode->getAttr(SATLXMLTags::sATLNameAttribute);
			TAudioControlID const nATLSwitchID = static_cast<TAudioControlID const>(AudioStringToID(sATLSwitchName));

			if ((nATLSwitchID != INVALID_AUDIO_CONTROL_ID) && (stl::find_in_map(m_rSwitches, nATLSwitchID, NPTR) == NPTR))
			{
				POOL_NEW_CREATE(CATLSwitch, pNewSwitch)(nATLSwitchID, eDataScope);
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
				m_pDebugNameStore->AddAudioSwitch(nATLSwitchID, sATLSwitchName);
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

				size_t const nATLSwitchStatesCount = static_cast<size_t>(pATLSwitchNode->getChildCount());

				for (size_t j = 0; j < nATLSwitchStatesCount; ++j)
				{
					XmlNodeRef const pATLSwitchStateNode(pATLSwitchNode->getChild(j));
					
					if (pATLSwitchStateNode && _stricmp(pATLSwitchStateNode->getTag(), SATLXMLTags::sATLSwitchStateTag) == 0)
					{
						char const* const sATLSwitchStateName = pATLSwitchStateNode->getAttr(SATLXMLTags::sATLNameAttribute);
						TAudioSwitchStateID const nATLSwitchStateID = static_cast<TAudioSwitchStateID const>(AudioStringToID(sATLSwitchStateName));
						
						if (nATLSwitchStateID != INVALID_AUDIO_SWITCH_STATE_ID)
						{
							size_t const nSwitchStateImplCount = static_cast<size_t>(pATLSwitchStateNode->getChildCount());

							CATLSwitchState::TImplPtrVec cSwitchStateImplVec;
							cSwitchStateImplVec.reserve(nSwitchStateImplCount);

							for (size_t k = 0; k < nSwitchStateImplCount; ++k)
							{
								XmlNodeRef const pStateImplNode(pATLSwitchStateNode->getChild(k));
								if (pStateImplNode)
								{
									char const* const sStateImplNodeTag = pStateImplNode->getTag();
									IATLSwitchStateImplData const* pNewSwitchStateImplData = NPTR;
									EATLSubsystem eReceiver = eAS_NONE;

									if (_stricmp(sStateImplNodeTag, SATLXMLTags::sATLSwitchRequestTag) == 0)
									{
										pNewSwitchStateImplData = NewAudioSwitchStateImplDataInternal(pStateImplNode);
										eReceiver = eAS_ATL_INTERNAL;
									}
									else 
									{
										pNewSwitchStateImplData = m_pImpl->NewAudioSwitchStateImplData(pStateImplNode);
										eReceiver = eAS_AUDIO_SYSTEM_IMPLEMENTATION;
									}

									if (pNewSwitchStateImplData != NPTR)
									{
										POOL_NEW_CREATE(CATLSwitchStateImpl, pSwitchStateImpl)(eReceiver, pNewSwitchStateImplData);
										cSwitchStateImplVec.push_back(pSwitchStateImpl);
									}
								}
							}

						POOL_NEW_CREATE(CATLSwitchState, pNewState)(nATLSwitchID, nATLSwitchStateID, cSwitchStateImplVec);
						pNewSwitch->cStates[nATLSwitchStateID] = pNewState;
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
						m_pDebugNameStore->AddAudioSwitchState(nATLSwitchID, nATLSwitchStateID, sATLSwitchStateName);
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
						}
					}
				}
				m_rSwitches[nATLSwitchID] = pNewSwitch;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
void CATLXMLProcessor::ParseAudioRtpcs(XmlNodeRef const pXMLRtpcRoot, EATLDataScope const eDataScope)
{
	size_t const nAudioRtpcChildrenCount = static_cast<size_t>(pXMLRtpcRoot->getChildCount());

	for (size_t i = 0; i < nAudioRtpcChildrenCount; ++i)
	{
		XmlNodeRef const pAudioRtpcNode(pXMLRtpcRoot->getChild(i));

		if (pAudioRtpcNode && _stricmp(pAudioRtpcNode->getTag(), SATLXMLTags::sATLRtpcTag) == 0)
		{
			char const* const sATLRtpcName = pAudioRtpcNode->getAttr(SATLXMLTags::sATLNameAttribute);
			TAudioControlID const nATLRtpcID = static_cast<TAudioControlID const>(AudioStringToID(sATLRtpcName));

			if ((nATLRtpcID != INVALID_AUDIO_CONTROL_ID) && (stl::find_in_map(m_rRtpcs, nATLRtpcID, NPTR) == NPTR))
			{
				size_t const nRtpcNodeChildrenCount = static_cast<size_t>(pAudioRtpcNode->getChildCount());
				CATLRtpc::TImplPtrVec cImplPtrs;
				cImplPtrs.reserve(nRtpcNodeChildrenCount);

				for (size_t j = 0; j < nRtpcNodeChildrenCount; ++j)
				{
					XmlNodeRef const pRtpcImplNode(pAudioRtpcNode->getChild(j));

					if (pRtpcImplNode)
					{
						char const* const sRtpcImplNodeTag = pRtpcImplNode->getTag();
						IATLRtpcImplData const* pNewRtpcImplData = NPTR;
						EATLSubsystem eReceiver = eAS_NONE;

						if (_stricmp(sRtpcImplNodeTag, SATLXMLTags::sATLRtpcRequestTag) == 0)
						{
							pNewRtpcImplData = NewAudioRtpcImplDataInternal(pRtpcImplNode);
							eReceiver = eAS_ATL_INTERNAL;
						}
						else 
						{
							pNewRtpcImplData = m_pImpl->NewAudioRtpcImplData(pRtpcImplNode);
							eReceiver = eAS_AUDIO_SYSTEM_IMPLEMENTATION;
						}

						if (pNewRtpcImplData != NPTR)
						{
							POOL_NEW_CREATE(CATLRtpcImpl, pRtpcImpl)(eReceiver, pNewRtpcImplData);
							cImplPtrs.push_back(pRtpcImpl);
						}
					}
				}

				POOL_NEW_CREATE(CATLRtpc, pNewRtpc)(nATLRtpcID, eDataScope, cImplPtrs);

				if (pNewRtpc != NPTR)
				{
					m_rRtpcs[nATLRtpcID] = pNewRtpc;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
					m_pDebugNameStore->AddAudioRtpc(nATLRtpcID, sATLRtpcName);
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
IATLTriggerImplData const* CATLXMLProcessor::NewAudioTriggerImplDataInternal(XmlNodeRef const pXMLTriggerRoot)
{
	//TODO: implement
	return NPTR;
}

///////////////////////////////////////////////////////////////////////////
IATLRtpcImplData const* CATLXMLProcessor::NewAudioRtpcImplDataInternal(XmlNodeRef const pXMLRtpcRoot)
{
	//TODO: implement
	return NPTR;
}

///////////////////////////////////////////////////////////////////////////
IATLSwitchStateImplData const* CATLXMLProcessor::NewAudioSwitchStateImplDataInternal(XmlNodeRef const pXMLSwitchRoot)
{
	SATLSwitchStateImplData_internal* pSwitchStateImpl = NPTR;

	char const* const sInternalSwitchNodeName = pXMLSwitchRoot->getAttr(SATLXMLTags::sATLNameAttribute);

	if ((sInternalSwitchNodeName != NPTR) && (sInternalSwitchNodeName[0] != 0) && (pXMLSwitchRoot->getChildCount() == 1))
	{
		XmlNodeRef const pValueNode(pXMLSwitchRoot->getChild(0));

		if (pValueNode && _stricmp(pValueNode->getTag(), SATLXMLTags::sATLValueTag) == 0)
		{
			char const* sInternalSwitchStateName	= pValueNode->getAttr(SATLXMLTags::sATLNameAttribute);

			if ((sInternalSwitchStateName != NPTR) && (sInternalSwitchStateName[0] != 0))
			{
				TAudioControlID const nATLInternalSwitchID = static_cast<TAudioControlID>(AudioStringToID(sInternalSwitchNodeName));
				TAudioSwitchStateID const nATLInternalStateID = static_cast<TAudioSwitchStateID>(AudioStringToID(sInternalSwitchStateName));
				POOL_NEW(SATLSwitchStateImplData_internal, pSwitchStateImpl)(nATLInternalSwitchID, nATLInternalStateID);
			}
		}
	}
	else
	{
		g_AudioLogger.Log(
			eALT_WARNING,
			"An ATLSwitchRequest %s inside ATLSwitchState needs to have exactly one ATLValue.",
			sInternalSwitchNodeName);
	}

	return pSwitchStateImpl;
}

///////////////////////////////////////////////////////////////////////////
IATLEnvironmentImplData const* CATLXMLProcessor::NewAudioEnvironmentImplDataInternal(XmlNodeRef const pXMLEnvironmentRoot)
{
	// TODO: implement
	return NPTR;
}

///////////////////////////////////////////////////////////////////////////
void CATLXMLProcessor::DeleteAudioTrigger(CATLTrigger const* const pOldTrigger)
{
	if (pOldTrigger != NPTR)
	{
		CATLTrigger::TImplPtrVec::const_iterator iTriggerImpl = pOldTrigger->m_cImplPtrs.begin();
		CATLTrigger::TImplPtrVec::const_iterator const iTriggerImplEnd = pOldTrigger->m_cImplPtrs.end();
		for (; iTriggerImpl != iTriggerImplEnd; ++iTriggerImpl)
		{
			CATLTriggerImpl const* const pTriggerImpl = *iTriggerImpl;

			if (pTriggerImpl->GetReceiver() == eAS_ATL_INTERNAL)
			{
				POOL_FREE_CONST(pTriggerImpl->m_pImplData);
			}
			else
			{
				m_pImpl->DeleteAudioTriggerImplData(pTriggerImpl->m_pImplData);
			}

			POOL_FREE_CONST(pTriggerImpl);
		}

		POOL_FREE_CONST(pOldTrigger);
	}
}

///////////////////////////////////////////////////////////////////////////
void CATLXMLProcessor::DeleteAudioRtpc(CATLRtpc const* const pOldRtpc)
{
	if (pOldRtpc != NPTR)
	{
		CATLRtpc::TImplPtrVec::const_iterator iRtpcImpl = pOldRtpc->m_cImplPtrs.begin();
		CATLRtpc::TImplPtrVec::const_iterator const iRtpcImplEnd = pOldRtpc->m_cImplPtrs.end();
		for (; iRtpcImpl != iRtpcImplEnd; ++iRtpcImpl)
		{
			CATLRtpcImpl const* const pRtpcImpl = *iRtpcImpl;

			if (pRtpcImpl->GetReceiver() == eAS_ATL_INTERNAL)
			{
				POOL_FREE_CONST(pRtpcImpl->m_pImplData);
			}
			else
			{
				m_pImpl->DeleteAudioRtpcImplData(pRtpcImpl->m_pImplData);
			}

			POOL_FREE_CONST(pRtpcImpl);
		}

		POOL_FREE_CONST(pOldRtpc);
	}
}

///////////////////////////////////////////////////////////////////////////
void CATLXMLProcessor::DeleteAudioSwitch(CATLSwitch const* const pOldSwitch)
{
	if (pOldSwitch != NPTR)
	{
		CATLSwitch::TStateMap::const_iterator iStatePair = pOldSwitch->cStates.begin();
		CATLSwitch::TStateMap::const_iterator const iStatePairEnd = pOldSwitch->cStates.end();

		for (; iStatePair != iStatePairEnd; ++iStatePair)
		{
			CATLSwitchState const* const pSwitchState = iStatePair->second;
			if (pSwitchState != NPTR)
			{
				CATLSwitchState::TImplPtrVec::const_iterator iStateImpl = pSwitchState->m_cImplPtrs.begin();
				CATLSwitchState::TImplPtrVec::const_iterator iStateImplEnd = pSwitchState->m_cImplPtrs.end();

				for (; iStateImpl != iStateImplEnd; ++iStateImpl)
				{
					CATLSwitchStateImpl const* const pStateImpl = *iStateImpl;

					if (pStateImpl->GetReceiver() == eAS_ATL_INTERNAL)
					{
						POOL_FREE_CONST(pStateImpl->m_pImplData);
					}
					else
					{
						m_pImpl->DeleteAudioSwitchStateImplData(pStateImpl->m_pImplData);
					}

					POOL_FREE_CONST(pStateImpl);
				}

				POOL_FREE_CONST(pSwitchState);
			}
		}

		POOL_FREE_CONST(pOldSwitch);
	}
}

///////////////////////////////////////////////////////////////////////////
void CATLXMLProcessor::DeleteAudioPreloadRequest(CATLPreloadRequest const* const pOldPreloadRequest)
{
	if (pOldPreloadRequest != NPTR)
	{
		EATLDataScope eScope = pOldPreloadRequest->GetDataScope();
		CATLPreloadRequest::TFileEntryIDs::const_iterator iFileID = pOldPreloadRequest->m_cFileEntryIDs.begin();
		CATLPreloadRequest::TFileEntryIDs::const_iterator const iFileIDEnd = pOldPreloadRequest->m_cFileEntryIDs.end();

		for (; iFileID != iFileIDEnd; ++iFileID)
		{
			m_rFileCacheMgr.TryRemoveFileCacheEntry(*iFileID, eScope);
		}

		POOL_FREE_CONST(pOldPreloadRequest);
	}
}

///////////////////////////////////////////////////////////////////////////
void CATLXMLProcessor::DeleteAudioEnvironment(CATLAudioEnvironment const* const pOldEnvironment)
{
	if (pOldEnvironment != NPTR)
	{
		CATLAudioEnvironment::TImplPtrVec::const_iterator iEnvironmentImpl = pOldEnvironment->m_cImplPtrs.begin();
		CATLAudioEnvironment::TImplPtrVec::const_iterator const iEnvironmentImplEnd = pOldEnvironment->m_cImplPtrs.end();
		for (; iEnvironmentImpl != iEnvironmentImplEnd; ++iEnvironmentImpl)
		{
			CATLEnvironmentImpl const* const pEnvImpl = *iEnvironmentImpl;

			if (pEnvImpl->GetReceiver() == eAS_ATL_INTERNAL)
			{
				POOL_FREE_CONST(pEnvImpl->m_pImplData);
			}
			else
			{
				m_pImpl->DeleteAudioEnvironmentImplData(pEnvImpl->m_pImplData);
			}

			POOL_FREE_CONST(pEnvImpl);
		}

		POOL_FREE_CONST(pOldEnvironment);
	}
}

///////////////////////////////////////////////////////////////////////////
SATLSharedData::SATLSharedData()
{
}

///////////////////////////////////////////////////////////////////////////
SATLSharedData::~SATLSharedData()
{
}

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
///////////////////////////////////////////////////////////////////////////
void CAudioEventManager::DrawDebugInfo(IRenderer* const pRenderer, float fPosX, float fPosY) const
{
	static float const fHeaderColor[4] = {1.0f, 1.0f, 1.0f, 0.9f};
	static float const fItemPlayingColor[4] = {0.1f, 0.6f, 0.1f, 0.9f};
	static float const fItemOtherColor[4] = {0.8f, 0.8f, 0.8f, 0.9f};

	pRenderer->Draw2dLabel(fPosX, fPosY, 1.6f, fHeaderColor, false, "Audio Events [%" PRISIZE_T "]", m_cActiveAudioEvents.size());
	fPosX += 20.0f;
	fPosY += 17.0f;

	TActiveEventMap::const_iterator Iter(m_cActiveAudioEvents.begin());
	TActiveEventMap::const_iterator const IterEnd(m_cActiveAudioEvents.end());

	for (; Iter != IterEnd; ++Iter)
	{
		CATLEvent* const pEvent = Iter->second;

		pRenderer->Draw2dLabel(fPosX, fPosY, 1.2f, 
			pEvent->m_eType == eAET_PLAY ? fItemPlayingColor : fItemOtherColor, 
			false,
			"%s on %s : %u",
			m_pDebugNameStore->LookupAudioTriggerName(pEvent->m_nTriggerID),
			m_pDebugNameStore->LookupAudioObjectName(pEvent->m_nObjectID),
			pEvent->GetID());

		fPosY += 10.0f;
	}
}

///////////////////////////////////////////////////////////////////////////
void CAudioEventManager::SetDebugNameStore(CATLDebugNameStore const* const pDebugNameStore)
{
	m_pDebugNameStore = pDebugNameStore;
}

//////////////////////////////////////////////////////////////////////////
bool CAudioObjectManager::ReserveID(TAudioObjectID& rAudioObjectID, char const* const sAudioObjectName)
{
	CATLAudioObject* const pNewObject  = GetInstance();

	bool bSuccess = false;
	rAudioObjectID = INVALID_AUDIO_OBJECT_ID;

	if (pNewObject != NPTR) 
	{
		EAudioRequestStatus const eImplResult = (m_pASImpl->RegisterAudioObject(pNewObject->GetImplDataPtr(), sAudioObjectName));

		if (eImplResult == eARS_SUCCESS) 
		{
			pNewObject->IncrementRefCount();
			rAudioObjectID = pNewObject->GetID();
			m_cAudioObjects.insert(std::make_pair(rAudioObjectID, pNewObject));

			bSuccess = true;
		}
		else
		{
			ReleaseInstance(pNewObject);

			bSuccess = false;
		}
	}

	return bSuccess;
}

///////////////////////////////////////////////////////////////////////////
size_t CAudioObjectManager::GetNumAudioObjects() const
{
	return m_cAudioObjects.size();
}

///////////////////////////////////////////////////////////////////////////
size_t CAudioObjectManager::GetNumActiveAudioObjects() const
{
	size_t nNumActiveAudioObjects = 0;
	TActiveObjectMap::const_iterator Iter(m_cAudioObjects.begin());
	TActiveObjectMap::const_iterator const IterEnd(m_cAudioObjects.end());

	for (; Iter != IterEnd; ++Iter)
	{
		CATLAudioObject* const pObject = Iter->second;

		if (pObject->HasActiveEvents())
		{
			++nNumActiveAudioObjects;
		}
	}

	return nNumActiveAudioObjects;
}

///////////////////////////////////////////////////////////////////////////
void CAudioObjectManager::DrawPerObjectDebugInfo(IRenderer* const pRenderer, Vec3 const& vListenerPos) const
{
	TActiveObjectMap::const_iterator Iter(m_cAudioObjects.begin());
	TActiveObjectMap::const_iterator const IterEnd(m_cAudioObjects.end());

	for (; Iter != IterEnd; ++Iter)
	{
		CATLAudioObject* const pObject = Iter->second;

		if (pObject->HasActiveEvents())
		{
			pObject->DrawDebugInfo(pRenderer, vListenerPos, m_pDebugNameStore);
		}
	}
}

///////////////////////////////////////////////////////////////////////////
void CAudioObjectManager::DrawDebugInfo(IRenderer* const pRenderer, float fPosX, float fPosY) const
{
	static float const fHeaderColor[4] = {1.0f, 1.0f, 1.0f, 0.9f};
	static float const fItemActiveColor[4] = {0.1f, 0.6f, 0.1f, 0.9f};
	static float const fItemInactiveColor[4] = {0.8f, 0.8f, 0.8f, 0.9f};

	size_t nObjects = 0;
	float const fHeaderPosY = fPosY;
	fPosX += 20.0f;
	fPosY += 17.0f;

	TActiveObjectMap::const_iterator Iter(m_cAudioObjects.begin());
	TActiveObjectMap::const_iterator const IterEnd(m_cAudioObjects.end());

	for (; Iter != IterEnd; ++Iter)
	{
		CATLAudioObject* const pObject = Iter->second;

		if (pObject->HasActiveEvents())
		{
			pRenderer->Draw2dLabel(fPosX, fPosY, 1.2f, 
				pObject->HasActiveEvents() ? fItemActiveColor : fItemInactiveColor, 
				false, 
				"%u : %s",
				pObject->GetID(), m_pDebugNameStore->LookupAudioObjectName(pObject->GetID()));

			fPosY += 10.0f;
			++nObjects;
		}
	}

	pRenderer->Draw2dLabel(fPosX, fHeaderPosY, 1.6f, fHeaderColor, false, "Audio Objects [%" PRISIZE_T "]", nObjects);
}

///////////////////////////////////////////////////////////////////////////
void CAudioObjectManager::SetDebugNameStore(CATLDebugNameStore* const pDebugNameStore)
{
	m_pDebugNameStore = pDebugNameStore;
}

///////////////////////////////////////////////////////////////////////////
void CATLXMLProcessor::SetDebugNameStore(CATLDebugNameStore* const pDebugNameStore)
{
	m_pDebugNameStore = pDebugNameStore;
}

#endif // INCLUDE_AUDIO_PRODUCTION_CODE
