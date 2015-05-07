// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#ifndef INTERNAL_INTERFACES_H_INCLUDED
#define INTERNAL_INTERFACES_H_INCLUDED

#include <IAudioInterfacesCommonData.h>
#include <IAudioSystem.h>
#include <IAudioSystemImplementation.h>
#include <smartptr.h>
#include <CryName.h>

enum EAudioEventResult ATL_ENUM_TYPE
{
	eAER_NONE						= 0,
	eAER_SUCCESS				= 1,
	eAER_FAILED					= 2,
	eAER_STILL_LOADING	= 3,
};

enum EAudioEventState ATL_ENUM_TYPE
{
	eAES_NONE						= 0,
	eAES_LOADING				= BIT(0),
	eAES_UNLOADING			= BIT(1),
	eAES_LOADED					= BIT(2),
	eAES_PLAYING				= BIT(3),
	eAES_PAUSED					= BIT(4),
	eAES_STOPPING				= BIT(5),
	eAES_STOPPED				= BIT(6),
	eAES_ON_STOP_CALLED	= BIT(7),
	eAES_RETRY_PLAY			= BIT(8),
};

enum EAudioRequestInfoFlags ATL_ENUM_TYPE
{
	eARIF_NONE											= 0,
	eARIF_WAITING_FOR_REMOVAL				= BIT(0),
	eARIF_WAITING_FOR_SYNC_CALLBACK	= BIT(1),
};

inline TATLIDType AudioStringToID(char const* const sSource)
{
	return static_cast<TATLIDType>(gEnv->pSystem->GetCrc32Gen()->GetCRC32Lowercase(sSource));
}

// do we still need this?
struct IParamInfo{};
struct IAudioEventInfo{};

struct IAudioEvent 
{
	virtual void							Update() = 0;
	virtual void							Prepare(IAudioEventInfo const* const pIAudioEventInfo) = 0;
	virtual void							Reset() = 0;
	virtual EAudioEventResult	Play() = 0;
	virtual void							TogglePause(bool const bPause) = 0;
	virtual void							Stop() = 0;
	virtual void							SetPos(Vec3 const& rPos) = 0;
	virtual void							SetParam(IParamInfo const* const pIParamInfo) = 0;
	virtual void							SetState(EAudioEventState const eState) = 0;
	virtual uint32						GetState() const = 0;
	virtual void							ReleaseAllInstances() = 0;
};

//////////////////////////////////////////////////////////////////////////
struct SAudioRequestDataInternal : public _i_multithread_reference_target_t 
{
	explicit SAudioRequestDataInternal(EAudioRequestType eType = eART_NONE)
		: eRequestType(eType)
	{}

	virtual void Release();

	EAudioRequestType const eRequestType;
};

//////////////////////////////////////////////////////////////////////////
struct SAudioManagerRequestDataInternalBase : public SAudioRequestDataInternal
{
	explicit SAudioManagerRequestDataInternalBase(EAudioManagerRequestType const eAMRType)
		: SAudioRequestDataInternal(eART_AUDIO_MANAGER_REQUEST)
		, eType(eAMRType)
	{}

	EAudioManagerRequestType const eType;

private:

	SAudioManagerRequestDataInternalBase();
};

//////////////////////////////////////////////////////////////////////////
template <EAudioManagerRequestType T>
struct SAudioManagerRequestDataInternal : public SAudioManagerRequestDataInternalBase
{
	SAudioManagerRequestDataInternal(SAudioManagerRequestData<T> const* const pAMRData)
		: SAudioManagerRequestDataInternalBase(pAMRData->eType)
	{}
};

//////////////////////////////////////////////////////////////////////////
template <>
struct SAudioManagerRequestDataInternal<eAMRT_RESERVE_AUDIO_OBJECT_ID> : public SAudioManagerRequestDataInternalBase
{
	SAudioManagerRequestDataInternal(SAudioManagerRequestData<eAMRT_RESERVE_AUDIO_OBJECT_ID> const* const pAMRData)
		: SAudioManagerRequestDataInternalBase(pAMRData->eType)
		, pObjectID(pAMRData->pObjectID)
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
		, sObjectName(pAMRData->sObjectName)
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
	{}

	TAudioObjectID* const pObjectID;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	CryFixedStringT<MAX_AUDIO_OBJECT_NAME_LENGTH>	sObjectName;
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
};

//////////////////////////////////////////////////////////////////////////
template <>
struct SAudioManagerRequestDataInternal<eAMRT_ADD_REQUEST_LISTENER> : public SAudioManagerRequestDataInternalBase
{
	SAudioManagerRequestDataInternal(SAudioManagerRequestData<eAMRT_ADD_REQUEST_LISTENER> const* const pAMRData)
		: SAudioManagerRequestDataInternalBase(pAMRData->eType)
		, pRequestOwner(pAMRData->pRequestOwner)
		, func(pAMRData->func)
	{}

	void const*	pRequestOwner;
	void				(*func)(SAudioRequestInfo const* const);
};

//////////////////////////////////////////////////////////////////////////
template <>
struct SAudioManagerRequestDataInternal<eAMRT_REMOVE_REQUEST_LISTENER> : public SAudioManagerRequestDataInternalBase
{
	SAudioManagerRequestDataInternal(SAudioManagerRequestData<eAMRT_REMOVE_REQUEST_LISTENER> const* const pAMRData)
		: SAudioManagerRequestDataInternalBase(pAMRData->eType)
		, pRequestOwner(pAMRData->pRequestOwner)
		, func(pAMRData->func)
	{}

	void const*	pRequestOwner;
	void				(*func)(SAudioRequestInfo const* const);
};

//////////////////////////////////////////////////////////////////////////
template <>
struct SAudioManagerRequestDataInternal<eAMRT_PARSE_CONTROLS_DATA> : public SAudioManagerRequestDataInternalBase
{
	SAudioManagerRequestDataInternal(SAudioManagerRequestData<eAMRT_PARSE_CONTROLS_DATA> const* const pAMRData)
		: SAudioManagerRequestDataInternalBase(pAMRData->eType)
		, sConfigPath(pAMRData->sFolderPath)
		, eDataScope(pAMRData->eDataScope)
	{}

	CryFixedStringT<MAX_AUDIO_FILE_PATH_LENGTH> const sConfigPath;
	EATLDataScope	const																eDataScope;
};

//////////////////////////////////////////////////////////////////////////
template <>
struct SAudioManagerRequestDataInternal<eAMRT_PARSE_PRELOADS_DATA> : public SAudioManagerRequestDataInternalBase
{
	SAudioManagerRequestDataInternal(SAudioManagerRequestData<eAMRT_PARSE_PRELOADS_DATA> const* const pAMRData)
		: SAudioManagerRequestDataInternalBase(pAMRData->eType)
		, sConfigPath(pAMRData->sFolderPath)
		, eDataScope(pAMRData->eDataScope)
	{}

	CryFixedStringT<MAX_AUDIO_FILE_PATH_LENGTH> const sConfigPath;
	EATLDataScope	const																eDataScope;
};

//////////////////////////////////////////////////////////////////////////
template <>
struct SAudioManagerRequestDataInternal<eAMRT_CLEAR_CONTROLS_DATA> : public SAudioManagerRequestDataInternalBase
{
	SAudioManagerRequestDataInternal(SAudioManagerRequestData<eAMRT_CLEAR_CONTROLS_DATA> const* const pAMRData)
		: SAudioManagerRequestDataInternalBase(pAMRData->eType)
		, eDataScope(pAMRData->eDataScope)
	{}

	EATLDataScope	const eDataScope;
};

//////////////////////////////////////////////////////////////////////////
template <>
struct SAudioManagerRequestDataInternal<eAMRT_CLEAR_PRELOADS_DATA> : public SAudioManagerRequestDataInternalBase
{
	SAudioManagerRequestDataInternal(SAudioManagerRequestData<eAMRT_CLEAR_PRELOADS_DATA> const* const pAMRData)
		: SAudioManagerRequestDataInternalBase(pAMRData->eType)
		, eDataScope(pAMRData->eDataScope)
	{}

	EATLDataScope	const eDataScope;
};

//////////////////////////////////////////////////////////////////////////
template <>
struct SAudioManagerRequestDataInternal<eAMRT_PRELOAD_SINGLE_REQUEST> : public SAudioManagerRequestDataInternalBase
{
	SAudioManagerRequestDataInternal(SAudioManagerRequestData<eAMRT_PRELOAD_SINGLE_REQUEST> const* const pAMRData)
		: SAudioManagerRequestDataInternalBase(pAMRData->eType)
		, nPreloadRequest(pAMRData->nPreloadRequestID)
		,	bAutoLoadOnly(pAMRData->bAutoLoadOnly)
	{}

	TAudioPreloadRequestID const nPreloadRequest;
	bool const bAutoLoadOnly;
};

//////////////////////////////////////////////////////////////////////////
template <>
struct SAudioManagerRequestDataInternal<eAMRT_UNLOAD_SINGLE_REQUEST> : public SAudioManagerRequestDataInternalBase
{
	SAudioManagerRequestDataInternal(SAudioManagerRequestData<eAMRT_UNLOAD_SINGLE_REQUEST> const* const pAMRData)
		: SAudioManagerRequestDataInternalBase(pAMRData->eType)
		, nPreloadRequest(pAMRData->nPreloadRequestID)
	{}

	TAudioPreloadRequestID const nPreloadRequest;
};

//////////////////////////////////////////////////////////////////////////
template <>
struct SAudioManagerRequestDataInternal<eAMRT_UNLOAD_AFCM_DATA_BY_SCOPE> : public SAudioManagerRequestDataInternalBase
{
	SAudioManagerRequestDataInternal(SAudioManagerRequestData<eAMRT_UNLOAD_AFCM_DATA_BY_SCOPE> const* const pAMRData)
		: SAudioManagerRequestDataInternalBase(pAMRData->eType)
		, eDataScope(pAMRData->eDataScope)
	{}

	EATLDataScope	const eDataScope;
};

//////////////////////////////////////////////////////////////////////////
template <>
struct SAudioManagerRequestDataInternal<eAMRT_REFRESH_AUDIO_SYSTEM> : public SAudioManagerRequestDataInternalBase
{
	SAudioManagerRequestDataInternal(SAudioManagerRequestData<eAMRT_REFRESH_AUDIO_SYSTEM> const* const pAMRData)
		: SAudioManagerRequestDataInternalBase(pAMRData->eType)
		, sLevelName(pAMRData->sLevelName)
	{}

	CryFixedStringT<MAX_AUDIO_FILE_NAME_LENGTH> const sLevelName;
};

//////////////////////////////////////////////////////////////////////////
template <>
struct SAudioManagerRequestDataInternal<eAMRT_CHANGE_LANGUAGE> : public SAudioManagerRequestDataInternalBase
{
	SAudioManagerRequestDataInternal(SAudioManagerRequestData<eAMRT_CHANGE_LANGUAGE> const* const pAMRData)
		: SAudioManagerRequestDataInternalBase(pAMRData->eType)
	{}
};

//////////////////////////////////////////////////////////////////////////
struct SAudioCallbackManagerRequestDataInternalBase : public SAudioRequestDataInternal
{
	explicit SAudioCallbackManagerRequestDataInternalBase(EAudioCallbackManagerRequestType const eAMRType)
		: SAudioRequestDataInternal(eART_AUDIO_CALLBACK_MANAGER_REQUEST)
		, eType(eAMRType)
	{}

	EAudioCallbackManagerRequestType const eType;

private:

	SAudioCallbackManagerRequestDataInternalBase();
};

//////////////////////////////////////////////////////////////////////////
template <EAudioCallbackManagerRequestType T>
struct SAudioCallbackManagerRequestDataInternal : public SAudioCallbackManagerRequestDataInternalBase
{
	SAudioCallbackManagerRequestDataInternal(SAudioCallbackManagerRequestData<T> const* const pAMRData)
		: SAudioCallbackManagerRequestDataInternalBase(pAMRData->eType)
	{}
};

//////////////////////////////////////////////////////////////////////////
template <>
struct SAudioCallbackManagerRequestDataInternal<eACMRT_REPORT_FINISHED_EVENT> : public SAudioCallbackManagerRequestDataInternalBase
{
	SAudioCallbackManagerRequestDataInternal(SAudioCallbackManagerRequestData<eACMRT_REPORT_FINISHED_EVENT> const* const pACMRData)
		: SAudioCallbackManagerRequestDataInternalBase(pACMRData->eType)
		, nEventID(pACMRData->nEventID)
		, bSuccess(pACMRData->bSuccess)
	{}

	TAudioEventID	const	nEventID;
	bool const					bSuccess;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioCallbackManagerRequestDataInternal<eACMRT_REPORT_PROCESSED_OBSTRUCTION_RAY> : public SAudioCallbackManagerRequestDataInternalBase
{
	explicit SAudioCallbackManagerRequestDataInternal(SAudioCallbackManagerRequestData<eACMRT_REPORT_PROCESSED_OBSTRUCTION_RAY> const* const pACMRData)
		: SAudioCallbackManagerRequestDataInternalBase(pACMRData->eType)
		, nRayID(pACMRData->nRayID)
		, nObjectID(pACMRData->nObjectID)
	{}

	TAudioObjectID const	nObjectID;
	size_t const					nRayID;
};

//////////////////////////////////////////////////////////////////////////
struct SAudioObjectRequestDataInternalBase : public SAudioRequestDataInternal
{
	explicit SAudioObjectRequestDataInternalBase(EAudioObjectRequestType const eAORType)
		: SAudioRequestDataInternal(eART_AUDIO_OBJECT_REQUEST)
		, eType(eAORType)
	{}

	EAudioObjectRequestType const eType;

private:

	SAudioObjectRequestDataInternalBase();
};

//////////////////////////////////////////////////////////////////////////
template <EAudioObjectRequestType T>
struct SAudioObjectRequestDataInternal : public SAudioObjectRequestDataInternalBase
{
	SAudioObjectRequestDataInternal()
		: SAudioObjectRequestDataInternalBase(T)
	{}
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestDataInternal<eAORT_EXECUTE_TRIGGER> : public SAudioObjectRequestDataInternalBase
{
	explicit SAudioObjectRequestDataInternal(SAudioObjectRequestData<eAORT_EXECUTE_TRIGGER> const* const pAORData)
		: SAudioObjectRequestDataInternalBase(pAORData->eType)
		, nTriggerID(pAORData->nTriggerID)
		, fTimeUntilRemovalInMS(pAORData->fTimeUntilRemovalInMS)
		, eLipSyncMethod(pAORData->eLipSyncMethod)
		, pCallback(pAORData->pCallback)
		, pCallbackCookie(pAORData->pCallbackCookie)
		, nFlags(pAORData->nFlags)
	{}

	TAudioControlID const						nTriggerID;
	float const											fTimeUntilRemovalInMS;
	ELipSyncMethod const						eLipSyncMethod;
	TTriggerFinishedCallback const	pCallback;
	void*	const											pCallbackCookie;
	TATLEnumFlagsType	const					nFlags;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestDataInternal<eAORT_PREPARE_TRIGGER> : public SAudioObjectRequestDataInternalBase
{
	explicit SAudioObjectRequestDataInternal(SAudioObjectRequestData<eAORT_PREPARE_TRIGGER> const* const pAORData)
		: SAudioObjectRequestDataInternalBase(pAORData->eType)
		, nTriggerID(pAORData->nTriggerID)
	{}

	TAudioControlID const nTriggerID;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestDataInternal<eAORT_UNPREPARE_TRIGGER> : public SAudioObjectRequestDataInternalBase
{
	explicit SAudioObjectRequestDataInternal(SAudioObjectRequestData<eAORT_UNPREPARE_TRIGGER> const* const pAORData)
		: SAudioObjectRequestDataInternalBase(pAORData->eType)
		, nTriggerID(pAORData->nTriggerID)
	{}

	TAudioControlID const nTriggerID;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestDataInternal<eAORT_STOP_TRIGGER> : public SAudioObjectRequestDataInternalBase
{
	explicit SAudioObjectRequestDataInternal(SAudioObjectRequestData<eAORT_STOP_TRIGGER> const* const pAORData)
		: SAudioObjectRequestDataInternalBase(pAORData->eType)
		, nTriggerID(pAORData->nTriggerID)
	{}

	TAudioControlID const nTriggerID;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestDataInternal<eAORT_STOP_ALL_TRIGGERS> : public SAudioObjectRequestDataInternalBase
{
	explicit SAudioObjectRequestDataInternal(SAudioObjectRequestData<eAORT_STOP_ALL_TRIGGERS> const* const pAORData)
		: SAudioObjectRequestDataInternalBase(pAORData->eType)
	{}
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestDataInternal<eAORT_SET_POSITION> : public SAudioObjectRequestDataInternalBase
{
	explicit SAudioObjectRequestDataInternal(SAudioObjectRequestData<eAORT_SET_POSITION> const* const pAORData)
		: SAudioObjectRequestDataInternalBase(pAORData->eType)
		, oPosition(pAORData->oPosition) 
	{}

	SATLWorldPosition const oPosition;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestDataInternal<eAORT_SET_RTPC_VALUE> : public SAudioObjectRequestDataInternalBase
{
	explicit SAudioObjectRequestDataInternal(SAudioObjectRequestData<eAORT_SET_RTPC_VALUE> const* const pAORData)
		: SAudioObjectRequestDataInternalBase(pAORData->eType)
		, nControlID(pAORData->nControlID)
		, fValue(pAORData->fValue) 
	{}

	TAudioControlID const	nControlID;
	float const						fValue;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestDataInternal<eAORT_SET_SWITCH_STATE> : public SAudioObjectRequestDataInternalBase
{
	explicit SAudioObjectRequestDataInternal(SAudioObjectRequestData<eAORT_SET_SWITCH_STATE> const* const pAORData)
		: SAudioObjectRequestDataInternalBase(pAORData->eType)
		, nSwitchID(pAORData->nSwitchID)
		, nStateID(pAORData->nStateID) 
	{}

	TAudioControlID const			nSwitchID;
	TAudioSwitchStateID const	nStateID;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestDataInternal<eAORT_SET_VOLUME> : public SAudioObjectRequestDataInternalBase
{
	explicit SAudioObjectRequestDataInternal(SAudioObjectRequestData<eAORT_SET_VOLUME> const* const pAORData)
		: SAudioObjectRequestDataInternalBase(pAORData->eType)
		, fVolume(pAORData->fVolume) 
	{}

	float const						fVolume;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestDataInternal<eAORT_SET_ENVIRONMENT_AMOUNT> : public SAudioObjectRequestDataInternalBase
{
	explicit SAudioObjectRequestDataInternal(SAudioObjectRequestData<eAORT_SET_ENVIRONMENT_AMOUNT> const* const pAORData)
		: SAudioObjectRequestDataInternalBase(pAORData->eType)
		, nEnvironmentID(pAORData->nEnvironmentID)
		, fAmount(pAORData->fAmount) 
	{}

	TAudioEnvironmentID const	nEnvironmentID;
	float const								fAmount;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestDataInternal<eAORT_RESET_ENVIRONMENTS> : public SAudioObjectRequestDataInternalBase
{
	explicit SAudioObjectRequestDataInternal(SAudioObjectRequestData<eAORT_RESET_ENVIRONMENTS> const* const pAORData)
		: SAudioObjectRequestDataInternalBase(pAORData->eType)
	{}
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestDataInternal<eAORT_RELEASE_OBJECT> : public SAudioObjectRequestDataInternalBase
{
	explicit SAudioObjectRequestDataInternal(SAudioObjectRequestData<eAORT_RELEASE_OBJECT> const* const pAORData)
		: SAudioObjectRequestDataInternalBase(pAORData->eType)
	{}
};

//////////////////////////////////////////////////////////////////////////
struct SAudioListenerRequestDataInternalBase : public SAudioRequestDataInternal
{
	explicit SAudioListenerRequestDataInternalBase(EAudioListenerRequestType const eALRType)
		: SAudioRequestDataInternal(eART_AUDIO_LISTENER_REQUEST)
		, eType(eALRType)
	{}

	EAudioListenerRequestType const eType;

private:

	SAudioListenerRequestDataInternalBase();
};

//////////////////////////////////////////////////////////////////////////
template<EAudioListenerRequestType T>
struct SAudioListenerRequestDataInternal : public SAudioListenerRequestDataInternalBase
{
	SAudioListenerRequestDataInternal()
		: SAudioListenerRequestDataInternalBase(T)
	{}
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioListenerRequestDataInternal<eALRT_SET_POSITION> : public SAudioListenerRequestDataInternalBase
{
	explicit SAudioListenerRequestDataInternal(SAudioListenerRequestData<eALRT_SET_POSITION> const* const pALRData)
		: SAudioListenerRequestDataInternalBase(pALRData->eType)
		, oNewPosition(pALRData->oNewPosition)
	{}

	SATLWorldPosition	const oNewPosition;
};

SAudioRequestDataInternal* ConvertToInternal(SAudioRequestDataBase const* const pExternalData);

class CAudioRequestInternal 
{
public:

	CAudioRequestInternal()
		: nFlags(eARF_NONE)
		, nAudioObjectID(INVALID_AUDIO_OBJECT_ID)
		, pOwner(NPTR)
		, sValue(NPTR)
		, eStatus(eARS_NONE)
		, nInternalInfoFlags(eARIF_NONE)
		, pData(NPTR)
	{}

	CAudioRequestInternal(SAudioRequest const& rExternalRequest)
		: nFlags(rExternalRequest.nFlags)
		, nAudioObjectID(rExternalRequest.nAudioObjectID)
		, pOwner(rExternalRequest.pOwner)
		, sValue(rExternalRequest.sValue)
		, eStatus(eARS_NONE)
		, nInternalInfoFlags(eARIF_NONE)
		, pData(ConvertToInternal(rExternalRequest.pData))
	{}


	~CAudioRequestInternal() {}

	uint32																nFlags;
	TAudioObjectID												nAudioObjectID;
	void*																	pOwner;
	char const*														sValue;
	EAudioRequestStatus										eStatus;
	uint32																nInternalInfoFlags;
	_smart_ptr<SAudioRequestDataInternal>	pData;
};

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
// Filter for drawing debug info to the screen
enum EAudioDebugDrawFilter ATL_ENUM_TYPE
{
	eADDF_ALL														= 0,
	eADDF_DRAW_SPHERES									= BIT(6),  // a
	eADDF_SHOW_OBJECT_LABEL							= BIT(7),  // b
	eADDF_SHOW_OBJECT_TRIGGERS					= BIT(8),  // c
	eADDF_SHOW_OBJECT_STATES						= BIT(9),  // d
	eADDF_SHOW_OBJECT_RTPCS							= BIT(10), // e
	eADDF_SHOW_OBJECT_ENVIRONMENTS			= BIT(11), // f
	eADDF_DRAW_OBSTRUCTION_RAYS					= BIT(12), // g
	eADDF_SHOW_OBSTRUCTION_RAY_LABELS		= BIT(13), // h

	eADDF_SHOW_ACTIVE_EVENTS						= BIT(27), // v
	eADDF_SHOW_ACTIVE_OBJECTS						= BIT(28), // w
	eADDF_SHOW_FILECACHE_MANAGER_INFO		= BIT(29), // x
};
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
#endif // INTERNAL_INTERFACES_H_INCLUDED
