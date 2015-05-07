// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#if !defined(AFX_STDAFX_H__9793C644_C91F_4BA6_A176_44537782901A__INCLUDED_)
#define AFX_STDAFX_H__9793C644_C91F_4BA6_A176_44537782901A__INCLUDED_

#pragma warning( disable:4786 ) 

#include <CryModuleDefs.h>
//#define eCryModule eCryM_SoundSystem
//#define RWI_NAME_TAG "RayWorldIntersection(Audio)"
//#define PWI_NAME_TAG "PrimitiveWorldIntersection(Audio)"

#if !defined(_RELEASE)
#define INCLUDE_AUDIO_PRODUCTION_CODE
#define ENABLE_AUDIO_LOGGING
#endif // _RELEASE

#include <platform.h>
#include <StlUtils.h>
#include <ProjectDefines.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <SoundAllocator.h>
#include <AudioLogger.h>

extern CSoundAllocator g_MemoryPoolPrimary;
extern CAudioLogger g_AudioLogger;

#define AUDIO_ALLOCATOR_MEMORY_POOL g_MemoryPoolPrimary

#include <STLSoundAllocator.h>
#include <AudioInternalInterfaces.h>

// Win32 or Win64 or Durango
//////////////////////////////////////////////////////////////////////////
#if (defined(WIN32) || defined(WIN64) || defined(DURANGO)) && !defined(XENON) && !defined(PS3)
	#include <windows.h>
#endif

// Win32
//////////////////////////////////////////////////////////////////////////
#if defined(WIN32) && !defined(WIN64) && !defined(XENON) && !defined(PS3) && !defined(DURANGO) && !defined(ORBIS) && !defined(CAFE)	
#endif // Win32

// Win64 (Note: WIN32 is automatically defined as well!)
//////////////////////////////////////////////////////////////////////////
#if defined(WIN32) && defined(WIN64) && !defined(XENON) && !defined(PS3) && !defined(DURANGO) && !defined(ORBIS) && !defined(CAFE)	
#endif // Win64

// DURANGO
//////////////////////////////////////////////////////////////////////////
#if defined(DURANGO)
//#include <xdk.h>
#endif // DURANGO

//////////////////////////////////////////////////////////////////////////
#if defined(ORBIS)
#endif // ORBIS

// CAFE
//////////////////////////////////////////////////////////////////////////
#if defined(CAFE)
#endif // CAFE

// Mac
//////////////////////////////////////////////////////////////////////////
#if defined(MAC)
#undef SOUNDSYSTEM_USE_XENON_XAUDIO
#endif

// Android
//////////////////////////////////////////////////////////////////////////
#if defined(ANDROID)
#define SOUNDSYSTEM_USE_FMODEX400
#undef SOUNDSYSTEM_USE_XENON_XAUDIO
// Android does not have a fmodeventnet library!
// #undef INCLUDE_AUDIO_PRODUCTION_CODE << To be reviewed
#endif

// iOS
//////////////////////////////////////////////////////////////////////////
#if defined(IOS)
//#define SOUNDSYSTEM_USE_FMODEX400
#undef INCLUDE_AUDIO_PRODUCTION_CODE
#endif

// Linux

//#include <memory>
//#include <map>
//#include <vector>
//#include <set>
//#include <algorithm>
//
//#include <ILog.h>
//#include <IConsole.h>
//#include <ISound.h>
//#include <ISystem.h>
//
//#include <Cry_Math.h>
//#include <ICryPak.h>


//structure which updates update time
//struct SUpdateTimer
//{
//#ifdef ENABLE_LW_PROFILERS
//	ILINE SUpdateTimer(uint32& updateTimeDst) : rUpdateTime(updateTimeDst)
//	{
//		QueryPerformanceCounter(&startTicks);
//	}
//	ILINE ~SUpdateTimer()
//	{
//		LARGE_INTEGER endTicks;
//		QueryPerformanceCounter(&endTicks);
//		rUpdateTime += (uint32)(endTicks.QuadPart - startTicks.QuadPart);
//	}
//	LARGE_INTEGER startTicks;
//	uint32 &rUpdateTime;
//	static float ConvertToMS(uint32 ticks)
//	{
//		LARGE_INTEGER freq; 
//		QueryPerformanceFrequency(&freq); 
//		float frequency = 1.f/(float)freq.QuadPart;
//		return (float)ticks * 1000.f * frequency;
//	}
//#else
//	ILINE SUpdateTimer(uint32& updateTimeDst){}
//	static float ConvertToMS(uint32 ticks){return 0.f;}
//#endif
//};
//#define UPDATE_TIMER SUpdateTimer t(m_UpdateTicks);

#endif // !defined(AFX_STDAFX_H__9793C644_C91F_4BA6_A176_44537782901A__INCLUDED_)
