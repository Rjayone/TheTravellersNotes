// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#ifndef AUDIO_PROXY_H_INCLUDED
#define AUDIO_PROXY_H_INCLUDED

#include <IAudioSystem.h>

class CAudioProxy : public IAudioProxy, public SAudioEventListener
{
public:

	CAudioProxy();
	virtual ~CAudioProxy();

	// IAudioProxy							
	VIRTUAL void								Initialize(char const* const sObjectName, bool const bInitAsync = true);
	VIRTUAL void								Release();
	VIRTUAL void								Reset();
	VIRTUAL void								ExecuteTrigger(TAudioControlID const nTriggerID, ELipSyncMethod const eLipSyncMethod);
	VIRTUAL void								ExecuteTrigger(
																TAudioControlID const nTriggerID,
																ELipSyncMethod const eLipSyncMethod,
																TTriggerFinishedCallback const pCallback,
																void* const pCallbackCookie);
	VIRTUAL void								StopTrigger(TAudioControlID const nTriggerID);
	VIRTUAL void								SetSwitchState(TAudioControlID const nSwitchID, TAudioSwitchStateID const nStateID);
	VIRTUAL void								SetRtpcValue(TAudioControlID const nRtpcID, float const fValue);
	VIRTUAL void								SetObstructionCalcType(EAudioObjectObstructionCalcType const eObstructionType);
	VIRTUAL void								SetPosition(SATLWorldPosition const& rPosition);
	VIRTUAL void								SetPosition(Vec3 const& rPosition);
	VIRTUAL void								SetEnvironmentAmount(TAudioEnvironmentID const nEnvironmentID, float const fValue);
	VIRTUAL void								SetCurrentEnvironments(EntityId const nEntityToIgnore);
	VIRTUAL void								SetLipSyncProvider(ILipSyncProvider* const pILipSyncProvider);
	// ~IAudioProxy

	// SAudioEventListener
	static void									OnAudioEvent(SAudioRequestInfo const* const pAudioRequestInfo);
	// ~SAudioEventListener

	void												ClearEnvironments();
	void												ExecuteQueuedCommands();

	static TAudioControlID			s_nObstructionCalcSwitchID;
	static TAudioSwitchStateID	s_aObstructionCalcStateIDs[eAOOCT_COUNT];

private:

	static size_t const					sMaxAreas = 10;

	TAudioObjectID							m_nAudioObjectID;
	SATLWorldPosition						m_oPosition;

	TATLEnumFlagsType						m_nFlags;

	ILipSyncProviderPtr					m_oLipSyncProvider;
	ELipSyncMethod							m_eCurrentLipSyncMethod;
	TAudioControlID							m_nCurrentLipSyncID;

	enum EQueuedAudioCommandType ATL_ENUM_TYPE
	{
		eQACT_NONE											= 0,
		eQACT_EXECUTE_TRIGGER						= 1,
		eQACT_STOP_TRIGGER							= 2,
		eQACT_SET_SWITCH_STATE					= 3,
		eQACT_SET_RTPC_VALUE						= 4,
		eQACT_SET_POSITION							= 5,
		eQACT_SET_ENVIRONMENT_AMOUNT		= 6,
		eQACT_SET_CURRENT_ENVIRONMENTS	= 7,
		eQACT_CLEAR_ENVIRONMENTS				= 8,
		eQACT_RESET											= 9,
		eQACT_RELEASE										= 10,
		eQACT_INITIALIZE								= 11,
	};

	struct SQueuedAudioCommand
	{
		SQueuedAudioCommand(EQueuedAudioCommandType const ePassedType)
			:	eType(ePassedType)
			,	nTriggerID(INVALID_AUDIO_CONTROL_ID)
			,	nSwitchID(INVALID_AUDIO_CONTROL_ID)
			,	nStateID(INVALID_AUDIO_SWITCH_STATE_ID)
			,	nRtpcID(INVALID_AUDIO_CONTROL_ID)
			,	fValue(0.0f)
			,	nEnvironmentID(INVALID_AUDIO_ENVIRONMENT_ID)
			,	sValue(NPTR)
			, pCallback(NPTR)
			, pCallbackCookie(NPTR)
			, eLipSyncMethod(eLSM_None)
			,	nEntityID(0)
		{}

		SQueuedAudioCommand& operator=(SQueuedAudioCommand const& rOther)
		{	
			const_cast<EQueuedAudioCommandType&>(eType) = rOther.eType;
			nTriggerID			= rOther.nTriggerID;
			nSwitchID				= rOther.nSwitchID;
			nStateID				= rOther.nStateID;
			nRtpcID					= rOther.nRtpcID;
			fValue					= rOther.fValue;
			nEnvironmentID	= rOther.nEnvironmentID;
			sValue					= rOther.sValue;
			oPosition				= rOther.oPosition;
			pCallback				= rOther.pCallback;
			pCallbackCookie	= rOther.pCallbackCookie;
			eLipSyncMethod	= rOther.eLipSyncMethod;
			nEntityID				= rOther.nEntityID;

			return *this;
		}

		EQueuedAudioCommandType const	eType;
		TAudioControlID								nTriggerID;
		TAudioControlID								nSwitchID;
		TAudioSwitchStateID						nStateID;
		TAudioControlID								nRtpcID;
		float													fValue;
		TAudioEnvironmentID						nEnvironmentID;
		string												sValue;
		SATLWorldPosition							oPosition;
		TTriggerFinishedCallback			pCallback;
		void*													pCallbackCookie;
		ELipSyncMethod								eLipSyncMethod;
		EntityId											nEntityID;
	};

	enum EAudioProxyFlags ATL_ENUM_TYPE
	{
		eAPF_NONE						= 0,
		eAPF_WAITING_FOR_ID	= BIT(0),
	};

	typedef std::deque<SQueuedAudioCommand, STLSoundAllocator<SQueuedAudioCommand> > TQueuedAudioCommands;
	TQueuedAudioCommands m_aQueuedAudioCommands;

	void	TryAddQueuedCommand(SQueuedAudioCommand const& rCommand);
	void	ExecuteTriggerInternal(
		TAudioControlID const nTriggerID,
		ELipSyncMethod const eLipSyncMethod,
		TTriggerFinishedCallback const pCallback = NPTR,
		void* const pCallbackCookie = NPTR);
};

#endif // AUDIO_PROXY_H_INCLUDED
