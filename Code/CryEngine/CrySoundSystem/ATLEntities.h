// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#ifndef ATLOBJECTS_H_INCLUDED
#define ATLOBJECTS_H_INCLUDED

#include "ATLUtils.h"
#include <IAudioSystem.h>
#include <IStreamEngine.h>

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
#include <TimeValue.h>
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

#define ATL_NAME_MAX_LENGTH 64

typedef CryFixedStringT<ATL_NAME_MAX_LENGTH>	TATLNameString;
typedef std::vector<TATLNameString>						TATLNameVec;
typedef std::vector<TAudioControlID>					TControlVector;

struct SATLXMLTags
{
	static char const* const sPlatform;

	static char const* const sRootNodeTag;
	static char const* const sTriggersNodeTag;
	static char const* const sRtpcsNodeTag;
	static char const* const sSwitchesNodeTag;
	static char const* const sPreloadsNodeTag;
	static char const* const sEnvironmentsNodeTag;

	static char const* const sATLTriggerTag;
	static char const* const sATLSwitchTag;
	static char const* const sATLRtpcTag;
	static char const* const sATLSwitchStateTag;
	static char const* const sATLEnvironmentTag;
	static char const* const sATLPlatformsTag;	
	static char const* const sATLConfigGroupTag;

	static char const* const sATLTriggerRequestTag;
	static char const* const sATLSwitchRequestTag;
	static char const* const sATLValueTag;
	static char const* const sATLRtpcRequestTag;
	static char const* const sATLPreloadRequestTag;
	static char const* const sATLEnvironmentRequestTag;

	static char const* const sATLNameAttribute;
	static char const* const sATLInternalNameAttribute;
	static char const* const sATLTypeAttribute;
	static char const* const sATLConfigGroupAttribute;

	static char const* const sATLDataLoadType;
};

struct SATLInternalControlIDs
{
	static TAudioControlID				nObstructionOcclusionCalcSwitchID;
	static TAudioControlID				nLoseFocusTriggerID;
	static TAudioControlID				nGetFocusTriggerID;
	static TAudioControlID				nMuteAllTriggerID;
	static TAudioControlID				nUnmuteAllTriggerID;
	static TAudioSwitchStateID		nOOCIgnoreStateID;		
	static TAudioSwitchStateID		nOOCSingleRayStateID;
	static TAudioSwitchStateID		nOOCMultiRayStateID;	
	static TAudioPreloadRequestID	nGlobalPreloadRequestID;
};

void InitATLControlIDs();// initializes the values in SATLInternalControlIDs (cannot make them static, need the CRCgen from pSystem)

// Forward declarations.
struct	IAudioSystemImplementation;
class		CATLCallbackManager;
struct	EventPhys;
struct	IATLAudioObjectData;
struct	IATLListenerData;
struct	IATLTriggerImplData;
struct	IATLRtpcImplData;
struct	IATLSwitchStateImplData;
struct	IATLEnvironmentImplData;
struct	IATLEventData;
struct	IATLAudioFileEntryData;

enum EATLObjectFlags ATL_ENUM_TYPE
{
	eAOF_NONE	= 0,
};

enum EATLSubsystem ATL_ENUM_TYPE
{
	eAS_NONE = 0,
	eAS_AUDIO_SYSTEM_IMPLEMENTATION,
	eAS_ATL_INTERNAL,
};

enum EATLEventType ATL_ENUM_TYPE
{
	eAET_NONE = 0,
	eAET_PLAY,
	eAET_PREPARE,
	eAET_UNPREPARE,
	eAET_FORCED_PREPARE,
};

enum EAudioFileFlags ATL_ENUM_TYPE
{
	eAFF_CACHED													= BIT(0),
	eAFF_NOTCACHED											= BIT(1),
	eAFF_NOTFOUND												= BIT(2),
	eAFF_MEMALLOCFAIL										= BIT(3),
	eAFF_REMOVABLE											= BIT(4),
	eAFF_LOADING												= BIT(5),
	eAFF_USE_COUNTED										= BIT(6),
	eAFF_NEEDS_RESET_TO_MANUAL_LOADING	= BIT(7),
	eAFF_LOCALIZED											= BIT(8),
};

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
struct SATLDebugInfo
{
	SATLDebugInfo()
		: pImplementationNameString(NPTR)
		, nActiveAudioObjects(0)
		, nActiveAudioEvents(0)
		, nActiveAudioListeners(0)
		, nSyncPhysRays(0)
		, nAsyncPhysRays(0)
	{}

	CryFixedStringT<ATL_NAME_MAX_LENGTH> const*	pImplementationNameString;
	SATLWorldPosition	oActiveListenerPosition;
	size_t						nActiveAudioObjects;
	size_t						nActiveAudioEvents;
	size_t						nActiveAudioListeners;
	size_t						nSyncPhysRays;
	size_t						nAsyncPhysRays;

	CryMutex					oMutex;
};
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

template <typename IDType>
class CATLEntity
{
public:

	CATLEntity(IDType const nID, EATLDataScope const eDataScope)
		: m_nID(nID)
		, m_eDataScope(eDataScope)
	{}

	virtual ~CATLEntity() {}

	virtual IDType				GetID() const {return m_nID;}

	virtual EATLDataScope	GetDataScope() const {return m_eDataScope;}

protected:	

	EATLDataScope					m_eDataScope;

private:

	IDType const					m_nID;
};

typedef CATLEntity<TAudioControlID> TATLControl;
typedef CATLEntity<TAudioObjectID> TATLObject;  

struct SATLSoundPropagationData
{
	SATLSoundPropagationData()
		: fObstruction(0.0f)
		, fOcclusion(0.0f)
	{}

	~SATLSoundPropagationData() {}

	float fObstruction;
	float fOcclusion;
};

class CATLListenerObject : public TATLObject
{
public:

	explicit CATLListenerObject(TAudioObjectID const nID, IATLListenerData* const pImplData = NPTR)
		: TATLObject(nID, eADS_NONE)
		, m_pImplData(pImplData)
	{}

	~CATLListenerObject() {}

	SATLWorldPosition	oPosition;
	IATLListenerData* const m_pImplData;
};

class CATLControlImpl
{
public:

	CATLControlImpl()
		: m_eReceiver(eAS_NONE)
	{}

	explicit CATLControlImpl(EATLSubsystem const receiver)
		: m_eReceiver(receiver)
	{}

	virtual ~CATLControlImpl() {}

	EATLSubsystem GetReceiver() const {return m_eReceiver;}

protected:

	EATLSubsystem	m_eReceiver;
};

class CATLTriggerImpl : public CATLControlImpl
{
public:

	explicit CATLTriggerImpl(
		TAudioTriggerImplID const nID,
		TAudioControlID const nTriggerID,
		EATLSubsystem const eReceiver,
		IATLTriggerImplData const* const pImplData = NPTR)
		: CATLControlImpl(eReceiver)
		, m_nATLID(nID)
		, m_nATLTriggerID(nTriggerID)
		, m_pImplData(pImplData)
	{}

	virtual ~CATLTriggerImpl() {}

	TAudioTriggerImplID const m_nATLID;
	TAudioControlID	const			m_nATLTriggerID;
	IATLTriggerImplData const* const m_pImplData;
};

class CATLTrigger : public TATLControl
{
public:

	typedef std::vector<CATLTriggerImpl const*, STLSoundAllocator<CATLTriggerImpl const*> > TImplPtrVec;

	CATLTrigger(TAudioControlID const nID, EATLDataScope const eDataScope, TImplPtrVec const& rImplPtrs)
		: TATLControl(nID, eDataScope)
		, m_cImplPtrs(rImplPtrs)
	{}

	TImplPtrVec const m_cImplPtrs;
};

class CATLRtpcImpl : public CATLControlImpl
{
public:

	CATLRtpcImpl(EATLSubsystem const eReceiver, IATLRtpcImplData const* const pImplData = NPTR)
		: CATLControlImpl(eReceiver)
		, m_pImplData(pImplData)
	{}

	virtual ~CATLRtpcImpl() {}

	IATLRtpcImplData const* const m_pImplData;
};

class CATLRtpc : public TATLControl
{
public:

	typedef std::vector<CATLRtpcImpl const*, STLSoundAllocator<CATLRtpcImpl const*> > TImplPtrVec;

	CATLRtpc(TAudioControlID const nID, EATLDataScope const eDataScope, TImplPtrVec const& cImplPtrs)
		: TATLControl(nID, eDataScope)
		, m_cImplPtrs(cImplPtrs)
	{}

	~CATLRtpc() {}

	TImplPtrVec	const m_cImplPtrs;
};

class CATLSwitchStateImpl : public CATLControlImpl
{
public:
	
	explicit CATLSwitchStateImpl(EATLSubsystem const eReceiver, IATLSwitchStateImplData const* const pImplData = NPTR)
		: CATLControlImpl(eReceiver)
		, m_pImplData(pImplData)
	{}

	virtual ~CATLSwitchStateImpl() {}

	IATLSwitchStateImplData const* const m_pImplData;
};

class CATLSwitchImpl 
{
public:
	
	CATLSwitchImpl() {}
	~CATLSwitchImpl() {}

	typedef std::map<
		TAudioSwitchStateID, 
		CATLSwitchStateImpl const*, 
		std::less<TAudioSwitchStateID>, 
		STLSoundAllocator<std::pair<TAudioSwitchStateID, CATLSwitchStateImpl const*> > > TStateMap;

	TStateMap m_cStates;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	TATLNameVec m_cStateNames;
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
};

class CATLSwitchState
{
public:

	typedef std::vector<CATLSwitchStateImpl const*, STLSoundAllocator<CATLSwitchStateImpl const*> > TImplPtrVec;

	CATLSwitchState(
		TAudioControlID const nSwitchID, 
		TAudioSwitchStateID const nStateID,
		TImplPtrVec const& cImplPtrs)
		: m_nID(nStateID)
		, m_nSwitchID(nSwitchID)
		, m_cImplPtrs(cImplPtrs)
	{}

	virtual ~CATLSwitchState()
	{}

	virtual TAudioSwitchStateID GetID() const {return m_nID;}
	virtual TAudioSwitchStateID GetParentID() const {return m_nSwitchID;}

	TImplPtrVec	const					m_cImplPtrs;

private:

	TAudioSwitchStateID const	m_nID;
	TAudioControlID	const			m_nSwitchID;
};

class CATLSwitch : public TATLControl
{
public:

	explicit CATLSwitch(TAudioControlID const nID, EATLDataScope const eDataScope)
		: TATLControl(nID, eDataScope)
	{}

	typedef std::map<TAudioSwitchStateID, CATLSwitchState const*, std::less<TAudioSwitchStateID>, STLSoundAllocator<std::pair <TAudioSwitchStateID, CATLSwitchState const*> > > TStateMap;
	TStateMap cStates;
};

class CATLEnvironmentImpl : public CATLControlImpl
{
public:

	explicit CATLEnvironmentImpl(EATLSubsystem const eReceiver, IATLEnvironmentImplData const* const pImplData = NPTR)
		: CATLControlImpl(eReceiver)
		, m_pImplData(pImplData)
	{}

	virtual ~CATLEnvironmentImpl() {}

	IATLEnvironmentImplData const* const m_pImplData;
};

class CATLAudioEnvironment : public CATLEntity<TAudioEnvironmentID>
{
public:

	typedef std::vector<CATLEnvironmentImpl const*, STLSoundAllocator<CATLEnvironmentImpl const*> > TImplPtrVec;

	explicit CATLAudioEnvironment(TAudioEnvironmentID const nID, EATLDataScope const eDataScope, TImplPtrVec const& rImplPtrs)
		: CATLEntity<TAudioEnvironmentID>(nID, eDataScope)
		, m_cImplPtrs(rImplPtrs)
	{}

	virtual ~CATLAudioEnvironment() {}

	TImplPtrVec const m_cImplPtrs;
};

class CATLEvent : public CATLEntity<TAudioEventID>
{
public:

	explicit CATLEvent(TAudioEventID const nID, EATLSubsystem const eSender, IATLEventData* const pImplData)		
		: CATLEntity<TAudioEventID>(nID, eADS_NONE)
		, m_nObjectID(INVALID_AUDIO_OBJECT_ID)
		, m_nTriggerID(INVALID_AUDIO_CONTROL_ID)
		, m_nTriggerImplID(INVALID_AUDIO_TRIGGER_IMPL_ID)
		, m_nTriggerInstanceID(INVALID_AUDIO_TRIGGER_INSTANCE_ID)
		, m_eType(eAET_NONE)
		, m_eSender(eSender)
		, m_pImplData(pImplData)
	{}

	virtual ~CATLEvent() {}

	void					SetDataScope(EATLDataScope const eDataScope) {m_eDataScope = eDataScope;}

	virtual void	Clear()
	{
		m_nObjectID						= INVALID_AUDIO_OBJECT_ID;
		m_nTriggerID					= INVALID_AUDIO_CONTROL_ID;
		m_nTriggerImplID			= INVALID_AUDIO_TRIGGER_IMPL_ID;
		m_nTriggerInstanceID	= INVALID_AUDIO_TRIGGER_INSTANCE_ID;
		m_eType								= eAET_NONE;
		m_eDataScope					= eADS_NONE;
	}

	TAudioObjectID					m_nObjectID;
	TAudioControlID					m_nTriggerID;
	TAudioTriggerImplID			m_nTriggerImplID;
	TAudioTriggerInstanceID	m_nTriggerInstanceID;
	EATLEventType						m_eType;	
	EATLSubsystem const			m_eSender;
	IATLEventData* const		m_pImplData;

private:

	CATLEvent(CATLEvent const&);						// not defined; calls will fail at compile time
	CATLEvent& operator=(CATLEvent const&);	// not defined; calls will fail at compile time
};

class CATLAudioFileEntry
{
public:
	
	explicit CATLAudioFileEntry(char const* const sPassedPath = NPTR, IATLAudioFileEntryData* const pImplData = NPTR)
		:	m_sPath(sPassedPath)
		, m_nSize(0)
		,	m_nUseCount(0)
		,	m_nMemoryBlockAlignment(AUDIO_MEMORY_ALIGNMENT)
		,	m_nFlags(eAFF_NOTFOUND)
		,	m_eDataScope(eADS_ALL)
		,	m_eStreamTaskType(eStreamTaskTypeCount)
		,	m_pMemoryBlock(NPTR)
		,	m_pReadStream(NPTR)
		, m_pImplData(pImplData)
	{
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
		m_oTimeCached.SetValue(0);
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
	}

	virtual ~CATLAudioFileEntry() {}

	CryFixedStringT<MAX_AUDIO_FILE_PATH_LENGTH>	m_sPath;
	size_t																			m_nSize;
	size_t																			m_nUseCount;
	size_t																			m_nMemoryBlockAlignment;
	TATLEnumFlagsType														m_nFlags;
	EATLDataScope																m_eDataScope;
	EStreamTaskType															m_eStreamTaskType;
	_smart_ptr<ICustomMemoryBlock>							m_pMemoryBlock;
	IReadStreamPtr															m_pReadStream;
	IATLAudioFileEntryData*											m_pImplData;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	CTimeValue																	m_oTimeCached;
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
};

class CATLPreloadRequest : public CATLEntity<TAudioPreloadRequestID>
{
public:

	typedef std::vector<TAudioFileEntryID, STLSoundAllocator<TAudioFileEntryID> > TFileEntryIDs;

	explicit CATLPreloadRequest(
		TAudioPreloadRequestID const nID,
		EATLDataScope const eDataScope,
		bool const bAutoLoad,
		TFileEntryIDs const& cFileEntryIDs)
		: CATLEntity<TAudioPreloadRequestID>(nID, eDataScope)
		,	m_bAutoLoad(bAutoLoad)
		, m_cFileEntryIDs(cFileEntryIDs)
	{}

	virtual ~CATLPreloadRequest() {}

	bool const		m_bAutoLoad;
	TFileEntryIDs	m_cFileEntryIDs;
};

struct SAudioTriggerCallbackData
{
	SAudioTriggerCallbackData()
		: nObjectID(INVALID_AUDIO_OBJECT_ID)
		, nTriggerID(INVALID_AUDIO_CONTROL_ID)
		, pCallback(NPTR)
		, pCallbackCookie(NPTR)
	{}

	SAudioTriggerCallbackData(
		TAudioObjectID const nPassedObjectID,
		TAudioControlID const nPassedTriggerID,
		TTriggerFinishedCallback const pPassedCallback,
		void* const pPassedCallbackCookie)
		: nObjectID(nPassedObjectID)
		, nTriggerID(nPassedTriggerID)
		, pCallback(pPassedCallback)
		, pCallbackCookie(pPassedCallbackCookie)
	{}

	TAudioObjectID						nObjectID;
	TAudioControlID						nTriggerID;
	TTriggerFinishedCallback	pCallback;
	void*											pCallbackCookie;
};

class CAudioTriggerCallbackQueue
{
public:

	void Push(SAudioTriggerCallbackData const& rCallbackData);
	void ExecuteAll();

private:

	typedef std::deque<SAudioTriggerCallbackData, STLSoundAllocator<SAudioTriggerCallbackData> > TTriggerCallbacks;

	TTriggerCallbacks	m_aTriggerCallbacks;
	CryCriticalSection m_oCriticalSection;
};

//-------------------- ATLObject container typedefs --------------------------
typedef std::map<TAudioControlID, CATLTrigger const*, std::less<TAudioControlID>, 
	STLSoundAllocator< std::pair<TAudioControlID, CATLTrigger const*> > > TATLTriggerLookup;
typedef std::map<TAudioControlID, CATLRtpc const*, std::less<TAudioControlID>, 
	STLSoundAllocator< std::pair<TAudioControlID, CATLRtpc const*> > > TATLRtpcLookup;
typedef std::map<TAudioControlID, CATLSwitch const*, std::less<TAudioControlID>, 
	STLSoundAllocator< std::pair<TAudioControlID, CATLSwitch const*> > > TATLSwitchLookup;
typedef std::map<TAudioPreloadRequestID, CATLPreloadRequest*, std::less<TAudioPreloadRequestID>, 
	STLSoundAllocator< std::pair<TAudioPreloadRequestID, CATLPreloadRequest*> > > TATLPreloadRequestLookup;
typedef std::map<TAudioEnvironmentID, CATLAudioEnvironment const*, std::less<TAudioEnvironmentID>, 
	STLSoundAllocator< std::pair<TAudioEnvironmentID, CATLAudioEnvironment const*> > > TATLEnvironmentLookup;

//------------------------ ATLInternal Entity Data ----------------------------
struct SATLSwitchStateImplData_internal : public IATLSwitchStateImplData
{
	SATLSwitchStateImplData_internal(TAudioControlID const nSwitchID, TAudioSwitchStateID const nSwitchStateID)
		: nATLInternalSwitchID(nSwitchID)
		, nATLInternalStateID(nSwitchStateID)
	{}
	~SATLSwitchStateImplData_internal()
	{}

	TAudioControlID const nATLInternalSwitchID;
	TAudioSwitchStateID const nATLInternalStateID;
};


#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
class CATLDebugNameStore 
{
public:

	CATLDebugNameStore();
	~CATLDebugNameStore();

	void				SyncChanges(CATLDebugNameStore const& rOtherNameStore);

	void				AddAudioObject(TAudioObjectID const nObjectID, char const* const sName);
	void				AddAudioTrigger(TAudioControlID const nTriggerID, char const* const sName);
	void				AddAudioRtpc(TAudioControlID const nRtpcID, char const* const sName);
	void				AddAudioSwitch(TAudioControlID const nSwitchID, char const* const sName);
	void				AddAudioSwitchState(TAudioControlID const nSwitchID, TAudioSwitchStateID const nStateID, char const* const sName);
	void				AddAudioPreloadRequest(TAudioPreloadRequestID const nRequestID, char const* const sName);
	void				AddAudioEnvironment(TAudioEnvironmentID const nEnvironmentID, char const* const sName);

	void				RemoveAudioObject(TAudioObjectID const nObjectID);
	void				RemoveAudioTrigger(TAudioControlID const nTriggerID);
	void				RemoveAudioRtpc(TAudioControlID const nRtpcID);
	void				RemoveAudioSwitch(TAudioControlID const nSwitchID);
	void				RemoveAudioPreloadRequest(TAudioPreloadRequestID const nRequestID);
	void				RemoveAudioEnvironment(TAudioEnvironmentID const nEnvironmentID);

	bool				AudioObjectsChanged() const {return m_bATLObjectsChanged;}
	bool				AudioTriggersChanged() const {return m_bATLTriggersChanged;}
	bool				AudioRtpcsChanged() const {return m_bATLRtpcsChanged;}
	bool				AudioSwitchesChanged() const {return m_bATLSwitchesChanged;}
	bool				AudioPreloadsChanged() const {return m_bATLPreloadsChanged;}
	bool				AudioEnvironamentsChanged() const {return m_bATLEnvironmentsChanged;}

	char const*	LookupAudioObjectName(TAudioObjectID const nObjectID) const;
	char const*	LookupAudioTriggerName(TAudioControlID const nTriggerID) const;
	char const*	LookupAudioRtpcName(TAudioControlID const nRtpcID) const;
	char const*	LookupAudioSwitchName(TAudioControlID const nSwitchID) const;
	char const*	LookupAudioSwitchStateName(TAudioControlID const nSwitchID, TAudioSwitchStateID const nStateID) const;
	char const*	LookupAudioPreloadRequestName(TAudioPreloadRequestID const nRequestID) const;
	char const*	LookupAudioEnvironmentName(TAudioEnvironmentID const nEnvironmentID) const;

private:

	typedef std::map<TAudioObjectID, CryFixedStringT<ATL_NAME_MAX_LENGTH>, std::less<TAudioObjectID>, STLSoundAllocator< std::pair<TAudioObjectID, CryFixedStringT<ATL_NAME_MAX_LENGTH> > > > TAudioObjectMap;
	typedef std::map<TAudioControlID, CryFixedStringT<ATL_NAME_MAX_LENGTH>, std::less<TAudioControlID>, STLSoundAllocator< std::pair<TAudioControlID, CryFixedStringT<ATL_NAME_MAX_LENGTH> > > > TAudioControlMap;
	typedef std::map<TAudioSwitchStateID, CryFixedStringT<ATL_NAME_MAX_LENGTH>, std::less<TAudioSwitchStateID>, STLSoundAllocator< std::pair<TAudioSwitchStateID, CryFixedStringT<ATL_NAME_MAX_LENGTH> > > > TAudioSwitchStateMap;
	typedef std::map<TAudioControlID, std::pair<CryFixedStringT<ATL_NAME_MAX_LENGTH>, TAudioSwitchStateMap>, std::less<TAudioControlID>, STLSoundAllocator< std::pair<TAudioControlID, std::pair<CryFixedStringT<ATL_NAME_MAX_LENGTH>, TAudioSwitchStateMap> > > > TAudioSwitchMap;
	typedef std::map<TAudioPreloadRequestID, CryFixedStringT<ATL_NAME_MAX_LENGTH>, std::less<TAudioPreloadRequestID>, STLSoundAllocator<std::pair<TAudioPreloadRequestID, CryFixedStringT<ATL_NAME_MAX_LENGTH> > > > TAudioPreloadRequestsMap;
	typedef std::map<TAudioEnvironmentID, CryFixedStringT<ATL_NAME_MAX_LENGTH>, std::less<TAudioEnvironmentID>, STLSoundAllocator<std::pair<TAudioEnvironmentID, CryFixedStringT<ATL_NAME_MAX_LENGTH > > > > TAudioEnvironmentMap;

	TAudioObjectMap							m_cATLObjectNames;
	TAudioControlMap						m_cATLTriggerNames;
	TAudioControlMap						m_cATLRtpcNames;
	TAudioSwitchMap							m_cATLSwitchNames;
	TAudioPreloadRequestsMap		m_cATLPreloadRequestNames;
	TAudioEnvironmentMap				m_cATLEnvironmentNames;

	mutable bool								m_bATLObjectsChanged;
	mutable bool								m_bATLTriggersChanged;
	mutable bool								m_bATLRtpcsChanged;
	mutable bool								m_bATLSwitchesChanged;
	mutable bool								m_bATLPreloadsChanged;
	mutable bool								m_bATLEnvironmentsChanged;
};
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
#endif // ATLOBJECTS_H_INCLUDED
