// CryEngine Source File.
// Copyright (C), Crytek, 1999-2014.

#include "stdafx.h"
#include "SoundCVars.h"
#include "AudioSystem.h"
#include "AudioSystemImpl_NULL.h"
#include <IAudioSystem.h>
#include <platform_impl.h>
#include <IEngineModule.h>
#include <BucketAllocatorImpl.h>
#include <CryExtension/ICryFactory.h>
#include <CryExtension/Impl/ClassWeaver.h>

#if defined(DURANGO)
#include <apu.h>
#endif // DURANGO

// Define global objects.
CSoundCVars		g_SoundCVars;
CAudioLogger	g_AudioLogger;
CSoundAllocator g_MemoryPoolPrimary;

//////////////////////////////////////////////////////////////////////////
class CSystemEventListner_Sound : public ISystemEventListener
{
public:

	virtual void OnSystemEvent( ESystemEvent event,UINT_PTR wparam,UINT_PTR lparam )
	{
		switch (event)
		{
		case ESYSTEM_EVENT_LEVEL_LOAD_START:
			{
				g_MemoryPoolPrimary.Cleanup();

				break;
			}
		case ESYSTEM_EVENT_LEVEL_POST_UNLOAD:
			{
				STLALLOCATOR_CLEANUP;
				g_MemoryPoolPrimary.Cleanup();

				break;
			}
		case ESYSTEM_EVENT_RANDOM_SEED:
			{
				g_random_generator.Seed(gEnv->bNoRandomSeed?0:(uint32)wparam);

				break;
			}
		case ESYSTEM_EVENT_ACTIVATE: 
			{
				// When Alt+Tabbing out of the application while it's in fullscreen mode 
				// ESYSTEM_EVENT_ACTIVATE is sent instead of ESYSTEM_EVENT_CHANGE_FOCUS.

				// wparam != 0 is active, wparam == 0 is inactive
				// lparam != 0 is minimized, lparam == 0 is not minimized

				if (wparam == 0 || lparam != 0)
				{
					//lost focus
					if (gEnv->pAudioSystem != NPTR)
					{
						gEnv->pAudioSystem->PushRequest(m_oLoseFocusRequest);
					}
				}
				else
				{
					// got focus
					if (gEnv->pAudioSystem != NPTR)
					{
						gEnv->pAudioSystem->PushRequest(m_oGetFocusRequest);
					}
				}

				break;
			}
		case ESYSTEM_EVENT_CHANGE_FOCUS:
			{
				// wparam != 0 is focused, wparam == 0 is not focused
				if (wparam == 0)
				{
					// lost focus
					if (gEnv->pAudioSystem != NPTR)
					{
						gEnv->pAudioSystem->PushRequest(m_oLoseFocusRequest);
					}
				}
				else
				{
					// got focus
					if (gEnv->pAudioSystem != NPTR)
					{
						gEnv->pAudioSystem->PushRequest(m_oGetFocusRequest);
					}
				}

				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void InitRequestData()
	{
		if (gEnv->pAudioSystem != NPTR)
		{
			m_oLoseFocusRequest.nFlags	= eARF_PRIORITY_HIGH;
			m_oLoseFocusRequest.pData		= &m_oAMLoseFocusData;

			m_oGetFocusRequest.nFlags	= eARF_PRIORITY_HIGH;
			m_oGetFocusRequest.pData	= &m_oAMGetFocusData;
		}
	}

private:
	
	SAudioRequest m_oLoseFocusRequest;
	SAudioRequest m_oGetFocusRequest;
	SAudioManagerRequestData<eAMRT_LOSE_FOCUS> m_oAMLoseFocusData;
	SAudioManagerRequestData<eAMRT_GET_FOCUS> m_oAMGetFocusData;
};

static CSystemEventListner_Sound g_system_event_listener_sound;


///////////////////////////////////////////////////////////////////////////
bool CreateAudioSystem(SSystemGlobalEnvironment& rEnv)
{
	bool bSuccess = false;

	// The AudioSystem must be the first object that is allocated from the audio memory pool after it has been created and the last that is freed from it!
	POOL_NEW_CREATE(CAudioSystem, pAudioSystem);

	if (pAudioSystem != NPTR)
	{
		//release the old AudioSystem
		if (rEnv.pAudioSystem != NPTR)
		{
			rEnv.pAudioSystem->Release();
			rEnv.pAudioSystem = NPTR;
		}

		rEnv.pAudioSystem = static_cast<IAudioSystem*>(pAudioSystem);
		bSuccess = true;
	}
	else
	{
		g_AudioLogger.Log(eALT_ERROR, "Could not create an instance of CAudioSystem! Keeping the default AudioSystem!\n");
	}

	return bSuccess;
} 

///////////////////////////////////////////////////////////////////////////
void PrepareAudioSystem(IAudioSystem* const pAudioSystem)
{
	// Must be blocking requests.
	SAudioRequest oAudioRequestData;
	oAudioRequestData.nFlags = eARF_PRIORITY_HIGH | eARF_EXECUTE_BLOCKING;

	SAudioManagerRequestData<eAMRT_INIT_AUDIO_SYSTEM> oAMData;
	oAudioRequestData.pData = &oAMData;
	pAudioSystem->PushRequest(oAudioRequestData);

	CryFixedStringT<MAX_AUDIO_FILE_PATH_LENGTH> sTemp(pAudioSystem->GetConfigPath());

	SAudioManagerRequestData<eAMRT_PARSE_CONTROLS_DATA> oAMData2(sTemp.c_str(), eADS_GLOBAL);
	oAudioRequestData.pData = &oAMData2;
	pAudioSystem->PushRequest(oAudioRequestData);

	SAudioManagerRequestData<eAMRT_PARSE_PRELOADS_DATA> oAMData3(sTemp.c_str(), eADS_GLOBAL);
	oAudioRequestData.pData = &oAMData3;
	pAudioSystem->PushRequest(oAudioRequestData);

	SAudioManagerRequestData<eAMRT_PRELOAD_SINGLE_REQUEST> oAMData4(SATLInternalControlIDs::nGlobalPreloadRequestID);
	oAudioRequestData.pData = &oAMData4;
	gEnv->pAudioSystem->PushRequest(oAudioRequestData);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// dll interface

void AddPhysicalBlock(long size)
{
#ifndef _LIB
	_CryMemoryManagerPoolHelper::FakeAllocation( size );
#else
	GetISystem()->GetIMemoryManager()->FakeAllocation( size );
#endif
}

//////////////////////////////////////////////////////////////////////////
class CEngineModule_CrySoundSystem : public IEngineModule
{
	CRYINTERFACE_SIMPLE(IEngineModule)
	CRYGENERATE_SINGLETONCLASS(CEngineModule_CrySoundSystem, "EngineModule_CrySoundSystem", 0xec73cf4362ca4a7f, 0x8b451076dc6fdb8b)

	virtual const char* GetName() {return "CrySoundSystem";}
	virtual const char* GetCategory() {return "CryEngine";}

	//////////////////////////////////////////////////////////////////////////
	virtual bool Initialize(SSystemGlobalEnvironment &env, const SSystemInitParams &initParams )
	{
		bool bSuccess = false;

		// initialize memory pools
		MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "ATL Memory Pool");
		size_t const nPrimarySize		= g_SoundCVars.m_nATLPoolSize << 10;
		uint8* const pPrimaryMemory	= new uint8[nPrimarySize];
		g_MemoryPoolPrimary.InitMem(nPrimarySize, pPrimaryMemory, "ATL Memory Pool");

		if (CreateAudioSystem(env))
		{
#if defined(DURANGO)
			// Do this before initializing the audio middleware!
			HRESULT const eResult = ApuCreateHeap(static_cast<UINT32>(g_SoundCVars.m_nFileCacheManagerSize << 10));

			if (eResult != S_OK)
			{
				CryFatalError("<Audio>: AudioSystem failed to allocate APU heap! (%d byte)", g_SoundCVars.m_nFileCacheManagerSize << 10);
			}
#endif // DURANGO

			if (!env.pSystem->InitializeEngineModule(
				g_SoundCVars.m_cvAudioSystemImplementationName->GetString(),
				"AudioModule_CryAudioImpl",
				initParams,
				false,
				true))
			{
				CryLogAlways("<Audio>: Failed to initialize the AudioSystemImplementation %s. Will run with AudioSystemImpl_NULL.",
					g_SoundCVars.m_cvAudioSystemImplementationName->GetString());

				POOL_NEW_CREATE(CAudioSystemImpl_NULL,pAudioSystemImpl_NULL);

				env.pAudioSystem->Initialize(pAudioSystemImpl_NULL);
			}

			PrepareAudioSystem(env.pAudioSystem);

			g_system_event_listener_sound.InitRequestData();
			env.pSystem->GetISystemEventDispatcher()->RegisterListener(&g_system_event_listener_sound);

			bSuccess = true;
		}
		else
		{
			g_AudioLogger.Log(eALT_ERROR, "Could not create AudioSystem!");
		}

		return bSuccess;
	}
};

CRYREGISTER_SINGLETON_CLASS(CEngineModule_CrySoundSystem)

//////////////////////////////////////////////////////////////////////////
CEngineModule_CrySoundSystem::CEngineModule_CrySoundSystem()
{
	// Register audio cvars
	g_SoundCVars.RegisterVariables();
}

//////////////////////////////////////////////////////////////////////////
CEngineModule_CrySoundSystem::~CEngineModule_CrySoundSystem()
{
}

#include <CrtDebugStats.h>
