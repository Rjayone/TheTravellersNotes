// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#ifndef SOUND_CVARS_H_INCLUDED
#define SOUND_CVARS_H_INCLUDED

#include <IAudioSystem.h>

class CSoundCVars
{
public:

	CSoundCVars();
	~CSoundCVars();

	void	RegisterVariables();
	void	UnregisterVariables();

	int			m_nATLPoolSize;
	int			m_nFileCacheManagerSize;
	int			m_nAudioObjectPoolSize;
	int			m_nAudioEventPoolSize;
	int			m_nAudioProxiesInitType;

	ICVar*	m_cvAudioSystemImplementationName;

	float		m_fOcclusionMaxDistance;
	float		m_fOcclusionMaxSyncDistance;
	float		m_fFullObstructionMaxDistance;
	float		m_fPositionUpdateThreshold;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	int			m_nIgnoreWindowFocus;
	int			m_nDrawAudioDebug;
	int			m_nFileCacheManagerDebugFilter;
	int			m_nAudioLoggingOptions;
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

private:

	static void CmdExecuteTrigger(IConsoleCmdArgs* pCmdArgs);
	static void CmdStopTrigger(IConsoleCmdArgs* pCmdArgs);
	static void CmdSetRtpc(IConsoleCmdArgs* pCmdArgs);
	static void CmdSetSwitchState(IConsoleCmdArgs* pCmdArgs);

	CSoundCVars(CSoundCVars const&);						// Copy protection
	CSoundCVars& operator=(CSoundCVars const&);	// Copy protection
};

extern CSoundCVars g_SoundCVars;

#endif // SOUND_CVARS_H_INCLUDED
