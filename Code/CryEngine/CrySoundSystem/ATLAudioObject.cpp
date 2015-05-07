// CryEngine Source File.
// Copyright (C), Crytek, 1999-2014.

#include "stdafx.h"
#include "ATLAudioObject.h"
#include "SoundCVars.h"
#include "ATLUtils.h"
#include <I3DEngine.h>
#include <IRenderer.h>
#include <IRenderAuxGeom.h>
#include <ISurfaceType.h>

///////////////////////////////////////////////////////////////////////////
void CATLAudioObjectBase::ReportStartedEvent(CATLEvent const* const pEvent)
{
	m_cActiveEvents.insert(pEvent->GetID());

	switch (pEvent->m_eType)
	{
	case eAET_PLAY:
		{
			SATLTriggerImplState& rTriggerImplState = m_cTriggerImpls[pEvent->m_nTriggerImplID];
			++rTriggerImplState.nPlayingEvents;
			rTriggerImplState.nFlags |= eATS_PLAYING;

			SATLTriggerInstanceState& rTriggerState = m_cTriggers[pEvent->m_nTriggerInstanceID];
			++rTriggerState.nPlayingEvents;
			rTriggerState.nTriggerID = pEvent->m_nTriggerID;
			rTriggerState.nFlags |= (eATS_PLAYING | eATS_STARTING);

			break;
		}
	case eAET_FORCED_PREPARE:
	case eAET_PREPARE:
		{
			m_cTriggerImpls[pEvent->m_nTriggerImplID].nFlags |= eATS_LOADING;

			SATLTriggerInstanceState& rTriggerState = m_cTriggers[pEvent->m_nTriggerInstanceID];
			rTriggerState.nTriggerID = pEvent->m_nTriggerID;
			rTriggerState.nFlags |= eATS_LOADING;

			break;
		}
	case eAET_UNPREPARE:
		{
			m_cTriggerImpls[pEvent->m_nTriggerImplID].nFlags |= eATS_UNLOADING;

			SATLTriggerInstanceState& rTriggerState = m_cTriggers[pEvent->m_nTriggerInstanceID];
			rTriggerState.nTriggerID = pEvent->m_nTriggerID;
			rTriggerState.nFlags |= eATS_UNLOADING;

			break;
		}
	default:
		{
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObjectBase::ReportFinishedEvent(CATLEvent const* const pEvent, bool const bSuccess)
{
	m_cActiveEvents.erase(pEvent->GetID());

	TObjectTriggerStates::iterator Iter(m_cTriggers.begin());

	if (FindPlace(m_cTriggers, pEvent->m_nTriggerInstanceID, Iter))
	{
		switch (pEvent->m_eType)
		{
		case eAET_PLAY:
			{
				SATLTriggerImplState& rTriggerImplState = m_cTriggerImpls[pEvent->m_nTriggerImplID];

				--rTriggerImplState.nPlayingEvents;

				if (rTriggerImplState.nPlayingEvents == 0) 
				{
					rTriggerImplState.nFlags &= ~eATS_PLAYING;
				}

				if (((--(Iter->second.nPlayingEvents)) == 0) && ((rTriggerImplState.nFlags & eATS_STARTING) == 0))
				{
					ReportFinishedTriggerInstance(Iter);
				}

				DecrementRefCount();

				break;
			}
		case eAET_PREPARE:
			{
				if (bSuccess)
				{
					ReportPrepUnprepTriggerImpl(pEvent->m_nTriggerImplID, true);
				}

				DecrementRefCount();

				break;
			}
		case eAET_UNPREPARE:
			{
				if (bSuccess)
				{
					ReportPrepUnprepTriggerImpl(pEvent->m_nTriggerImplID, false);
				}

				DecrementRefCount();

				break;
			}
		case eAET_FORCED_PREPARE:
			{

				break;
			}
		default:
			{

				break;
			}
		}

	}
	else
	{
		g_AudioLogger.Log(eALT_WARNING, "Reported finished event %u on an inactive trigger %u", pEvent->GetID(), pEvent->m_nTriggerID);
	}
}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObjectBase::ReportPrepUnprepTriggerImpl(TAudioTriggerImplID const nTriggerImplID, bool const bPrepared)
{
	if (bPrepared)
	{
		m_cTriggerImpls[nTriggerImplID].nFlags |= eATS_PREPARED;
	}
	else
	{
		m_cTriggerImpls[nTriggerImplID].nFlags &= ~eATS_PREPARED;
	}
}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObjectBase::ReportStartedTriggerInstance(		
	TAudioTriggerInstanceID const nTriggerInstanceID,
	TTriggerFinishedCallback const pCallback,
	void* const pCallbackCookie,
	TATLEnumFlagsType const nFlags)
{
	TObjectTriggerStates::iterator iTriggerInstanceEntry(m_cTriggers.begin());

	if (FindPlace(m_cTriggers, nTriggerInstanceID, iTriggerInstanceEntry))
	{
		SATLTriggerInstanceState& rTriggerInstState = iTriggerInstanceEntry->second;

		rTriggerInstState.pCallbackCookie = pCallbackCookie;
		rTriggerInstState.pCallback = pCallback;

		rTriggerInstState.nFlags &= (~eATS_STARTING);

		if ((nFlags & eATFCF_CALL_ON_AUDIO_THREAD) != 0)
		{
			rTriggerInstState.nFlags |= eATS_CALLBACK_ON_AUDIO_THREAD;
		}

		if (rTriggerInstState.nPlayingEvents == 0)
		{
			//All of the events have finished before we got here, so need to report this trigger as finished.
			ReportFinishedTriggerInstance(iTriggerInstanceEntry);
		}
	}
	else
	{
		g_AudioLogger.Log(eALT_WARNING, "Reported a started instance %u that couldn't be found on an object %u",
			nTriggerInstanceID, GetID());
	}
}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObjectBase::SetSwitchState(TAudioControlID const nSwitchID, TAudioSwitchStateID const nStateID)
{
	m_cSwitchStates[nSwitchID] = nStateID;
}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObjectBase::SetRtpc(TAudioControlID const nRtpcID, float const fValue)
{
	m_cRtpcs[nRtpcID] = fValue;
}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObjectBase::SetEnvironmentAmount(TAudioEnvironmentID const nEnvironmentID, float const fAmount)
{
	if (fAmount > 0.0f)
	{
		m_cEnvironments[nEnvironmentID] = fAmount;
	}
	else
	{
		m_cEnvironments.erase(nEnvironmentID);
	}
}

///////////////////////////////////////////////////////////////////////////
TObjectTriggerImplStates const& CATLAudioObjectBase::GetTriggerImpls() const
{
	return m_cTriggerImpls;
}

///////////////////////////////////////////////////////////////////////////
TObjectEnvironmentMap const& CATLAudioObjectBase::GetEnvironments() const
{
	return m_cEnvironments;
}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObjectBase::ClearEnvironments()
{
	m_cEnvironments.clear();
}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObjectBase::Update(float const fUpdateIntervalMS, SATLWorldPosition const& rListenerPosition)
{

}

CAudioTriggerCallbackQueue* CATLAudioObjectBase::s_pTriggerCallbackQueue = NPTR;

///////////////////////////////////////////////////////////////////////////
void CATLAudioObjectBase::Clear()
{
	m_cActiveEvents.clear();

	m_cTriggers.clear();
	m_cTriggerImpls.clear();
	m_cSwitchStates.clear();
	m_cRtpcs.clear();
	m_cEnvironments.clear();

	m_nRefCounter = 0;
}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObjectBase::ReportFinishedTriggerInstance(TObjectTriggerStates::iterator& iTriggerEntry)
{
	SATLTriggerInstanceState& rTriggerInstState = iTriggerEntry->second;
	TTriggerFinishedCallback const pCallback = rTriggerInstState.pCallback;

	if (pCallback != NPTR)
	{
		if ((rTriggerInstState.nFlags & eATS_CALLBACK_ON_AUDIO_THREAD) != 0)
		{
			(*pCallback)(GetID(), rTriggerInstState.nTriggerID, rTriggerInstState.pCallbackCookie);
		}
		else
		{
			s_pTriggerCallbackQueue->Push(
				SAudioTriggerCallbackData(GetID(), rTriggerInstState.nTriggerID, rTriggerInstState.pCallback, rTriggerInstState.pCallbackCookie));
		}
	}

	if ((rTriggerInstState.nFlags & eATS_PREPARED) != 0)
	{
		// if the trigger instance was manually prepared -- keep it
		rTriggerInstState.nFlags &= ~eATS_PLAYING;
	}
	else
	{
		//if the trigger instance wasn't prepared -- kill it
		m_cTriggers.erase(iTriggerEntry);
	}
}

float const CATLAudioObject::CPropagationProcessor::SRayInfo::s_fSmoothingAlpha = 0.05f;

///////////////////////////////////////////////////////////////////////////
void CATLAudioObject::CPropagationProcessor::SRayInfo::Reset()
{
	fTotalSoundOcclusion = 0.0f;
	nNumHits = 0;
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	fAvgHits = 0.0f;
	fDistToFirstObstacle = FLT_MAX;
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
}

bool CATLAudioObject::CPropagationProcessor::s_bCanIssueRWIs = false;

///////////////////////////////////////////////////////////////////////////
int CATLAudioObject::CPropagationProcessor::OnObstructionTest(EventPhys const* pEvent)
{
	EventPhysRWIResult* const pRWIResult = (EventPhysRWIResult*)pEvent;

	if (pRWIResult->iForeignData == PHYS_FOREIGN_ID_SOUND_OBSTRUCTION)
	{
		CPropagationProcessor::SRayInfo* const pRayInfo = static_cast<CPropagationProcessor::SRayInfo*>(pRWIResult->pForeignData);

		if (pRayInfo != NPTR)
		{
			ProcessObstructionRay(pRWIResult->nHits, pRayInfo);

			SAudioRequest oRequest;
			SAudioCallbackManagerRequestData<eACMRT_REPORT_PROCESSED_OBSTRUCTION_RAY> oRequestData(pRayInfo->nAudioObjectID, pRayInfo->nRayID);
			oRequest.pData = &oRequestData;

			gEnv->pAudioSystem->PushRequest(oRequest);
		}
		else
		{
			assert(false);
		}
	}
	else
	{
		assert(false);
	}

	return 1;
}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObject::CPropagationProcessor::ProcessObstructionRay(int const nNumHits, SRayInfo* const pRayInfo, bool const bReset)
{
	float fTotalOcclusion	= 0.0f;
	int nNumRealHits			= 0;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	float fMinDistance		= FLT_MAX;
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

	if (nNumHits > 0)
	{
		ISurfaceTypeManager* const pSurfaceTypeManager = gEnv->p3DEngine->GetMaterialManager()->GetSurfaceTypeManager();
		size_t const nCount = min(static_cast<size_t>(nNumHits)+1, AUDIO_MAX_RAY_HITS);

		for (size_t i = 0; i < nCount; ++i)
		{
			float const fDist = pRayInfo->aHits[i].dist;

			if (fDist > 0.0f)
			{
				ISurfaceType* const pMat = pSurfaceTypeManager->GetSurfaceType(pRayInfo->aHits[i].surface_idx);

				if (pMat != NPTR)
				{
					const ISurfaceType::SPhysicalParams& physParams = pMat->GetPhyscalParams();
					fTotalOcclusion += physParams.sound_obstruction;// not clamping b/w 0 and 1 for performance reasons
					++nNumRealHits;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
					fMinDistance = min(fMinDistance, fDist);
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
				}
			}
		}
	}

	fTotalOcclusion = clamp_tpl(fTotalOcclusion, 0.0f, 1.0f);

	// If the num hits differs too much from the last ray, reduce the change in TotalOcclusion in inverse proportion.
	// This reduces thrashing at the boundaries of occluding entities.
	int const nAbsHitDiff = abs(nNumRealHits - pRayInfo->nNumHits);
	float const fNumHitCorrection = nAbsHitDiff > 1 ? 1.0f/nAbsHitDiff : 1.0f;
	
	pRayInfo->nNumHits = nNumRealHits;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	pRayInfo->fDistToFirstObstacle = fMinDistance;
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

	if (bReset)
	{
		pRayInfo->fTotalSoundOcclusion = fTotalOcclusion;
		
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
		pRayInfo->fAvgHits = static_cast<float>(pRayInfo->nNumHits);
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
	}
	else
	{
		pRayInfo->fTotalSoundOcclusion += fNumHitCorrection * (fTotalOcclusion - pRayInfo->fTotalSoundOcclusion) * SRayInfo::s_fSmoothingAlpha;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
		pRayInfo->fAvgHits += (pRayInfo->nNumHits - pRayInfo->fAvgHits) * SRayInfo::s_fSmoothingAlpha; 
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
	}
}

///////////////////////////////////////////////////////////////////////////
size_t CATLAudioObject::CPropagationProcessor::NumRaysFromCalcType(EAudioObjectObstructionCalcType const eCalcType)
{
	size_t nNumRays = 0;
	switch (eCalcType)
	{
	case eAOOCT_IGNORE:
		{
			nNumRays = 0;

			break;
		}	
	case eAOOCT_SINGLE_RAY:
		{
			nNumRays = 1;

			break;
		}
		case eAOOCT_MULTI_RAY:
			{
				nNumRays = 5;

				break;
			}
		case eAOOCT_NONE: // falls through
		default:
			{
				assert(false);//unknown ObstructionOcclusionCalcType

				break;
			}
	}

	return nNumRays;
}

#define AUDIO_MAX_OBSTRUCTION_RAYS			static_cast<size_t>(5)
#define AUDIO_MIN_OBSTRUCTION_DISTANCE	0.3f

///////////////////////////////////////////////////////////////////////////
CATLAudioObject::CPropagationProcessor::CPropagationProcessor(
	TAudioObjectID const nObjectID,
	SATLWorldPosition const& rPosition,
	size_t& rRefCounter) 
	: m_nRemainingRays(0)
	, m_nTotalRays(0)
	, m_oObstructionValue(0.2f, 0.0001f)
	, m_oOcclusionValue(0.2f, 0.0001f)
	, m_rPosition(rPosition)
	, m_rRefCounter(rRefCounter)
	, m_fCurrListenerDist(0.0f)
	, m_eObstOcclCalcType(eAOOCT_NONE)

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	, m_vRayDebugInfos(AUDIO_MAX_OBSTRUCTION_RAYS)
	, m_fTimeSinceLastUpdateMS(0.0f)
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
{
	m_vRayInfos.reserve(AUDIO_MAX_OBSTRUCTION_RAYS);

	for (size_t i = 0; i < AUDIO_MAX_OBSTRUCTION_RAYS; ++i)
	{
		m_vRayInfos.push_back(SRayInfo(i, nObjectID));
	}
}

//////////////////////////////////////////////////////////////////////////
CATLAudioObject::CPropagationProcessor::~CPropagationProcessor()
{
}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObject::CPropagationProcessor::Update(float const fUpdateMS)
{
	m_oObstructionValue.Update(fUpdateMS);
	m_oOcclusionValue.Update(fUpdateMS);

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	m_fTimeSinceLastUpdateMS += fUpdateMS;
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
}

//////////////////////////////////////////////////////////////////////////
void CATLAudioObject::CPropagationProcessor::SetObstructionOcclusionCalcType(EAudioObjectObstructionCalcType const eObstOcclCalcType)
{
	m_eObstOcclCalcType = eObstOcclCalcType;

	if (eObstOcclCalcType == eAOOCT_IGNORE)
	{
		// Reset the sound obstruction/occlusion when disabled.
		m_oObstructionValue.Reset();
		m_oOcclusionValue.Reset();
	}
}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObject::CPropagationProcessor::GetPropagationData(SATLSoundPropagationData& rPropagationData) const
{
	rPropagationData.fObstruction = m_oObstructionValue.GetCurrent();
	rPropagationData.fOcclusion = m_oOcclusionValue.GetCurrent();
}

inline float frand_symm() {return 2.0f*cry_frand() - 1.0f;}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObject::CPropagationProcessor::RunObstructionQuery(
	SATLWorldPosition const& rListenerPosition,
	bool const bSyncCall,
	bool const bReset)
{
	if (m_nRemainingRays == 0)
	{
		static Vec3 const vUp(0.0f, 0.0f, 1.0f);
		static float const fMinPeripheralRayOffset = 0.3f;
		static float const fMaxPeripheralRayOffset = 1.0f;
		static float const fMinRandomOffset = 0.05f;
		static float const fMaxRandomOffset = 0.5f;
		static float const fMinOffsetDistance = 1.0f;
		static float const fMaxOffsetDistance = 20.0f;

		// the previous query has already been processed
		Vec3 const& vListener = rListenerPosition.GetPositionVec();
		Vec3 const& vObject = m_rPosition.GetPositionVec();
		Vec3 const vDiff = vObject - vListener;

		float const fDistance = vDiff.GetLength();

		m_fCurrListenerDist = fDistance;

		m_nTotalRays = NumRaysFromCalcType(m_eObstOcclCalcType);

		float const fOffsetParam = clamp_tpl((fDistance - fMinOffsetDistance) / (fMaxOffsetDistance - fMinOffsetDistance), 0.0f, 1.0f);
		float const fOffsetScale = fMaxPeripheralRayOffset * fOffsetParam + fMinPeripheralRayOffset * (1.0f - fOffsetParam);
		float const fRndOffsetScale = fMaxRandomOffset * fOffsetParam + fMinRandomOffset * (1.0f - fOffsetParam);

		Vec3 const vSide = vDiff.GetNormalized() % vUp;

		// the 0th ray is always shot from the listener to the center of the source 
		// the 0th ray only gets 1/2 of the random variation
		CastObstructionRay(
			vListener,
			(vUp * frand_symm() + vSide * frand_symm()) * fRndOffsetScale * 0.5f,
			vDiff,
			0,
			bSyncCall,
			bReset);

		if (m_nTotalRays > 1)
		{
			// rays 1 and 2 start below and above the listener and go towards the source
			CastObstructionRay(
				vListener - (vUp * fOffsetScale),
				(vUp * frand_symm() + vSide * frand_symm()) * fRndOffsetScale,
				vDiff,
				1,
				bSyncCall,
				bReset);
			CastObstructionRay(
				vListener + (vUp * fOffsetScale),
				(vUp * frand_symm() + vSide * frand_symm()) * fRndOffsetScale,
				vDiff,
				2,
				bSyncCall,
				bReset);

			// rays 3 and 4 start left and right of the listener and go towards the source
			CastObstructionRay(
				vListener - (vSide * fOffsetScale),
				(vUp * frand_symm() + vSide * frand_symm()) * fRndOffsetScale,
				vDiff,
				3,
				bSyncCall,
				bReset);
			CastObstructionRay(
				vListener + (vSide * fOffsetScale),
				(vUp * frand_symm() + vSide * frand_symm()) * fRndOffsetScale,
				vDiff,
				4,
				bSyncCall,
				bReset);
		}

		if (bSyncCall)
		{
			// If the ObstructionQuery was synchronous, calculate the new obstruction/occlusion values right away.
			// Reset the resulting values without smoothing if bReset is true.
			ProcessObstructionOcclusion(bReset);
		}
		else
		{
			++m_rRefCounter;// for each async ray BATCH increase the refcount by 1
		}
	}
}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObject::CPropagationProcessor::ReportRayProcessed(size_t const nRayID)
{
	assert(m_nRemainingRays > 0);// make sure there are rays remaining
	assert((0 <= nRayID) && (nRayID <= m_nTotalRays));// check RayID validity

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	if (m_nRemainingRays == 0 || m_rRefCounter == 0)
	{
		CryFatalError("Negative ref or ray count on audio object %u", m_vRayInfos[nRayID].nAudioObjectID);
	}
#endif

	if (--m_nRemainingRays == 0)
	{
		ProcessObstructionOcclusion();
		--m_rRefCounter;
	}
}

//////////////////////////////////////////////////////////////////////////
void CATLAudioObject::CPropagationProcessor::ReleasePendingRays()
{
	if (m_nRemainingRays > 0)
	{
		m_nRemainingRays = 0;
		--m_rRefCounter;
	}
}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObject::CPropagationProcessor::ProcessObstructionOcclusion(bool const bReset)
{
	if (m_nTotalRays > 0)
	{
		// the obstruction value always comes from the 0th ray, going directly from the listener to the source
		float fObstruction = m_vRayInfos[0].fTotalSoundOcclusion;
		float fOcclusion = 0.0f;

		if (m_fCurrListenerDist > ATL_FLOAT_EPSILON)
		{
			if (m_nTotalRays > 1)
			{
				// the total occlusion value is the average of the occlusions of all of the peripheral rays
				for (size_t i = 1; i < m_nTotalRays; ++i)
				{
					fOcclusion += m_vRayInfos[i].fTotalSoundOcclusion;
				}

				fOcclusion /= (m_nTotalRays - 1);
			}
			else
			{
				fOcclusion = fObstruction;
			}

			//the obstruction effect gets less pronounced when the distance between the object and the listener increases
			fObstruction *= min (g_SoundCVars.m_fFullObstructionMaxDistance / m_fCurrListenerDist, 1.0f);

			// since the Obstruction filter is applied on top of Occlusion, make sure we only apply what's exceeding the occlusion value
			fObstruction = max(fObstruction - fOcclusion, 0.0f);
		}
		else
		{
			// sound is tracking the listener, there is no obstruction
			fObstruction = 0.0f;
			fOcclusion = 0.0f;
		}

		m_oObstructionValue.SetNewTarget(fObstruction, bReset);
		m_oOcclusionValue.SetNewTarget(fOcclusion, bReset);

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
		if (m_fTimeSinceLastUpdateMS > 100.0f) // only re-sample the rays about 10 times per second for a smoother debug drawing
		{
			m_fTimeSinceLastUpdateMS = 0.0f;

			for (size_t i = 0; i < m_nTotalRays; ++i)
			{
				SRayInfo const& rRayInfo			= m_vRayInfos[i];
				SRayDebugInfo& rRayDebugInfo	= m_vRayDebugInfos[i];

				rRayDebugInfo.vBegin = rRayInfo.vStartPosition + rRayInfo.vRndOffset;
				rRayDebugInfo.vEnd = rRayInfo.vStartPosition + rRayInfo.vRndOffset+ rRayInfo.vDirection;

				if (rRayDebugInfo.vStableEnd.IsZeroFast()) 
				{
					// to be moved to the PropagationProcessor Reset method
					rRayDebugInfo.vStableEnd = rRayDebugInfo.vEnd;
				}
				else
				{
					rRayDebugInfo.vStableEnd += (rRayDebugInfo.vEnd - rRayDebugInfo.vStableEnd) * 0.1f;
				}

				rRayDebugInfo.fDistToNearestObstacle	= rRayInfo.fDistToFirstObstacle;
				rRayDebugInfo.nNumHits								= rRayInfo.nNumHits;
				rRayDebugInfo.fAvgHits								= rRayInfo.fAvgHits;
				rRayDebugInfo.fOcclusionValue					= rRayInfo.fTotalSoundOcclusion;
			}
		}
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
	}
}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObject::CPropagationProcessor::CastObstructionRay(
	Vec3 const& rOrigin, 
	Vec3 const& rRndOffset,
	Vec3 const& rDirection, 
	size_t const nRayIdx,
	bool const bSyncCall,
	bool const bReset)
{
	static int const nPhysicsFlags =  ent_water|ent_static|ent_sleeping_rigid|ent_rigid|ent_terrain;
	SRayInfo& rRayInfo = m_vRayInfos[nRayIdx];

	int const nNumHits = gEnv->pPhysicalWorld->RayWorldIntersection(
		rOrigin + rRndOffset, 
		rDirection, 
		nPhysicsFlags, 
		bSyncCall ? rwi_pierceability0 : rwi_pierceability0 | rwi_queue,
		rRayInfo.aHits,
		static_cast<int>(AUDIO_MAX_RAY_HITS), 
		NPTR,
		0, 
		&rRayInfo, 
		PHYS_FOREIGN_ID_SOUND_OBSTRUCTION);

	if (bSyncCall)
	{
		ProcessObstructionRay(nNumHits, &rRayInfo, bReset);
	}
	else
	{
		++m_nRemainingRays;
	}

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	rRayInfo.vStartPosition	= rOrigin;
	rRayInfo.vDirection			= rDirection;
	rRayInfo.vRndOffset			= rRndOffset;

	if (bSyncCall)
	{
		++s_nTotalSyncPhysRays;
	}
	else
	{
		++s_nTotalAsyncPhysRays;
	}
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObject::Update(float const fUpdateIntervalMS, SATLWorldPosition const& rListenerPosition)
{
	CATLAudioObjectBase::Update(fUpdateIntervalMS, rListenerPosition);
	m_oPropagationProcessor.Update(fUpdateIntervalMS);

	if (CanRunObstructionOcclusion())
	{
		float const fDistance = (m_oPosition.GetPositionVec() - rListenerPosition.GetPositionVec()).GetLength();

		if ((AUDIO_MIN_OBSTRUCTION_DISTANCE < fDistance) && (fDistance < g_SoundCVars.m_fOcclusionMaxDistance))
		{
			// make the physics ray cast call sync or async depending on the distance to the listener
			bool const bSyncCall = (fDistance <= g_SoundCVars.m_fOcclusionMaxSyncDistance);
			m_oPropagationProcessor.RunObstructionQuery(rListenerPosition, bSyncCall);
		}
	}
}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObject::ReportPhysicsRayProcessed(size_t const nRayID)
{
	m_oPropagationProcessor.ReportRayProcessed(nRayID);
}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObject::SetPosition(SATLWorldPosition const& oNewPosition)
{
	m_oPosition = oNewPosition;
}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObject::SetTriggerFlag(TAudioControlID const nTriggerID, EATLTriggerStatus const eStatusFlag, bool bOn)
{
	uint32& nBitField = m_cTriggers[nTriggerID].nFlags;// this creates an entry if nTriggerID is not yet present in m_cTriggers

	if (bOn)
	{
		nBitField |= eStatusFlag;
	}
	else
	{
		nBitField &= ~eStatusFlag;
	}
}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObject::Clear()
{
	CATLAudioObjectBase::Clear();
	m_oPosition = SATLWorldPosition();
}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObject::SetObstructionOcclusionCalc(EAudioObjectObstructionCalcType const ePassedOOCalcType)
{
	assert(ePassedOOCalcType != eAOOCT_NONE);
	m_oPropagationProcessor.SetObstructionOcclusionCalcType(ePassedOOCalcType);
}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObject::ResetObstructionOcclusion(SATLWorldPosition const& rListenerPosition)
{
	// cast the obstruction rays synchronously and reset the obstruction/occlusion values 
	// (instead of merely setting them as targets for the SmoothFloats)
	m_oPropagationProcessor.RunObstructionQuery(rListenerPosition, true, true);
}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObject::GetPropagationData(SATLSoundPropagationData& rPropagationData)
{
	m_oPropagationProcessor.GetPropagationData(rPropagationData);
}

//////////////////////////////////////////////////////////////////////////
void CATLAudioObject::ReleasePendingRays()
{
	m_oPropagationProcessor.ReleasePendingRays();
}

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
///////////////////////////////////////////////////////////////////////////
void CATLAudioObjectBase::CheckBeforeRemoval(CATLDebugNameStore const* const pDebugNameStore)
{
	// warn if there is activity on an object being cleared
	if (!m_cActiveEvents.empty())
	{
		char const* const sEventString = GetEventIDs("; ").c_str();
		g_AudioLogger.Log(eALT_WARNING, "Active events on an object to be released: %u! EventIDs: %s", GetID(), sEventString);
	}

	if (!m_cTriggers.empty())
	{
		char const* const sTriggerString = GetTriggerNames("; ", pDebugNameStore).c_str();
		g_AudioLogger.Log(eALT_WARNING, "Active triggers on an object to be released: %u! TriggerNames: %s", GetID(), sTriggerString);
	}
}

typedef std::map<TAudioControlID, size_t, std::less<TAudioControlID>, 
	STLSoundAllocator<std::pair<TAudioControlID, size_t> > > TTriggerCountMap;

///////////////////////////////////////////////////////////////////////////
CryFixedStringT<128> CATLAudioObjectBase::GetTriggerNames(char const* const sSeparator, CATLDebugNameStore const* const pDebugNameStore)
{
	CryFixedStringT<128> sTriggerString;
	TTriggerCountMap cTriggerCounts;
	TObjectTriggerStates::const_iterator iTrigger = m_cTriggers.begin();
	TObjectTriggerStates::const_iterator const iTriggerEnd = m_cTriggers.end();

	for (; iTrigger != iTriggerEnd; ++iTrigger)
	{
		++(cTriggerCounts[iTrigger->second.nTriggerID]);
	}

	TTriggerCountMap::const_iterator iTriggerCount = cTriggerCounts.begin();
	TTriggerCountMap::const_iterator const iTriggerCountEnd = cTriggerCounts.end();

	for (; iTriggerCount != iTriggerCountEnd; ++iTriggerCount)
	{
		char const* const pName = pDebugNameStore->LookupAudioTriggerName(iTriggerCount->first);

		if (pName != NPTR)
		{
			size_t const nInstances = iTriggerCount->second;

			if (nInstances == 1)
			{
				sTriggerString.Format("%s%s%s", sTriggerString.c_str(), pName, sSeparator);
			}
			else
			{
				sTriggerString.Format("%s%s(%d inst.)%s", sTriggerString.c_str(), pName, nInstances, sSeparator);
			}
		}
	}

	return sTriggerString;
}

///////////////////////////////////////////////////////////////////////////
CryFixedStringT<128> CATLAudioObjectBase::GetEventIDs(char const* const sSeparator)
{
	CryFixedStringT<128> sEventString;
	TObjectEventSet::const_iterator iEvent = m_cActiveEvents.begin();
	TObjectEventSet::const_iterator iEnd = m_cActiveEvents.end();

	for (; iEvent != iEnd; ++iEvent)
	{
		sEventString.Format("%s%u%s", sEventString.c_str(), *iEvent, sSeparator);
	}

	return sEventString;
}

//////////////////////////////////////////////////////////////////////////
float const CATLAudioObjectBase::CStateDebugDrawData::fMinAlpha = 0.5f;
float const CATLAudioObjectBase::CStateDebugDrawData::fMaxAlpha = 1.0f;
int const CATLAudioObjectBase::CStateDebugDrawData::nMaxToMinUpdates = 100;

///////////////////////////////////////////////////////////////////////////
CATLAudioObjectBase::CStateDebugDrawData::CStateDebugDrawData(TAudioSwitchStateID const nState)
	: nCurrentState(nState)
	, fCurrentAlpha(fMaxAlpha)
{
}

///////////////////////////////////////////////////////////////////////////
CATLAudioObjectBase::CStateDebugDrawData::~CStateDebugDrawData()
{
}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObjectBase::CStateDebugDrawData::Update(TAudioSwitchStateID const nNewState)
{
	if ((nNewState == nCurrentState) && (fCurrentAlpha > fMinAlpha))
	{	
		fCurrentAlpha -= (fMaxAlpha - fMinAlpha)/ nMaxToMinUpdates;
	}
	else if (nNewState != nCurrentState)
	{
		nCurrentState = nNewState;
		fCurrentAlpha = fMaxAlpha;
	}
}

///////////////////////////////////////////////////////////////////////////
void CATLAudioObject::DrawDebugInfo(IRenderer* const pRenderer, Vec3 const& vListenerPos, CATLDebugNameStore const* const pDebugNameStore) const
{
	m_oPropagationProcessor.DrawObstructionRays(pRenderer);

	if (!m_cTriggers.empty())
	{
		Vec3 vPos(m_oPosition.GetPositionVec());

		Vec3 vScreenPos(ZERO);
		pRenderer->ProjectToScreen(vPos.x, vPos.y, vPos.z, &vScreenPos.x, &vScreenPos.y, &vScreenPos.z);
		if ((0.0f <= vScreenPos.z) && (vScreenPos.z <= 1.0f))
		{
			vScreenPos.x = vScreenPos.x * 0.01f * pRenderer->GetWidth();
			vScreenPos.y = vScreenPos.y * 0.01f * pRenderer->GetHeight();

			float const fDist = vPos.GetDistance(vListenerPos);

			if ((g_SoundCVars.m_nDrawAudioDebug & eADDF_DRAW_SPHERES) != 0)
			{
				IRenderAuxGeom* const pAuxGeom(pRenderer->GetIRenderAuxGeom());
				if (pAuxGeom != NPTR)
				{
					SAuxGeomRenderFlags const nPreviousRenderFlags = pAuxGeom->GetRenderFlags();
					SAuxGeomRenderFlags nNewRenderFlags(e_Def3DPublicRenderflags|e_AlphaBlended);
					nNewRenderFlags.SetCullMode(e_CullModeNone);
					pAuxGeom->SetRenderFlags(nNewRenderFlags);
					float const fRadius = 0.15f;
					pAuxGeom->DrawSphere(vPos, fRadius, ColorB(255, 1, 1, 255));
					pAuxGeom->SetRenderFlags(nPreviousRenderFlags);
				}
			}


			float const fFontSize = 1.3f;
			float const fLineHeight = 12.0f;

			if ((g_SoundCVars.m_nDrawAudioDebug & eADDF_SHOW_OBJECT_STATES) != 0)
			{
				float const fSwitchTextColor[4] = {0.8f, 0.8f, 0.8f, 1.0f};
				CryFixedStringT<128> sSwitchString(0);
				Vec3 vSwitchPos(vScreenPos);

				TObjectStateMap::const_iterator iState= m_cSwitchStates.begin();
				TObjectStateMap::const_iterator const iStateEnd = m_cSwitchStates.end();

				for (; iState != iStateEnd; ++iState)
				{
					TAudioControlID const nSwitchID = iState->first;
					TAudioSwitchStateID const nStateID = iState->second;

					char const* const pSwitchName = pDebugNameStore->LookupAudioSwitchName(nSwitchID);
					char const* const pStateName = pDebugNameStore->LookupAudioSwitchStateName(nSwitchID, nStateID);

					if ((pSwitchName != NPTR) && (pStateName != NPTR))
					{
						CStateDebugDrawData& oDrawData = m_cStateDrawInfoMap[nSwitchID];
						oDrawData.Update(nStateID);

						float const fColor[4] = {fSwitchTextColor[0], fSwitchTextColor[1], fSwitchTextColor[2], oDrawData.fCurrentAlpha};

						vSwitchPos.y -= fLineHeight;
						pRenderer->Draw2dLabel(
							vSwitchPos.x, 
							vSwitchPos.y, 
							fFontSize, 
							fColor, 
							false,
							"%s: %s\n",
							pSwitchName,
							pStateName);
					}
				}
			}

			if ((g_SoundCVars.m_nDrawAudioDebug & eADDF_SHOW_OBJECT_LABEL) != 0)
			{
				static float const fObjectTextColor[4] = {0.90f, 0.90f, 0.90f, 1.0f};
				static float const fObjectGrayTextColor[4] = {0.50f, 0.50f, 0.50f, 1.0f};

				size_t const nNumRays = m_oPropagationProcessor.GetNumRays();
				SATLSoundPropagationData oPropagationData;
				m_oPropagationProcessor.GetPropagationData(oPropagationData);

				TAudioObjectID const nObjectID = GetID();
				pRenderer->Draw2dLabel(
					vScreenPos.x, 
					vScreenPos.y, 
					fFontSize, 
					fObjectTextColor, 
					false, 
					"%s ID: %u RefCnt:%2" PRISIZE_T " Dist:%4.1fm",
					pDebugNameStore->LookupAudioObjectName(nObjectID), 
					nObjectID,
					GetRefCount(),
					fDist);
				pRenderer->Draw2dLabel(
					vScreenPos.x, 
					vScreenPos.y + fLineHeight, 
					fFontSize, 
					nNumRays > 0 ? fObjectTextColor : fObjectGrayTextColor, 
					false, 
					"Obst: %3.2f Occl: %3.2f #Rays: %1" PRISIZE_T,
					oPropagationData.fObstruction,
					oPropagationData.fOcclusion,
					nNumRays);
			}

			if ((g_SoundCVars.m_nDrawAudioDebug & eADDF_SHOW_OBJECT_TRIGGERS) != 0)
			{
				float const fTriggerTextColor[4] = {0.8f, 0.8f, 0.8f, 1.0f};
				CryFixedStringT<128> sTriggerString;
				TTriggerCountMap cTriggerCounts;
				TObjectTriggerStates::const_iterator iTrigger = m_cTriggers.begin();
				TObjectTriggerStates::const_iterator const iTriggerEnd = m_cTriggers.end();

				for (; iTrigger != iTriggerEnd; ++iTrigger)
				{
					if ((iTrigger->second.nFlags & eATS_PLAYING) != 0)
					{
						++(cTriggerCounts[iTrigger->second.nTriggerID]);
					}
				}

				TTriggerCountMap::const_iterator iTriggerCount = cTriggerCounts.begin();
				TTriggerCountMap::const_iterator const iTriggerCountEnd = cTriggerCounts.end();

				for (; iTriggerCount != iTriggerCountEnd; ++iTriggerCount)
				{
					char const* const pName = pDebugNameStore->LookupAudioTriggerName(iTriggerCount->first);
					if (pName != NPTR)
					{
						size_t const nInstances = iTriggerCount->second;
						if (nInstances == 1)
						{
							sTriggerString.Format("%s%s\n", sTriggerString.c_str(), pName);
						}
						else
						{
							sTriggerString.Format("%s%s: %" PRISIZE_T "\n", sTriggerString.c_str(), pName, nInstances);
						}
					}
				}

				pRenderer->Draw2dLabel(
					vScreenPos.x,
					vScreenPos.y + 2.0f*fLineHeight, 
					fFontSize,
					fTriggerTextColor,
					false,
					"%s",
					sTriggerString.c_str());
			}

			if ((g_SoundCVars.m_nDrawAudioDebug & eADDF_SHOW_OBJECT_RTPCS) != 0)
			{
				float const fRtpcTextColor[4] = {0.8f, 0.8f, 0.8f, 1.0f};
				CryFixedStringT<128> sRtpcString(0);
				Vec3 vRtpcPos(vScreenPos);

				TObjectRtpcMap::const_iterator iRtpc= m_cRtpcs.begin();
				TObjectRtpcMap::const_iterator const iRtpcEnd = m_cRtpcs.end();

				for (; iRtpc != iRtpcEnd; ++iRtpc)
				{
					TAudioControlID const nRtpcID = iRtpc->first;
					float const fRtpcValue = iRtpc->second;

					char const* const pRtpcName = pDebugNameStore->LookupAudioRtpcName(nRtpcID);

					if (pRtpcName != NPTR)
					{
						float const fXOffset = (strlen(pRtpcName) + 5.6f) * 5.4f * fFontSize;

						vRtpcPos.y -= fLineHeight;
						pRenderer->Draw2dLabel(
							vRtpcPos.x - fXOffset, 
							vRtpcPos.y, 
							fFontSize, 
							fRtpcTextColor, 
							false,
							"%s: %2.2f\n",
							pRtpcName,
							fRtpcValue);
					}
				}
			}

			if ((g_SoundCVars.m_nDrawAudioDebug & eADDF_SHOW_OBJECT_ENVIRONMENTS) != 0)
			{
				float const fEnvTextColor[4] = {0.8f, 0.8f, 0.8f, 1.0f};
				CryFixedStringT<128> sEnvString(0);
				Vec3 vEnvPos(vScreenPos);

				TObjectEnvironmentMap::const_iterator iEnv= m_cEnvironments.begin();
				TObjectEnvironmentMap::const_iterator const iEnvEnd = m_cEnvironments.end();

				for (; iEnv != iEnvEnd; ++iEnv)
				{
					TAudioControlID const nEnvID = iEnv->first;
					float const fEnvValue = iEnv->second;

					char const* const pEnvName = pDebugNameStore->LookupAudioEnvironmentName(nEnvID);

					if (pEnvName != NPTR)
					{
						float const fXOffset = (strlen(pEnvName) + 5.1f) * 5.4f * fFontSize;

						vEnvPos.y += fLineHeight;
						pRenderer->Draw2dLabel(
							vEnvPos.x - fXOffset, 
							vEnvPos.y, 
							fFontSize, 
							fEnvTextColor, 
							false,
							"%s: %.2f\n",
							pEnvName,
							fEnvValue);
					}
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
size_t CATLAudioObject::CPropagationProcessor::s_nTotalSyncPhysRays = 0;
size_t CATLAudioObject::CPropagationProcessor::s_nTotalAsyncPhysRays = 0;

///////////////////////////////////////////////////////////////////////////
void CATLAudioObject::CPropagationProcessor::DrawObstructionRays(IRenderer* const pRenderer) const
{
	static ColorB const cObstructedRayColor(200, 20, 1, 255);
	static ColorB const cFreeRayColor(20, 200, 1, 255);
	static ColorB const cIntersectionSphereColor(250, 200, 1, 240);
	static float const aObstructedRayLabelColor[4] = {1.0f, 0.0f, 0.02f, 0.9f};
	static float const aFreeRayLabelColor[4] = {0.0f, 1.0f, 0.02f, 0.9f};
	static float const fCollisionPtSphereRadius = 0.01f;

	if (CanRunObstructionOcclusion())
	{
		IRenderAuxGeom* const pAuxGeom(pRenderer->GetIRenderAuxGeom());

		if (pAuxGeom != NPTR)
		{
			SAuxGeomRenderFlags const nPreviousRenderFlags = pAuxGeom->GetRenderFlags();
			SAuxGeomRenderFlags nNewRenderFlags(e_Def3DPublicRenderflags|e_AlphaBlended);
			nNewRenderFlags.SetCullMode(e_CullModeNone);

			for (size_t i = 0; i < m_nTotalRays; ++i)
			{
				bool const bRayObstructed = (m_vRayDebugInfos[i].nNumHits > 0); 
				Vec3 const vRayEnd = bRayObstructed ? 
					m_vRayDebugInfos[i].vBegin + (m_vRayDebugInfos[i].vEnd - m_vRayDebugInfos[i].vBegin).GetNormalized() * m_vRayDebugInfos[i].fDistToNearestObstacle :
					m_vRayDebugInfos[i].vEnd;// only draw the ray to the first collision point

				if ((g_SoundCVars.m_nDrawAudioDebug & eADDF_DRAW_OBSTRUCTION_RAYS) != 0)
				{
					ColorB const& rRayColor = bRayObstructed ? cObstructedRayColor: cFreeRayColor;

					pAuxGeom->SetRenderFlags(nNewRenderFlags);

					if (bRayObstructed)
					{
						// mark the nearest collision with a small sphere
						pAuxGeom->DrawSphere(vRayEnd, fCollisionPtSphereRadius, cIntersectionSphereColor);
					}

					pAuxGeom->DrawLine(m_vRayDebugInfos[i].vBegin, rRayColor, vRayEnd, rRayColor, 1.0f);
					pAuxGeom->SetRenderFlags(nPreviousRenderFlags);
				}

				if ((g_SoundCVars.m_nDrawAudioDebug & eADDF_SHOW_OBSTRUCTION_RAY_LABELS) != 0)
				{
					Vec3 vScreenPos(ZERO);
					pRenderer->ProjectToScreen(m_vRayDebugInfos[i].vStableEnd.x, m_vRayDebugInfos[i].vStableEnd.y, m_vRayDebugInfos[i].vStableEnd.z, &vScreenPos.x, &vScreenPos.y, &vScreenPos.z);

					if ((0.0f <= vScreenPos.z) && (vScreenPos.z <= 1.0f))
					{
						vScreenPos.x = vScreenPos.x * 0.01f * pRenderer->GetWidth();
						vScreenPos.y = vScreenPos.y * 0.01f * pRenderer->GetHeight();

						float const fColorLerp = clamp_tpl(m_vRayInfos[i].fAvgHits, 0.0f, 1.0f);
						float const aLabelColor[4] = 
						{aObstructedRayLabelColor[0] * fColorLerp + aFreeRayLabelColor[0] * (1.0f - fColorLerp), 
						aObstructedRayLabelColor[1] * fColorLerp + aFreeRayLabelColor[1] * (1.0f - fColorLerp), 
						aObstructedRayLabelColor[2] * fColorLerp + aFreeRayLabelColor[2] * (1.0f - fColorLerp), 
						aObstructedRayLabelColor[3] * fColorLerp + aFreeRayLabelColor[3] * (1.0f - fColorLerp)};

						pRenderer->Draw2dLabel(
							vScreenPos.x, 
							vScreenPos.y - 12.0f, 
							1.2f, 
							aLabelColor,
							true,
							"ObjID:%u\n#Hits:%2.1f\nOccl:%3.2f", 
							m_vRayInfos[i].nAudioObjectID, // a const member, will not be overwritten by a thread filling the obstruction data in
							m_vRayDebugInfos[i].fAvgHits, 
							m_vRayDebugInfos[i].fOcclusionValue);
					}
				}
			}
		}		
	}
}
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
