// CryEngine Source File.
// Copyright (C), Crytek, 1999-2014.

#include "stdafx.h"
#include "SoundCVars.h"
#include <ISystem.h>
#include <IConsole.h>

//////////////////////////////////////////////////////////////////////////
void OnCVarChangedAudioSystemCallback(ICVar* pCVar)
{
	if (gEnv->pAudioSystem != NPTR)
	{
		gEnv->pAudioSystem->OnCVarChanged(pCVar);
	}
}

//////////////////////////////////////////////////////////////////////////
CSoundCVars::CSoundCVars()
	:	m_nATLPoolSize(0)
	, m_nFileCacheManagerSize(0)
	,	m_nAudioObjectPoolSize(0)
	,	m_nAudioEventPoolSize(0)
	,	m_nAudioProxiesInitType(0)
	, m_fOcclusionMaxDistance(0.0f)
	, m_fOcclusionMaxSyncDistance(0.0f)
	, m_fFullObstructionMaxDistance(0.0f)
	,	m_fPositionUpdateThreshold(0.0f)
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	, m_nIgnoreWindowFocus(0)
	, m_nDrawAudioDebug(0)
	,	m_nFileCacheManagerDebugFilter(0)
	, m_nAudioLoggingOptions(0)
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
{
}

//////////////////////////////////////////////////////////////////////////
CSoundCVars::~CSoundCVars()
{
}

//////////////////////////////////////////////////////////////////////////
void CSoundCVars::RegisterVariables()
{
#if defined(WIN32) || defined(WIN64)
	m_nATLPoolSize											= 8<<10;		// 8 MiB on PC
	m_nFileCacheManagerSize							= 384<<10;	// 384 MiB on PC
	m_nAudioObjectPoolSize							=	512;
	m_nAudioEventPoolSize								=	256;
	m_nAudioProxiesInitType							= 0;
	m_fOcclusionMaxDistance							= 500.0f;
	m_fOcclusionMaxSyncDistance					= 10.0f;
	m_fFullObstructionMaxDistance				= 5.0f;
	m_fPositionUpdateThreshold					= 0.1f;
#elif defined(DURANGO)
	m_nATLPoolSize											= 8<<10;		// 8 MiB on XboxOne
	m_nFileCacheManagerSize							= 384<<10;	// 384 MiB on XboxOne
	m_nAudioObjectPoolSize							=	512;
	m_nAudioEventPoolSize								=	256;
	m_nAudioProxiesInitType							= 0;
	m_fOcclusionMaxDistance							= 500.0f;
	m_fOcclusionMaxSyncDistance					= 10.0f;
	m_fFullObstructionMaxDistance				= 5.0f;
	m_fPositionUpdateThreshold					= 0.1f;
#elif defined(ORBIS)
	m_nATLPoolSize											= 8<<10;		// 8 MiB on PS4
	m_nFileCacheManagerSize							= 384<<10;	// 384 MiB on PS4
	m_nAudioObjectPoolSize							=	512;
	m_nAudioEventPoolSize								=	256;
	m_nAudioProxiesInitType							= 0;
	m_fOcclusionMaxDistance							= 500.0f;
	m_fOcclusionMaxSyncDistance					= 10.0f;
	m_fFullObstructionMaxDistance				= 5.0f;
	m_fPositionUpdateThreshold					= 0.1f;
#elif defined(MAC)
	m_nATLPoolSize											= 8<<10;		// 8 MiB on MAC
	m_nFileCacheManagerSize							= 384<<10;	// 384 MiB on MAC
	m_nAudioObjectPoolSize							=	512;
	m_nAudioEventPoolSize								=	256;
	m_nAudioProxiesInitType							= 0;
	m_fOcclusionMaxDistance							= 500.0f;
	m_fOcclusionMaxSyncDistance					= 10.0f;
	m_fFullObstructionMaxDistance				= 5.0f;
	m_fPositionUpdateThreshold					= 0.1f;
#elif defined(LINUX)
	m_nATLPoolSize											= 8<<10;		// 8 MiB on Linux
	m_nFileCacheManagerSize							= 384<<10;	// 384 MiB on Linux
	m_nAudioObjectPoolSize							=	512;
	m_nAudioEventPoolSize								=	256;
	m_nAudioProxiesInitType							= 0;
	m_fOcclusionMaxDistance							= 500.0f;
	m_fOcclusionMaxSyncDistance					= 10.0f;
	m_fFullObstructionMaxDistance				= 5.0f;
	m_fPositionUpdateThreshold					= 0.1f;
#elif defined(CAFE)
	m_nATLPoolSize											= 8<<10;		// 8 MiB on WiiU
	m_nFileCacheManagerSize							= 128<<10;	// 128 MiB on WiiU
	m_nAudioObjectPoolSize							=	256;
	m_nAudioEventPoolSize								=	128;
	m_nAudioProxiesInitType							= 0;
	m_fOcclusionMaxDistance							= 500.0f;
	m_fOcclusionMaxSyncDistance					= 10.0f;
	m_fFullObstructionMaxDistance				= 5.0f;
	m_fPositionUpdateThreshold					= 0.1f;
#elif defined(IOS)
	m_nATLPoolSize											= 8<<10;		// 8 MiB on IOS because it is SMALL
	m_nFileCacheManagerSize							= 2<<10;		// 2 MiB on IOS because it is SMALL
	m_nAudioObjectPoolSize							=	128;
	m_nAudioEventPoolSize								=	64;
	m_nAudioProxiesInitType							= 0;
	m_fOcclusionMaxDistance							= 500.0f;
	m_fOcclusionMaxSyncDistance					= 10.0f;
	m_fFullObstructionMaxDistance				= 5.0f;
	m_fPositionUpdateThreshold					= 0.1f;
#elif defined(ANDROID)
	m_nATLPoolSize											= 8<<10;		// 8 MiB on ANDROID
	m_nFileCacheManagerSize							= 2<<10;		// 2 MiB on ANDROID
	m_nAudioObjectPoolSize							=	128;
	m_nAudioEventPoolSize								=	64;
	m_nAudioProxiesInitType							= 0;
	m_fOcclusionMaxDistance							= 500.0f;
	m_fOcclusionMaxSyncDistance					= 10.0f;
	m_fFullObstructionMaxDistance				= 5.0f;
	m_fPositionUpdateThreshold					= 0.1f;
#else
#error "Undefined platform."
#endif

	m_cvAudioSystemImplementationName = REGISTER_STRING("s_AudioSystemImplementationName", "CryAudioImplWwise", VF_REQUIRE_APP_RESTART,
		"Holds the name of the AudioSystemImplementation library to be used.\n"
		"Usage: s_AudioSystemImplementationName <name of the library without extension>\n"
		"Default: CryAudioImplWwise\n");

	REGISTER_CVAR2("s_ATLPoolSize", &m_nATLPoolSize, m_nATLPoolSize, VF_REQUIRE_APP_RESTART, 
		"Specifies the size (in KiB) of the memory pool to be used by the ATL.\n"
		"Usage: s_ATLPoolSize [0/...]\n"
		"Default: 8192 (8 MiB)\n");

	REGISTER_CVAR2("s_OcclusionMaxDistance", &m_fOcclusionMaxDistance, m_fOcclusionMaxDistance, VF_CHEAT|VF_CHEAT_NOCHECK,
		"Obstruction/Occlusion is not calculated for the sounds, whose distance to the listener is greater than this value. Setting this value to 0 disables obstruction/occlusion calculations.\n"
		"Usage: s_OcclusionMaxDistance [0/...]\n" 
		"Default: 500 m\n");

	REGISTER_CVAR2("s_OcclusionMaxSyncDistance", &m_fOcclusionMaxSyncDistance, m_fOcclusionMaxSyncDistance, VF_CHEAT|VF_CHEAT_NOCHECK,
		"Physics rays are processed synchronously for the sounds that are closer to the listener than this value, and asynchronously for the rest (possible performance optimization).\n"
		"Usage: s_OcclusionMaxSyncDistance [0/...]\n" 
		"Default: 10 m\n");

	REGISTER_CVAR2("s_FullObstructionMaxDistance", &m_fFullObstructionMaxDistance, m_fFullObstructionMaxDistance, VF_CHEAT|VF_CHEAT_NOCHECK,
		"for the sounds, whose distance to the listener is greater than this value, the obstruction is value gets attenuated with distance.\n"
		"Usage: s_FullObstructionMaxDistance [0/...]\n" 
		"Default: 5 m\n");

	REGISTER_CVAR2("s_PositionUpdateThreshold", &m_fPositionUpdateThreshold, m_fPositionUpdateThreshold, VF_CHEAT|VF_CHEAT_NOCHECK,
		"An audio object has to move by at least this amount to issue a position update request to the audio system.\n"
		"This kind of optimization should ideally be done by the parent system so this is here for convenience."
		"Usage: s_PositionUpdateThreshold [0/...]\n" 
		"Default: 0.1 (10 cm)\n");

	REGISTER_CVAR2("s_FileCacheManagerSize", &m_nFileCacheManagerSize, m_nFileCacheManagerSize, VF_REQUIRE_APP_RESTART,
		"Sets the size in KiB the AFCM will allocate on the heap.\n"
		"Usage: s_FileCacheManagerSize [0/...]\n" 
		"Default PC: 393216 (384 MiB), XboxOne: 393216 (384 MiB), PS4: 393216 (384 MiB), MAC: 393216 (384 MiB), Linux: 393216 (384 MiB), WiiU: 131072 (128 MiB)\n");

	REGISTER_CVAR2("s_AudioObjectPoolSize", &m_nAudioObjectPoolSize, m_nAudioObjectPoolSize, VF_REQUIRE_APP_RESTART,
		"Sets the number of preallocated audio objects and corresponding audio proxies.\n"
		"Usage: s_AudioObjectPoolSize [0/...]\n" 
		"Default PC: 512, XboxOne: 512, PS4: 512, MAC: 512, Linux: 512, WiiU: 256 IOS: 128 Android: 128\n");

	REGISTER_CVAR2("s_AudioEventPoolSize", &m_nAudioEventPoolSize, m_nAudioEventPoolSize, VF_REQUIRE_APP_RESTART,
		"Sets the number of preallocated audio events.\n"
		"Usage: s_AudioEventPoolSize [0/...]\n" 
		"Default PC: 256, XboxOne: 256, PS4: 256, MAC: 256, Linux: 256, WiiU: 128 IOS: 64 Android: 64\n");

	REGISTER_CVAR2("s_AudioProxiesInitType", &m_nAudioProxiesInitType, m_nAudioProxiesInitType, VF_NULL,
		"Can override AudioProxies' init type on a global scale.\n"
		"If set it determines whether AudioProxies initialize synchronously or asynchronously.\n"
		"This is a performance type cvar as asynchronously initializing AudioProxies\n"
		"will have a greatly reduced impact on the calling thread.\n"
		"Be aware though that when set to initialize asynchronously that audio will play back delayed.\n"
		"By how much will greatly depend on the audio thread's work load.\n"
		"0: AudioProxy specific initialization.\n"
		"1: All AudioProxies initialize synchronously.\n"
		"2: All AudioProxies initialize asynchronously.\n"
		"Usage: s_AudioProxiesInitType [0/1/2]\n"
		"Default PC: 0, XboxOne: 0, PS4: 0, MAC: 0, Linux: 0, WiiU: 0 iOS: 0 Android: 0\n");

	REGISTER_COMMAND("s_ExecuteTrigger", CmdExecuteTrigger, VF_CHEAT,
		"Execute an Audio Trigger.\n"
		"The first argument is the name of the AudioTrigger to be executed, the second argument is an optional AudioObject ID.\n"
		"If the second argument is provided, the AudioTrigger is executed on the AudioObject with the given ID,\n"
		"otherwise, the AudioTrigger is executed on the GlobalAudioObject\n"
		"Usage: s_ExecuteTrigger Play_chicken_idle 605 or s_ExecuteTrigger MuteDialog\n");

	REGISTER_COMMAND("s_StopTrigger", CmdStopTrigger, VF_CHEAT,
		"Execute an Audio Trigger.\n"
		"The first argument is the name of the AudioTrigger to be stopped, the second argument is an optional AudioObject ID.\n"
		"If the second argument is provided, the AudioTrigger is stopped on the AudioObject with the given ID,\n"
		"otherwise, the AudioTrigger is stopped on the GlobalAudioObject\n"
		"Usage: s_StopTrigger Play_chicken_idle 605 or s_StopTrigger MuteDialog\n");

	REGISTER_COMMAND("s_SetRtpc", CmdSetRtpc, VF_CHEAT,
		"Set an Audio RTPC value.\n"
		"The first argument is the name of the AudioRtpc to be set, the second argument is the float value to be set,"
		"the third argument is an optional AudioObject ID.\n"
		"If the third argument is provided, the AudioRtpc is set on the AudioObject with the given ID,\n"
		"otherwise, the AudioRtpc is set on the GlobalAudioObject\n"
		"Usage: s_SetRtpc character_speed  0.0  601 or s_SetRtpc volume_music 1.0\n");

	REGISTER_COMMAND("s_SetSwitchState", CmdSetSwitchState, VF_CHEAT,
		"Set an Audio Switch to a provided State.\n"
		"The first argument is the name of the AudioSwitch to, the second argument is the name of the SwitchState to be set,"
		"the third argument is an optional AudioObject ID.\n"
		"If the third argument is provided, the AudioSwitch is set on the AudioObject with the given ID,\n"
		"otherwise, the AudioSwitch is set on the GlobalAudioObject\n"
		"Usage: s_SetSwitchState SurfaceType concrete 601 or s_SetSwitchState weather rain\n");

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	REGISTER_CVAR2("s_IgnoreWindowFocus", &m_nIgnoreWindowFocus, 0, VF_DEV_ONLY,
		"If set to 1, the sound system will continue playing when the Editor or Game window loses focus.\n"
		"Usage: s_IgnoreWindowFocus [0/1]\n" 
		"Default: 0 (off)\n");

	REGISTER_CVAR2("s_DrawAudioDebug", &m_nDrawAudioDebug, 0, VF_CHEAT|VF_CHEAT_NOCHECK|VF_BITFIELD,
		"Draws AudioTranslationLayer related debug data to the screen.\n"
		"Usage: s_DrawAudioDebug [0ab...] (flags can be combined)\n"
		"0: No audio debug info on the screen.\n"
		"a: Draw spheres around active audio objects.\n"
		"b: Show text labels for active audio objects.\n"
		"c: Show trigger names for active audio objects.\n"
		"d: Show current states for active audio objects.\n"
		"e: Show RTPC values for active audio objects.\n"
		"f: Show Environment amounts for active audio objects.\n"
		"g: Draw occlusion rays.\n"
		"h: Show occlusion ray labels.\n"
		"v: List active Events.\n"
		"w: List active Audio Objects.\n"
		"x: Show FileCache Manager debug info.\n"
		);

	REGISTER_CVAR2("s_FileCacheManagerDebugFilter", &m_nFileCacheManagerDebugFilter, 0, VF_CHEAT|VF_CHEAT_NOCHECK|VF_BITFIELD,
		"Allows for filtered display of the different AFCM entries such as Globals, Level Specifics, Game Hints and so on.\n"
		"Usage: s_FileCacheManagerDebugFilter [0ab...] (flags can be combined)\n"
		"Default: 0 (all)\n"
		"a: Globals\n"
		"b: Level Specifics\n"
		"c: Game Hints\n");

	REGISTER_CVAR2("s_AudioLoggingOptions", &m_nAudioLoggingOptions, 0, VF_CHEAT|VF_CHEAT_NOCHECK|VF_BITFIELD,
		"Toggles the logging of audio related messages.\n"
		"Usage: s_AudioLoggingOptions [0ab...] (flags can be combined)\n"
		"Default: 0 (none)\n"
		"a: Errors\n"
		"b: Warnings\n"
		"c: Comments\n");

#endif // INCLUDE_AUDIO_PRODUCTION_CODE
}

//////////////////////////////////////////////////////////////////////////
void CSoundCVars::UnregisterVariables()
{
	IConsole* const pConsole = gEnv->pConsole;
	assert(pConsole);

	pConsole->UnregisterVariable("s_AudioSystemImplementationName");
	pConsole->UnregisterVariable("s_ATLPoolSize");
	pConsole->UnregisterVariable("s_OcclusionMaxDistance");
	pConsole->UnregisterVariable("s_OcclusionMaxSyncDistance");
	pConsole->UnregisterVariable("s_FullObstructionMaxDistance");
	pConsole->UnregisterVariable("s_PositionUpdateThreshold");
	pConsole->UnregisterVariable("s_FileCacheManagerSize");
	pConsole->UnregisterVariable("s_AudioObjectPoolSize");
	pConsole->UnregisterVariable("s_AudioEventPoolSize");
	pConsole->UnregisterVariable("s_AudioProxiesInitType");

	pConsole->UnregisterVariable("s_ExecuteTrigger");
	pConsole->UnregisterVariable("s_StopTrigger");
	pConsole->UnregisterVariable("s_SetRtpc");
	pConsole->UnregisterVariable("s_SetSwitchState");

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	pConsole->UnregisterVariable("s_IgnoreWindowFocus");
	pConsole->UnregisterVariable("s_DrawAudioDebug");
	pConsole->UnregisterVariable("s_FileCacheManagerDebugFilter");
	pConsole->UnregisterVariable("s_AudioLoggingOptions");
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
}

//////////////////////////////////////////////////////////////////////////
void CSoundCVars::CmdExecuteTrigger(IConsoleCmdArgs* pCmdArgs)
{
	TAudioControlID nTriggerID = INVALID_AUDIO_CONTROL_ID;
	TAudioObjectID nObjectID = INVALID_AUDIO_OBJECT_ID;

	int const nArgCount = pCmdArgs->GetArgCount();

	if ((2 <= nArgCount) || (nArgCount <= 3))
	{
		gEnv->pAudioSystem->GetAudioTriggerID(pCmdArgs->GetArg(1), nTriggerID);

		if (nArgCount == 3)
		{
			int const nTempID = atoi(pCmdArgs->GetArg(2));
			if (nTempID > 0)
			{
				nObjectID = static_cast<TAudioObjectID>(nTempID);
			}
			else
			{
				g_AudioLogger.Log(eALT_ERROR, "Invalid Object ID: %s", pCmdArgs->GetArg(2));
			}
		}

		if (nTriggerID == INVALID_AUDIO_CONTROL_ID)
		{
			g_AudioLogger.Log(eALT_ERROR, "Unknown trigger name: %s", pCmdArgs->GetArg(1));
		}
		else
		{
			SAudioRequest oRequest;
			SAudioObjectRequestData<eAORT_EXECUTE_TRIGGER> oRequestData(nTriggerID, 0.0f);

			oRequest.nAudioObjectID = nObjectID;
			oRequest.nFlags = eARF_PRIORITY_NORMAL;
			oRequest.pData = &oRequestData;

			gEnv->pAudioSystem->PushRequest(oRequest);
		}
	}
	else
	{
		g_AudioLogger.Log(eALT_ERROR,"Usage: s_ExecuteTrigger [TriggerName] [[Optional Object ID]]");
	}
}

//////////////////////////////////////////////////////////////////////////
void CSoundCVars::CmdStopTrigger(IConsoleCmdArgs* pCmdArgs)
{
	TAudioControlID nTriggerID = INVALID_AUDIO_CONTROL_ID;
	TAudioObjectID nObjectID = INVALID_AUDIO_OBJECT_ID;

	int const nArgCount = pCmdArgs->GetArgCount();

	if ((2 <= nArgCount) || (nArgCount <= 3))
	{
		gEnv->pAudioSystem->GetAudioTriggerID(pCmdArgs->GetArg(1), nTriggerID);

		if (nArgCount == 3)
		{
			int const nTempID = atoi(pCmdArgs->GetArg(2));
			if (nTempID > 0)
			{
				nObjectID = static_cast<TAudioObjectID>(nTempID);
			}
			else
			{
				g_AudioLogger.Log(eALT_ERROR, "Invalid Object ID: %s", pCmdArgs->GetArg(2));
			}
		}

		if (nTriggerID == INVALID_AUDIO_CONTROL_ID)
		{
			g_AudioLogger.Log(eALT_ERROR, "Unknown trigger name: %s", pCmdArgs->GetArg(1));
		}
		else
		{
			SAudioRequest oRequest;
			SAudioObjectRequestData<eAORT_STOP_TRIGGER> oRequestData(nTriggerID);

			oRequest.nAudioObjectID = nObjectID;
			oRequest.nFlags = eARF_PRIORITY_NORMAL;
			oRequest.pData = &oRequestData;

			gEnv->pAudioSystem->PushRequest(oRequest);
		}
	}
	else
	{
		g_AudioLogger.Log(eALT_ERROR,"Usage: s_StopTrigger [TriggerName] [[Optional Object ID]]");
	}
}

//////////////////////////////////////////////////////////////////////////
void CSoundCVars::CmdSetRtpc(IConsoleCmdArgs* pCmdArgs)
{
	TAudioControlID nRtpcID = INVALID_AUDIO_CONTROL_ID;
	TAudioObjectID nObjectID = INVALID_AUDIO_OBJECT_ID;
	double fValue = 0.0f;

	int const nArgCount = pCmdArgs->GetArgCount();

	if ((3 <= nArgCount) || (nArgCount <= 4))
	{
		gEnv->pAudioSystem->GetAudioRtpcID(pCmdArgs->GetArg(1), nRtpcID);

		fValue = atof(pCmdArgs->GetArg(2));

		if (nArgCount == 4)
		{
			int const nTempID = atoi(pCmdArgs->GetArg(3));
			if (nTempID > 0)
			{
				nObjectID = static_cast<TAudioObjectID>(nTempID);
			}
			else
			{
				g_AudioLogger.Log(eALT_ERROR, "Invalid Object ID: %s", pCmdArgs->GetArg(3));
				return;
			}
		}

		if (nRtpcID == INVALID_AUDIO_CONTROL_ID)
		{
			g_AudioLogger.Log(eALT_ERROR, "Unknown Rtpc name: %s", pCmdArgs->GetArg(1));
		}
		else
		{
			SAudioRequest oRequest;
			SAudioObjectRequestData<eAORT_SET_RTPC_VALUE> oRequestData(nRtpcID, static_cast<float>(fValue));

			oRequest.nAudioObjectID = nObjectID;
			oRequest.nFlags = eARF_PRIORITY_NORMAL;
			oRequest.pData = &oRequestData;

			gEnv->pAudioSystem->PushRequest(oRequest);
		}
	}
	else
	{
		g_AudioLogger.Log(eALT_ERROR,"Usage: s_SetRtpc [RtpcName] [RtpcValue] [[Optional Object ID]]");
	}
}

//////////////////////////////////////////////////////////////////////////
void CSoundCVars::CmdSetSwitchState(IConsoleCmdArgs* pCmdArgs)
{
	TAudioControlID nSwitchID = INVALID_AUDIO_CONTROL_ID;
	TAudioObjectID nObjectID = INVALID_AUDIO_OBJECT_ID;
	TAudioSwitchStateID nSwitchStateID = INVALID_AUDIO_SWITCH_STATE_ID;

	int const nArgCount = pCmdArgs->GetArgCount();

	if ((3 <= nArgCount) || (nArgCount <= 4))
	{
		gEnv->pAudioSystem->GetAudioSwitchID(pCmdArgs->GetArg(1), nSwitchID);

		if (nSwitchID != INVALID_AUDIO_CONTROL_ID)
		{
			gEnv->pAudioSystem->GetAudioSwitchStateID(nSwitchID, pCmdArgs->GetArg(2), nSwitchStateID);

			if (nArgCount == 4)
			{	
				int const nTempID = atoi(pCmdArgs->GetArg(3));
				if (nTempID > 0)
				{
					nObjectID = static_cast<TAudioObjectID>(nTempID);
				}
				else
				{
					g_AudioLogger.Log(eALT_ERROR, "Invalid Object ID: %s", pCmdArgs->GetArg(3));
					return;
				}
			}

			if (nSwitchStateID != INVALID_AUDIO_SWITCH_STATE_ID)
			{
				SAudioRequest oRequest;
				SAudioObjectRequestData<eAORT_SET_SWITCH_STATE> oRequestData(nSwitchID, nSwitchStateID);

				oRequest.nAudioObjectID = nObjectID;
				oRequest.nFlags = eARF_PRIORITY_NORMAL;
				oRequest.pData = &oRequestData;

				gEnv->pAudioSystem->PushRequest(oRequest);
			}
			else
			{
				g_AudioLogger.Log(eALT_ERROR, "Invalid  Switch State name: %s", pCmdArgs->GetArg(2));
			}
		}
		else
		{
			g_AudioLogger.Log(eALT_ERROR, "Unknown Switch name: %s", pCmdArgs->GetArg(1));
		}
	}
	else
	{
		g_AudioLogger.Log(eALT_ERROR,"Usage: s_SetSwitchState [SwitchName] [SwitchStateName] [[Optional Object ID]]");
	}
}
