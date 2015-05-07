// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#ifndef CATLAUDIOOBJECT_H_INCLUDED
#define CATLAUDIOOBJECT_H_INCLUDED

#include "ATLEntities.h"
#include <ATLEntityData.h>
#include <IPhysics.h>
#include <TimeValue.h>

enum EATLTriggerStatus ATL_ENUM_TYPE
{
	eATS_NONE													=	0,
	eATS_PLAYING											= BIT(0),
	eATS_PREPARED											= BIT(1),
	eATS_LOADING											= BIT(2),
	eATS_UNLOADING										= BIT(3),
	eATS_STARTING											= BIT(4),
	eATS_WAITING_FOR_REMOVAL					= BIT(5),
	eATS_CALLBACK_ON_AUDIO_THREAD			= BIT(6),
};

struct SATLTriggerImplState
{
	uint32 nFlags;
	size_t nPlayingEvents;
};

struct SATLTriggerInstanceState
{
	SATLTriggerInstanceState()
		: nFlags(eATS_NONE)
		, nTriggerID(INVALID_AUDIO_CONTROL_ID)
		, nPlayingEvents(0)
		, fExpirationTimeMS(0.0f)
		, fRemainingTimeMS(0.0f)
		, pCallbackCookie(NPTR)
		, pCallback(NPTR)
	{}

	TATLEnumFlagsType					nFlags;
	TAudioControlID						nTriggerID;
	size_t										nPlayingEvents;
	float											fExpirationTimeMS;
	float											fRemainingTimeMS;
	void*											pCallbackCookie;
	TTriggerFinishedCallback	pCallback;
};

struct SATLSwitchState
{
	TAudioSwitchStateID	nStateID;
};

struct SATLRtpcState
{
	float				sValue;
};

// CATLAudioObjectBase-related typedefs
typedef std::set<TAudioEventID, std::less<TAudioEventID>, STLSoundAllocator<TAudioEventID> > TObjectEventSet;

typedef std::map<TAudioTriggerImplID, SATLTriggerImplState, std::less<TAudioTriggerImplID>,
	STLSoundAllocator<std::pair<TAudioTriggerImplID, SATLTriggerImplState> > > TObjectTriggerImplStates;

typedef std::map<TAudioTriggerInstanceID, SATLTriggerInstanceState, std::less<TAudioTriggerInstanceID>, 
	STLSoundAllocator<std::pair<TAudioTriggerInstanceID, SATLTriggerInstanceState> > > TObjectTriggerStates;

typedef std::map<TAudioControlID, TAudioSwitchStateID, std::less<TAudioControlID>, 
	STLSoundAllocator<std::pair<TAudioControlID, TAudioSwitchStateID> > > TObjectStateMap;

typedef std::map<TAudioControlID, float, std::less<TAudioControlID>, 
	STLSoundAllocator<std::pair<TAudioControlID, float> > > TObjectRtpcMap;

typedef std::map<TAudioEnvironmentID, float, std::less<TAudioEnvironmentID>, 
	STLSoundAllocator<std::pair<TAudioEnvironmentID, float> > > TObjectEnvironmentMap;

// this class wraps the common functionality shared by the AudioObject and the GlobalAudioObject
class CATLAudioObjectBase : public TATLObject
{
public:

	static void SetAudioTriggerCallbackQueue(CAudioTriggerCallbackQueue* const pCallbackQueue) {s_pTriggerCallbackQueue = pCallbackQueue;}

	virtual ~CATLAudioObjectBase() {}

	void ReportStartedEvent(CATLEvent const* const pEvent);
	void ReportFinishedEvent(CATLEvent const* const pEvent, bool const bSuccess);

	void ReportPrepUnprepTriggerImpl(TAudioTriggerImplID const nTriggerImplID, bool const bPrepared);

	void ReportStartedTriggerInstance(
		TAudioTriggerInstanceID const nTriggerInstanceID,
		TTriggerFinishedCallback const pCallback,
		void* const pCallbackCookie,
		TATLEnumFlagsType const nFalgs);

	void SetSwitchState(TAudioControlID const nSwitchID, TAudioSwitchStateID const nStateID);
	void SetRtpc(TAudioControlID const nRtpcID, float const fValue);
	void SetEnvironmentAmount(TAudioEnvironmentID const nEnvironmentID, float const fAmount);

	TObjectTriggerImplStates const&	GetTriggerImpls() const;
	TObjectEnvironmentMap const&		GetEnvironments() const;
	void														ClearEnvironments();

	bool HasActiveEvents() {return !m_cActiveEvents.empty();}
	TObjectEventSet const& GetActiveEvents () const {return m_cActiveEvents;}

	void IncrementRefCount() {++m_nRefCounter;}
	void DecrementRefCount() {assert(m_nRefCounter>0); --m_nRefCounter;}
	size_t GetRefCount() const {return m_nRefCounter;}
	IATLAudioObjectData* GetImplDataPtr() const {return m_pImplData;}

	virtual bool HasPosition() const = 0;

protected:

	static CAudioTriggerCallbackQueue* s_pTriggerCallbackQueue;

	CATLAudioObjectBase(TAudioObjectID const nObjectID, EATLDataScope const eDataScope, IATLAudioObjectData* const pImplData = NPTR)
		: TATLObject(nObjectID, eDataScope)
		, m_nRefCounter(0)
		, m_pImplData(pImplData)
	{}

	void Clear();
	void Update(float const fUpdateIntervalMS, SATLWorldPosition const& rListenerPosition);
	void ReportFinishedTriggerInstance(TObjectTriggerStates::iterator& iTriggerEntry);

	TObjectEventSet							m_cActiveEvents;
	TObjectTriggerStates				m_cTriggers;
	TObjectTriggerImplStates		m_cTriggerImpls;
	TObjectRtpcMap							m_cRtpcs;
	TObjectEnvironmentMap				m_cEnvironments;
	TObjectStateMap							m_cSwitchStates;
	size_t											m_nRefCounter;
	IATLAudioObjectData* const	m_pImplData;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
public:

	void CheckBeforeRemoval(CATLDebugNameStore const* const pDebugNameStore);

protected:

	class CStateDebugDrawData
	{
	public:

		CStateDebugDrawData(TAudioSwitchStateID const nState = INVALID_AUDIO_SWITCH_STATE_ID);
		~CStateDebugDrawData();

		void Update(TAudioSwitchStateID const nNewState);

		TAudioSwitchStateID nCurrentState;
		float								fCurrentAlpha;

	private:

		static float const	fMaxAlpha;
		static float const	fMinAlpha;
		static int const		nMaxToMinUpdates;
	};

	typedef std::map<TAudioControlID, CStateDebugDrawData, std::less<TAudioControlID>, 
		STLSoundAllocator<std::pair<TAudioControlID, CStateDebugDrawData> > > TStateDrawInfoMap;

	CryFixedStringT<128> GetTriggerNames(char const* const sSeparator, CATLDebugNameStore const* const pDebugNameStore);
	CryFixedStringT<128> GetEventIDs(char const* const sSeparator);

	mutable TStateDrawInfoMap		m_cStateDrawInfoMap;
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
};

class CATLGlobalAudioObject : public CATLAudioObjectBase
{
public:

	explicit CATLGlobalAudioObject(TAudioObjectID const nID, IATLAudioObjectData* const pImplData = NPTR)
		:	CATLAudioObjectBase(nID, eADS_GLOBAL, pImplData)
	{}

	~CATLGlobalAudioObject() {}

	VIRTUAL bool HasPosition() const {return false;}
};

#define AUDIO_MAX_RAY_HITS static_cast<size_t>(5)

class CATLAudioObject : public CATLAudioObjectBase
{
public:

	class CPropagationProcessor
	{
	public:

		static bool s_bCanIssueRWIs;

		struct SRayInfo
		{
			static float const s_fSmoothingAlpha;

			SRayInfo(size_t const nPassedRayID, TAudioObjectID const nPassedAudioObjectID)
				: nRayID(nPassedRayID)
				, nAudioObjectID(nPassedAudioObjectID)
				, fTotalSoundOcclusion(0.0f)
				, nNumHits(0)
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
				, vStartPosition(ZERO)
				, vDirection(ZERO)
				, vRndOffset(ZERO)
				, fAvgHits(0.0f)
				, fDistToFirstObstacle(FLT_MAX)
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
			{}

			SRayInfo& operator=( const SRayInfo &rOther )
			{
				const_cast<size_t&>(nRayID) = rOther.nRayID;
				const_cast<TAudioObjectID&>(nAudioObjectID) = rOther.nAudioObjectID;
				fTotalSoundOcclusion = rOther.fTotalSoundOcclusion;
				nNumHits = rOther.nNumHits;
				for (size_t i = 0; i < AUDIO_MAX_RAY_HITS; ++i)
					aHits[i] = rOther.aHits[i];
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
				vStartPosition = rOther.vStartPosition;
				vDirection = rOther.vDirection;
				vRndOffset = rOther.vRndOffset;
				fAvgHits = rOther.fAvgHits;
				fDistToFirstObstacle = rOther.fDistToFirstObstacle;
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

				return *this;
			}

			~SRayInfo() {}

			void Reset();

			size_t const					nRayID;
			TAudioObjectID const	nAudioObjectID;
			float									fTotalSoundOcclusion;
			int										nNumHits;
			ray_hit								aHits[AUDIO_MAX_RAY_HITS];

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
			Vec3									vStartPosition;
			Vec3									vDirection;
			Vec3									vRndOffset;
			float									fAvgHits;
			float									fDistToFirstObstacle;
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
		};

		typedef std::vector<SRayInfo, STLSoundAllocator<SRayInfo> > TRayInfoVec;

	public:

		CPropagationProcessor(
			TAudioObjectID const nObjectID, 
			SATLWorldPosition const& rPosition, 
			size_t& rRefCounter);

		~CPropagationProcessor();

		// PhysicsSystem callback
		static int		OnObstructionTest(EventPhys const* pEvent);
		static void		ProcessObstructionRay(int const nNumHits, SRayInfo* const pRayInfo, bool const bReset = false);
		static size_t	NumRaysFromCalcType(EAudioObjectObstructionCalcType const eCalcType);

		void					Update(float const fUpdateMS);
		void					SetObstructionOcclusionCalcType(EAudioObjectObstructionCalcType const eObstOcclCalcType);
		bool					CanRunObstructionOcclusion() const {return s_bCanIssueRWIs && m_eObstOcclCalcType != eAOOCT_IGNORE;}
		void					GetPropagationData(SATLSoundPropagationData& rPropagationData) const;
		void					RunObstructionQuery(SATLWorldPosition const& rListenerPosition, bool const bSyncCall, bool const bReset = false);
		void					ReportRayProcessed(size_t const nRayID);
		void					ReleasePendingRays();

	private:

		void					ProcessObstructionOcclusion(bool const bReset = false);
		void					CastObstructionRay(Vec3 const& rOrigin,
																		Vec3 const& rRndOffset,
																		Vec3 const& rDirection,
																		size_t const nRayIdx,
																		bool const bSyncCall,
																		bool const bReset = false);

		size_t													m_nRemainingRays;
		size_t													m_nTotalRays;

		CSmoothFloat										m_oObstructionValue;
		CSmoothFloat										m_oOcclusionValue;
		SATLWorldPosition const&				m_rPosition;

		size_t&													m_rRefCounter;

		float														m_fCurrListenerDist;
		TRayInfoVec											m_vRayInfos;
		EAudioObjectObstructionCalcType	m_eObstOcclCalcType;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	public:

		static size_t	s_nTotalSyncPhysRays;
		static size_t	s_nTotalAsyncPhysRays;

		void					DrawObstructionRays(IRenderer* const pRenderer) const;
		size_t				GetNumRays() const {return NumRaysFromCalcType(m_eObstOcclCalcType);}
	
	private:
		
		struct SRayDebugInfo
		{
			SRayDebugInfo()
				: vBegin(ZERO)
				, vEnd(ZERO)
				, vStableEnd(ZERO)
				, fOcclusionValue(0.0f)
				, fDistToNearestObstacle(FLT_MAX)
				, nNumHits(0)
				, fAvgHits(0.0f)
			{}

			~SRayDebugInfo() {}

			Vec3	vBegin;
			Vec3	vEnd;
			Vec3	vStableEnd;
			float fOcclusionValue;
			float fDistToNearestObstacle;
			int		nNumHits;
			float fAvgHits;
		};

		typedef std::vector<SRayDebugInfo, STLSoundAllocator<SRayDebugInfo> > TRayDebugInfoVec;

private:

		TRayDebugInfoVec						m_vRayDebugInfos;
		mutable float								m_fTimeSinceLastUpdateMS;
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
	};

public:

	explicit CATLAudioObject(TAudioObjectID const nID, IATLAudioObjectData* const pImplData = NPTR)
		: CATLAudioObjectBase(nID, eADS_NONE, pImplData)
		, m_nFlags(eAOF_NONE)
		, m_oPropagationProcessor(nID, m_oPosition, m_nRefCounter)
	{}

	virtual ~CATLAudioObject() {}

	// CATLAudioObjectBase
	VIRTUAL bool	HasPosition() const {return true;}
	// ~CATLAudioObjectBase

	virtual void	Update(float const fUpdateIntervalMS, SATLWorldPosition const& rListenerPosition);
	virtual void	Clear();
	void					ReportPhysicsRayProcessed(size_t const nRayID);
	void					SetPosition(SATLWorldPosition const& oNewPosition);
	void					SetObstructionOcclusionCalc(EAudioObjectObstructionCalcType const ePassedOOCalcType);
	void					ResetObstructionOcclusion(SATLWorldPosition const& rListenerPosition);
	bool					CanRunObstructionOcclusion() const {return m_oPropagationProcessor.CanRunObstructionOcclusion();}
	void					GetPropagationData(SATLSoundPropagationData& rPropagationData);
	void					ReleasePendingRays();

private:

	CATLAudioObject(CATLAudioObject const&);						// not defined; calls will fail at compile time
	CATLAudioObject& operator=(CATLAudioObject const&);	// not defined; calls will fail at compile time

	void					SetTriggerFlag(TAudioControlID const nTriggerID, EATLTriggerStatus const eStatusFlag, bool bOn = true);

	uint32								m_nFlags;
	SATLWorldPosition			m_oPosition;
	CPropagationProcessor	m_oPropagationProcessor;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
public:

	void DrawDebugInfo(IRenderer* const pRenderer, Vec3 const& vListenerPos, CATLDebugNameStore const* const pDebugNameStore) const;
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
};

#endif // CATLAUDIOOBJECT_H_INCLUDED
