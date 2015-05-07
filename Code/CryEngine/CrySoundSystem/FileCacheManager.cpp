// CryEngine Source File.
// Copyright (C), Crytek, 1999-2014.

#include "stdafx.h"
#include "FileCacheManager.h"
#include "SoundCVars.h"
#include <CustomMemoryHeap.h>
#include <IAudioSystemImplementation.h>
#include <IRenderer.h>
#include <IRenderAuxGeom.h>

//////////////////////////////////////////////////////////////////////////
CFileCacheManager::CFileCacheManager(TATLPreloadRequestLookup& rPreloadRequests)
	:	m_rPreloadRequests(rPreloadRequests)
	,	m_nCurrentByteTotal(0)
	,	m_nMaxByteTotal(0)
	,	m_pImpl(NPTR)
{
}

//////////////////////////////////////////////////////////////////////////
CFileCacheManager::~CFileCacheManager()
{
	// Empty on purpose
}

//////////////////////////////////////////////////////////////////////////
void CFileCacheManager::Init(IAudioSystemImplementation* const pImpl)
{
	m_pImpl = pImpl;
	AllocateHeap(static_cast<size_t>(g_SoundCVars.m_nFileCacheManagerSize), "AudioFileCacheManager");
}

//////////////////////////////////////////////////////////////////////////
void CFileCacheManager::Release()
{
	m_pImpl = NPTR;
}

//////////////////////////////////////////////////////////////////////////
void CFileCacheManager::Update()
{
	// Not used for now as we do not queue entries!
}

//////////////////////////////////////////////////////////////////////////
void CFileCacheManager::AllocateHeap(size_t const nSize, char const* const sUsage)
{
	if (nSize > 0)
	{
#if defined(DURANGO)
		m_pMemoryHeap = static_cast<CCustomMemoryHeap*>(gEnv->pSystem->GetIMemoryManager()->CreateCustomMemoryHeapInstance(IMemoryManager::eapAPU));
#else
		m_pMemoryHeap = static_cast<CCustomMemoryHeap*>(gEnv->pSystem->GetIMemoryManager()->CreateCustomMemoryHeapInstance(IMemoryManager::eapCustomAlignment));
#endif // DURANGO

		if (m_pMemoryHeap.get() != NPTR)
		{
			m_nMaxByteTotal = nSize << 10;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
TAudioFileEntryID CFileCacheManager::TryAddFileCacheEntry(XmlNodeRef const pFileNode, EATLDataScope const eDataScope, bool const bAutoLoad)
{
	TAudioFileEntryID nID = INVALID_AUDIO_FILE_ENTRY_ID;

	SATLAudioFileEntryInfo oFileEntryInfo;
	
	if (m_pImpl->ParseAudioFileEntry(pFileNode, &oFileEntryInfo) == eARS_SUCCESS)
	{
		CryFixedStringT<MAX_AUDIO_FILE_PATH_LENGTH> sFullPath(m_pImpl->GetAudioFileLocation(&oFileEntryInfo));
		sFullPath += oFileEntryInfo.sFileName;
		POOL_NEW_CREATE(CATLAudioFileEntry, pNewAudioFileEntry)(sFullPath, oFileEntryInfo.pImplData);

		if (pNewAudioFileEntry != NPTR)
		{
			pNewAudioFileEntry->m_nMemoryBlockAlignment = oFileEntryInfo.nMemoryBlockAlignment;

			if (oFileEntryInfo.bLocalized) 
			{
				pNewAudioFileEntry->m_nFlags |= eAFF_LOCALIZED;
			}

			nID = static_cast<TAudioFileEntryID>(AudioStringToID(oFileEntryInfo.sFileName));
			CATLAudioFileEntry* const __restrict pExisitingAudioFileEntry = stl::find_in_map(m_cAudioFileEntries, nID, NPTR);

			if (pExisitingAudioFileEntry == NPTR)
			{
				if (!bAutoLoad)
				{
					// Can now be ref-counted and therefore manually unloaded.
					pNewAudioFileEntry->m_nFlags |= eAFF_USE_COUNTED;
				}

				pNewAudioFileEntry->m_eDataScope = eDataScope;
				pNewAudioFileEntry->m_sPath.MakeLower();
				size_t const nFileSize = gEnv->pCryPak->FGetSize(pNewAudioFileEntry->m_sPath.c_str());

				if (nFileSize > 0)
				{
					pNewAudioFileEntry->m_nSize						= nFileSize;
					pNewAudioFileEntry->m_nFlags					= (pNewAudioFileEntry->m_nFlags | eAFF_NOTCACHED) & ~eAFF_NOTFOUND;
					pNewAudioFileEntry->m_eStreamTaskType	= eStreamTaskTypeSound;
				}

				m_cAudioFileEntries[nID] = pNewAudioFileEntry;
			}
			else
			{
				if ((pExisitingAudioFileEntry->m_nFlags & eAFF_USE_COUNTED) != 0 && bAutoLoad)
				{
					// This file entry is upgraded from "manual loading" to "auto loading" but needs a reset to "manual loading" again!
					pExisitingAudioFileEntry->m_nFlags = (pExisitingAudioFileEntry->m_nFlags | eAFF_NEEDS_RESET_TO_MANUAL_LOADING) & ~eAFF_USE_COUNTED;
					g_AudioLogger.Log(eALT_ALWAYS, "Upgraded file entry from \"manual loading\" to \"auto loading\": %s", pExisitingAudioFileEntry->m_sPath.c_str());
				}

				// Entry already exists, free the memory!
				m_pImpl->DeleteAudioFileEntryData(pNewAudioFileEntry->m_pImplData);
				POOL_FREE(pNewAudioFileEntry);
			}
		}
	}
	
	return nID;
}

//////////////////////////////////////////////////////////////////////////
bool CFileCacheManager::TryRemoveFileCacheEntry(TAudioFileEntryID const nAudioFileID, EATLDataScope const eDataScope)
{
	bool bSuccess = false;
	TAudioFileEntries::iterator Iter(m_cAudioFileEntries.find(nAudioFileID));
	TAudioFileEntries::const_iterator const IterEnd(m_cAudioFileEntries.end());

	if (Iter != IterEnd)
	{
		CATLAudioFileEntry* const pAudioFileEntry = Iter->second;

		if (pAudioFileEntry->m_eDataScope == eDataScope)
		{
			if ((pAudioFileEntry->m_nFlags & (eAFF_CACHED | eAFF_LOADING)) == 0)
			{
				m_pImpl->DeleteAudioFileEntryData(pAudioFileEntry->m_pImplData);
				POOL_FREE(pAudioFileEntry);
				m_cAudioFileEntries.erase(Iter);
			}
			else 
			{
				g_AudioLogger.Log(eALT_ALWAYS, "Trying to remove a cached or loading file cache entry %s", pAudioFileEntry->m_sPath.c_str());
			}
		}
		else if ((eDataScope == eADS_LEVEL_SPECIFIC) && ((pAudioFileEntry->m_nFlags & eAFF_NEEDS_RESET_TO_MANUAL_LOADING) != 0))
		{
			pAudioFileEntry->m_nFlags = (pAudioFileEntry->m_nFlags | eAFF_USE_COUNTED) & ~eAFF_NEEDS_RESET_TO_MANUAL_LOADING;
			g_AudioLogger.Log(eALT_ALWAYS, "Downgraded file entry from \"auto loading\" to \"manual loading\": %s", pAudioFileEntry->m_sPath.c_str());
		}
	}

	return bSuccess;
}

//////////////////////////////////////////////////////////////////////////
void CFileCacheManager::UpdateLocalizedFileCacheEntries()
{
	TAudioFileEntries::iterator Iter(m_cAudioFileEntries.begin());
	TAudioFileEntries::const_iterator const IterEnd(m_cAudioFileEntries.end());

	for (; Iter != IterEnd; ++Iter)
	{
		CATLAudioFileEntry* const pAudioFileEntry = Iter->second;

		if (pAudioFileEntry != NPTR && (pAudioFileEntry->m_nFlags & eAFF_LOCALIZED) != 0)
		{
			if ((pAudioFileEntry->m_nFlags & (eAFF_CACHED | eAFF_LOADING)) != 0)
			{
				// The file needs to be unloaded first.
				size_t const nUseCount = pAudioFileEntry->m_nUseCount;
				pAudioFileEntry->m_nUseCount = 0;// Needed to uncache without an error.
				UncacheFile(pAudioFileEntry);

				UpdateLocalizedFileEntryData(pAudioFileEntry);

				TryCacheFileCacheEntryInternal(pAudioFileEntry, Iter->first, true, true, nUseCount);
			}
			else
			{
				// The file is not cached or loading, it is safe to update the corresponding CATLAudioFileEntry data.
				UpdateLocalizedFileEntryData(pAudioFileEntry);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CFileCacheManager::TryLoadRequest(TAudioPreloadRequestID const nPreloadRequestID, bool const bLoadSynchronously, bool const bAutoLoadOnly)
{
	bool bFullSuccess = false;
	bool bFullFailure = true;
	CATLPreloadRequest* const pPreloadRequest = stl::find_in_map(m_rPreloadRequests, nPreloadRequestID, NPTR);

	if (pPreloadRequest != NPTR && !pPreloadRequest->m_cFileEntryIDs.empty() && (!bAutoLoadOnly || (bAutoLoadOnly && pPreloadRequest->m_bAutoLoad)))
	{
		bFullSuccess = true;
		CATLPreloadRequest::TFileEntryIDs::const_iterator Iter(pPreloadRequest->m_cFileEntryIDs.begin());
		CATLPreloadRequest::TFileEntryIDs::const_iterator const IterEnd(pPreloadRequest->m_cFileEntryIDs.end());

		for (; Iter != IterEnd; ++Iter)
		{
			TAudioFileEntryID const nFileID = (*Iter);
			CATLAudioFileEntry* const pAudioFileEntry = stl::find_in_map(m_cAudioFileEntries, nFileID, NPTR);

			if (pAudioFileEntry != NPTR)
			{
				bool const bTemp = TryCacheFileCacheEntryInternal(pAudioFileEntry, nFileID, bLoadSynchronously);
				bFullSuccess = bFullSuccess && bTemp;
				bFullFailure = bFullFailure && !bTemp;
			}
		}
	}

	return bFullSuccess ? eARS_SUCCESS : bFullFailure ? eARS_FAILURE : eARS_PARTIAL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CFileCacheManager::TryUnloadRequest(TAudioPreloadRequestID const nPreloadRequestID)
{
	bool bFullSuccess = false;
	bool bFullFailure = true;
	CATLPreloadRequest* const pPreloadRequest = stl::find_in_map(m_rPreloadRequests, nPreloadRequestID, NPTR);

	if (pPreloadRequest != NPTR && !pPreloadRequest->m_cFileEntryIDs.empty())
	{
		bFullSuccess = true;
		CATLPreloadRequest::TFileEntryIDs::const_iterator Iter(pPreloadRequest->m_cFileEntryIDs.begin());
		CATLPreloadRequest::TFileEntryIDs::const_iterator const IterEnd(pPreloadRequest->m_cFileEntryIDs.end());

		for (; Iter != IterEnd; ++Iter)
		{
			TAudioFileEntryID const nFileID = (*Iter);
			CATLAudioFileEntry* const pAudioFileEntry = stl::find_in_map(m_cAudioFileEntries, nFileID, NPTR);

			if (pAudioFileEntry != NPTR)
			{
				bool const bTemp = UncacheFileCacheEntryInternal(pAudioFileEntry, true);
				bFullSuccess = bFullSuccess && bTemp;
				bFullFailure = bFullFailure && !bTemp;
			}
		}
	}

	return bFullSuccess ? eARS_SUCCESS : bFullFailure ? eARS_FAILURE : eARS_PARTIAL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CFileCacheManager::UnloadDataByScope(EATLDataScope const eDataScope)
{
	TAudioFileEntries::iterator Iter(m_cAudioFileEntries.begin());
	TAudioFileEntries::const_iterator const IterEnd(m_cAudioFileEntries.end());

	while (Iter != IterEnd)
	{
		CATLAudioFileEntry* const pAudioFileEntry = Iter->second;

		if (pAudioFileEntry != NPTR && pAudioFileEntry->m_eDataScope == eDataScope)
		{
			if (UncacheFileCacheEntryInternal(pAudioFileEntry, true, true))
			{
				m_cAudioFileEntries.erase(Iter++);
				continue;
			}
		}

		++Iter;
	}

	return eARS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
bool CFileCacheManager::UncacheFileCacheEntryInternal(CATLAudioFileEntry* const pAudioFileEntry, bool const bNow, bool const bIgnoreUsedCount /* = false */)
{
	bool bSuccess = false;

	// In any case decrement the used count.
	if (pAudioFileEntry->m_nUseCount > 0)
	{
		--pAudioFileEntry->m_nUseCount;
	}

	if (pAudioFileEntry->m_nUseCount < 1 || bIgnoreUsedCount)
	{
		// Must be cached to proceed.
		if ((pAudioFileEntry->m_nFlags & eAFF_CACHED) != 0)
		{
			// Only "use-counted" files can become removable!
			if ((pAudioFileEntry->m_nFlags & eAFF_USE_COUNTED) != 0)
			{
				pAudioFileEntry->m_nFlags |= eAFF_REMOVABLE;
			}

			if (bNow || bIgnoreUsedCount)
			{
				UncacheFile(pAudioFileEntry);
			}
		}
		else if ((pAudioFileEntry->m_nFlags & (eAFF_LOADING|eAFF_MEMALLOCFAIL)) != 0)
		{
			// Reset the entry in case it's still loading or was a memory allocation fail.
			UncacheFile(pAudioFileEntry);
		}

		// The file was either properly uncached, queued for uncache or not cached at all.
		bSuccess = true;
	}

	return bSuccess;
}

//////////////////////////////////////////////////////////////////////////
void CFileCacheManager::StreamAsyncOnComplete(IReadStream* pStream, unsigned int nError)
{
	// We "user abort" quite frequently so this is not something we want to assert on.
	assert(nError == 0 || nError == ERROR_USER_ABORT);

	FinishStreamInternal(pStream, nError);
}

//////////////////////////////////////////////////////////////////////////
void CFileCacheManager::DrawDebugInfo(IRenderer* const pRenderer, float const fPosX, float const fPosY)
{
#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	if (((g_SoundCVars.m_nDrawAudioDebug & eADDF_SHOW_FILECACHE_MANAGER_INFO) != 0) && (pRenderer != NPTR))
	{
		EATLDataScope eDataScope = eADS_ALL;

		if ((g_SoundCVars.m_nFileCacheManagerDebugFilter & eAFCMDF_ALL) == 0)
		{
			if ((g_SoundCVars.m_nFileCacheManagerDebugFilter & eAFCMDF_GLOBALS) != 0)
			{
				eDataScope = eADS_GLOBAL;
			}
			else if ((g_SoundCVars.m_nFileCacheManagerDebugFilter & eAFCMDF_LEVEL_SPECIFICS) != 0)
			{
				eDataScope = eADS_LEVEL_SPECIFIC;
			}
		}

		CTimeValue const tFrameTime = gEnv->pTimer->GetAsyncTime();

		CryFixedStringT<512> sTemp;
		float const fEntryDrawSize  = 1.1f;
		float const fEntryStepSize  = 12.0f;
		float fPositionY            = fPosY + 20.0f;
		float fPositionX						= fPosX + 20.0f;
		float fTime                 = 0.0f;
		float fRatio                = 0.0f;
		float fOriginalAlpha        = 0.7f;
		float* pfColor              = NPTR;

		// The colors.
		float fWhite[4]		= {1.0f, 1.0f, 1.0f, fOriginalAlpha};
		float fCyan[4]		= {0.0f, 1.0f, 1.0f, fOriginalAlpha};
		float fOrange[4]	= {1.0f, 0.5f, 0.0f, fOriginalAlpha};
		float fGreen[4]		= {0.0f, 1.0f, 0.0f, fOriginalAlpha};
		float fRed[4]			= {1.0f, 0.0f, 0.0f, fOriginalAlpha};
		float fRedish[4]	= {0.7f, 0.0f, 0.0f, fOriginalAlpha};
		float fBlue[4]		= {0.1f, 0.2f, 0.8f, fOriginalAlpha};
		float fYellow[4]	= {1.0f, 1.0f, 0.0f, fOriginalAlpha};
		float fDarkish[4]	= {0.3f, 0.3f, 0.3f, fOriginalAlpha};

		pRenderer->Draw2dLabel(fPosX, fPositionY, 1.6f, fOrange, false, "FileCacheManager (%d of %d KiB) [Entries: %d]", static_cast<int>(m_nCurrentByteTotal >> 10), static_cast<int>(m_nMaxByteTotal >> 10), static_cast<int>(m_cAudioFileEntries.size()));
		fPositionY += 15.0f;

		TAudioFileEntries::const_iterator Iter(m_cAudioFileEntries.begin());
		TAudioFileEntries::const_iterator const IterEnd(m_cAudioFileEntries.end());

		for (; Iter != IterEnd; ++Iter)
		{
			CATLAudioFileEntry* const pAudioFileEntry = Iter->second;

			if (pAudioFileEntry->m_eDataScope == eADS_GLOBAL &&
				((g_SoundCVars.m_nFileCacheManagerDebugFilter == eAFCMDF_ALL) ||
				(g_SoundCVars.m_nFileCacheManagerDebugFilter & eAFCMDF_GLOBALS) != 0 ||
				((g_SoundCVars.m_nFileCacheManagerDebugFilter & eAFCMDF_USE_COUNTED) != 0 &&
				(pAudioFileEntry->m_nFlags & eAFF_USE_COUNTED) != 0)))
			{
				if ((pAudioFileEntry->m_nFlags & eAFF_LOADING) != 0)
				{
					pfColor = fRed;
				}
				else if ((pAudioFileEntry->m_nFlags & eAFF_MEMALLOCFAIL) != 0)
				{
					pfColor = fBlue;
				}
				else if ((pAudioFileEntry->m_nFlags & eAFF_REMOVABLE) != 0)
				{
					pfColor = fGreen;
				}
				else if ((pAudioFileEntry->m_nFlags & eAFF_NOTCACHED) != 0)
				{
					pfColor = fWhite;
				}
				else if ((pAudioFileEntry->m_nFlags & eAFF_NOTFOUND) != 0)
				{
					pfColor = fRedish;
				}
				else
				{
					pfColor = fCyan;
				}
				
				fTime          = (tFrameTime - pAudioFileEntry->m_oTimeCached).GetSeconds();
				fRatio         = fTime / 5.0f;
				fOriginalAlpha = pfColor[3];
				pfColor[3]     *= clamp_tpl(fRatio, 0.2f, 1.0f);

				sTemp.clear();

				if ((pAudioFileEntry->m_nFlags & eAFF_USE_COUNTED) != 0)
				{
					if (pAudioFileEntry->m_nSize < 1024)
					{
						sTemp.Format(sTemp + "%s (%" PRISIZE_T " Byte) [%" PRISIZE_T "]", pAudioFileEntry->m_sPath.c_str(), pAudioFileEntry->m_nSize, pAudioFileEntry->m_nUseCount);
					}
					else
					{
						sTemp.Format(sTemp + "%s (%" PRISIZE_T " KiB) [%" PRISIZE_T "]", pAudioFileEntry->m_sPath.c_str(), pAudioFileEntry->m_nSize >> 10, pAudioFileEntry->m_nUseCount);
					}
				}
				else
				{
					if (pAudioFileEntry->m_nSize < 1024)
					{
						sTemp.Format(sTemp + "%s (%" PRISIZE_T " Byte)", pAudioFileEntry->m_sPath.c_str(), pAudioFileEntry->m_nSize);
					}
					else
					{
						sTemp.Format(sTemp + "%s (%" PRISIZE_T " KiB)", pAudioFileEntry->m_sPath.c_str(), pAudioFileEntry->m_nSize >> 10);
					}
				}
				
				pRenderer->Draw2dLabel(fPositionX, fPositionY, fEntryDrawSize, pfColor, false, sTemp.c_str());
				pfColor[3] = fOriginalAlpha;
				fPositionY += fEntryStepSize;
			}
		}

		Iter = m_cAudioFileEntries.begin();

		for (; Iter != IterEnd; ++Iter)
		{
			CATLAudioFileEntry* const pAudioFileEntry = Iter->second;

			if (pAudioFileEntry->m_eDataScope == eADS_LEVEL_SPECIFIC &&
				((g_SoundCVars.m_nFileCacheManagerDebugFilter == eAFCMDF_ALL) ||
				(g_SoundCVars.m_nFileCacheManagerDebugFilter & eAFCMDF_LEVEL_SPECIFICS) != 0 ||
				((g_SoundCVars.m_nFileCacheManagerDebugFilter & eAFCMDF_USE_COUNTED) != 0 &&
				(pAudioFileEntry->m_nFlags & eAFF_USE_COUNTED) != 0)))
			{
				if ((pAudioFileEntry->m_nFlags & eAFF_LOADING) != 0)
				{
					pfColor = fRed;
				}
				else if ((pAudioFileEntry->m_nFlags & eAFF_MEMALLOCFAIL) != 0)
				{
					pfColor = fBlue;
				}
				else if ((pAudioFileEntry->m_nFlags & eAFF_REMOVABLE) != 0)
				{
					pfColor = fGreen;
				}
				else if ((pAudioFileEntry->m_nFlags & eAFF_NOTCACHED) != 0)
				{
					pfColor = fWhite;
				}
				else if ((pAudioFileEntry->m_nFlags & eAFF_NOTFOUND) != 0)
				{
					pfColor = fRedish;
				}
				else
				{
					pfColor = fYellow;
				}

				fTime          = (tFrameTime - pAudioFileEntry->m_oTimeCached).GetSeconds();
				fRatio         = fTime / 5.0f;
				fOriginalAlpha = pfColor[3];
				pfColor[3]     *= clamp_tpl(fRatio, 0.2f, 1.0f);

				sTemp.clear();

				if ((pAudioFileEntry->m_nFlags & eAFF_USE_COUNTED) != 0)
				{
					if (pAudioFileEntry->m_nSize < 1024)
					{
						sTemp.Format(sTemp + "%s (%" PRISIZE_T " Byte) [%" PRISIZE_T "]", pAudioFileEntry->m_sPath.c_str(), pAudioFileEntry->m_nSize, pAudioFileEntry->m_nUseCount);
					}
					else
					{
						sTemp.Format(sTemp + "%s (%" PRISIZE_T " KiB) [%" PRISIZE_T "]", pAudioFileEntry->m_sPath.c_str(), pAudioFileEntry->m_nSize >> 10, pAudioFileEntry->m_nUseCount);
					}
				}
				else
				{
					if (pAudioFileEntry->m_nSize < 1024)
					{
						sTemp.Format(sTemp + "%s (%" PRISIZE_T " Byte)", pAudioFileEntry->m_sPath.c_str(), pAudioFileEntry->m_nSize);
					}
					else
					{
						sTemp.Format(sTemp + "%s (%" PRISIZE_T " KiB)", pAudioFileEntry->m_sPath.c_str(), pAudioFileEntry->m_nSize >> 10);
					}
				}

				pRenderer->Draw2dLabel(fPositionX, fPositionY, fEntryDrawSize, pfColor, false, sTemp.c_str());
				pfColor[3] = fOriginalAlpha;
				fPositionY += fEntryStepSize;
			}

		}
	}
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
}

//////////////////////////////////////////////////////////////////////////
bool CFileCacheManager::DoesRequestFitInternal(size_t const nRequestSize)
{
	// Make sure these unsigned values don't flip around.
	assert(m_nCurrentByteTotal <= m_nMaxByteTotal);
	bool bSuccess = false;

	if (nRequestSize <= (m_nMaxByteTotal - m_nCurrentByteTotal))
	{
		// Here the requested size is available without the need of first cleaning up.
		bSuccess = true;
	}
	else
	{
		// Determine how much memory would get freed if all eAFF_REMOVABLE files get thrown out.
		// We however skip files that are already queued for unload. The request will get queued up in that case.
		size_t nPossibleMemoryGain = 0;

		// Check the single file entries for removability.
		TAudioFileEntries::const_iterator Iter(m_cAudioFileEntries.begin());
		TAudioFileEntries::const_iterator const IterEnd(m_cAudioFileEntries.end());

		for (; Iter != IterEnd; ++Iter)
		{
			CATLAudioFileEntry* const pAudioFileEntry = Iter->second;

			if (pAudioFileEntry != NPTR &&
				(pAudioFileEntry->m_nFlags & eAFF_CACHED) != 0 &&
				(pAudioFileEntry->m_nFlags & eAFF_REMOVABLE) != 0)
			{
				nPossibleMemoryGain += pAudioFileEntry->m_nSize;
			}
		}

		size_t const nMaxAvailableSize = (m_nMaxByteTotal - (m_nCurrentByteTotal - nPossibleMemoryGain));

		if (nRequestSize <= nMaxAvailableSize)
		{
			// Here we need to cleanup first before allowing the new request to be allocated.
			TryToUncacheFiles();

			// We should only indicate success if there's actually really enough room for the new entry!
			bSuccess = (m_nMaxByteTotal - m_nCurrentByteTotal) >= nRequestSize;
		}
	}

	return bSuccess;
}

//////////////////////////////////////////////////////////////////////////
bool CFileCacheManager::FinishStreamInternal(IReadStreamPtr const pStream, int unsigned const nError)
{
	bool bSuccess = false;

	TAudioFileEntryID const nFileID = static_cast<TAudioFileEntryID>(pStream->GetUserData());
	CATLAudioFileEntry* const pAudioFileEntry = stl::find_in_map(m_cAudioFileEntries, nFileID, NPTR);
	assert(pAudioFileEntry != NPTR);

	// Must be loading in to proceed.
	if (pAudioFileEntry != NPTR && (pAudioFileEntry->m_nFlags & eAFF_LOADING) != 0)
	{
		if (nError == 0)
		{
			pAudioFileEntry->m_pReadStream	= NPTR;
			pAudioFileEntry->m_nFlags				= (pAudioFileEntry->m_nFlags | eAFF_CACHED) & ~(eAFF_LOADING|eAFF_NOTCACHED);

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
			pAudioFileEntry->m_oTimeCached = gEnv->pTimer->GetAsyncTime();
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

			SATLAudioFileEntryInfo sFileEntryInfo;
			sFileEntryInfo.nMemoryBlockAlignment = pAudioFileEntry->m_nMemoryBlockAlignment;
			sFileEntryInfo.pFileData = pAudioFileEntry->m_pMemoryBlock->GetData();
			sFileEntryInfo.nSize = pAudioFileEntry->m_nSize;
			sFileEntryInfo.pImplData = pAudioFileEntry->m_pImplData;
			sFileEntryInfo.sFileName = PathUtil::GetFile(pAudioFileEntry->m_sPath.c_str());

			m_pImpl->RegisterInMemoryFile(&sFileEntryInfo);
			bSuccess = true;
		}
		else if (nError == ERROR_USER_ABORT)
		{
			// We abort this stream only during entry Uncache().
			// Therefore there's no need to call Uncache() during stream abort with error code ERROR_USER_ABORT.
			g_AudioLogger.Log(eALT_ALWAYS, "AFCM: user aborted stream for file %s (error: %u)", pAudioFileEntry->m_sPath.c_str(), nError);
		}
		else
		{
			UncacheFileCacheEntryInternal(pAudioFileEntry, true, true);
			g_AudioLogger.Log(eALT_ERROR, "AFCM: failed to stream in file %s (error: %u)", pAudioFileEntry->m_sPath.c_str(), nError);
		}
	}

	return bSuccess;
}

//////////////////////////////////////////////////////////////////////////
bool CFileCacheManager::AllocateMemoryBlockInternal(CATLAudioFileEntry* const __restrict pAudioFileEntry)
{
	// Must not have valid memory yet.
	assert(pAudioFileEntry->m_pMemoryBlock.get() == NPTR);

	if (m_pMemoryHeap.get() != NPTR)
	{
		pAudioFileEntry->m_pMemoryBlock = m_pMemoryHeap->AllocateBlock(pAudioFileEntry->m_nSize, pAudioFileEntry->m_sPath.c_str(), pAudioFileEntry->m_nMemoryBlockAlignment);
	}

	if (pAudioFileEntry->m_pMemoryBlock.get() == NPTR)
	{
		// Memory block is either full or too fragmented, let's try to throw everything out that can be removed and allocate again.
		TryToUncacheFiles();

		// And try again!
		if (m_pMemoryHeap.get() != NPTR)
		{
			pAudioFileEntry->m_pMemoryBlock = m_pMemoryHeap->AllocateBlock(pAudioFileEntry->m_nSize, pAudioFileEntry->m_sPath.c_str(), pAudioFileEntry->m_nMemoryBlockAlignment);
		}
	}

	return pAudioFileEntry->m_pMemoryBlock.get() != NPTR;
}

//////////////////////////////////////////////////////////////////////////
void CFileCacheManager::UncacheFile(CATLAudioFileEntry* const pAudioFileEntry)
{
	m_nCurrentByteTotal -= pAudioFileEntry->m_nSize;

	if (pAudioFileEntry->m_pReadStream)
	{
		pAudioFileEntry->m_pReadStream->Abort();
		pAudioFileEntry->m_pReadStream = NPTR;
	}

	if (pAudioFileEntry->m_pMemoryBlock.get() != NPTR && pAudioFileEntry->m_pMemoryBlock->GetData() != NPTR)
	{
		SATLAudioFileEntryInfo sFileEntryInfo;
		sFileEntryInfo.nMemoryBlockAlignment = pAudioFileEntry->m_nMemoryBlockAlignment;
		sFileEntryInfo.pFileData = pAudioFileEntry->m_pMemoryBlock->GetData();
		sFileEntryInfo.nSize = pAudioFileEntry->m_nSize;
		sFileEntryInfo.pImplData = pAudioFileEntry->m_pImplData;
		sFileEntryInfo.sFileName = PathUtil::GetFile(pAudioFileEntry->m_sPath.c_str());

		m_pImpl->UnregisterInMemoryFile(&sFileEntryInfo);
	}

	pAudioFileEntry->m_pMemoryBlock	= NPTR;
	pAudioFileEntry->m_nFlags				= (pAudioFileEntry->m_nFlags | eAFF_NOTCACHED) & ~(eAFF_CACHED|eAFF_REMOVABLE);
	assert(pAudioFileEntry->m_nUseCount == 0);
	pAudioFileEntry->m_nUseCount		= 0;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	pAudioFileEntry->m_oTimeCached.SetValue(0);
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
}

//////////////////////////////////////////////////////////////////////////
void CFileCacheManager::TryToUncacheFiles()
{
	TAudioFileEntries::iterator Iter(m_cAudioFileEntries.begin());
	TAudioFileEntries::const_iterator const IterEnd(m_cAudioFileEntries.end());

	for (; Iter != IterEnd; ++Iter)
	{
		CATLAudioFileEntry* const pAudioFileEntry = Iter->second;

		if (pAudioFileEntry != NPTR &&
			(pAudioFileEntry->m_nFlags & eAFF_CACHED) != 0 &&
			(pAudioFileEntry->m_nFlags & eAFF_REMOVABLE) != 0)
		{
			UncacheFileCacheEntryInternal(pAudioFileEntry, true);
		}
	}
}

///////////////////////////////////////////////////////////////////////////
void CFileCacheManager::UpdateLocalizedFileEntryData(CATLAudioFileEntry* const pAudioFileEntry)
{
	static SATLAudioFileEntryInfo oFileEntryInfo;
	oFileEntryInfo.bLocalized = true;
	oFileEntryInfo.nSize = 0;
	oFileEntryInfo.pFileData = NPTR;
	oFileEntryInfo.nMemoryBlockAlignment = 0;

	CryFixedStringT<MAX_AUDIO_FILE_NAME_LENGTH> sFileName(PathUtil::GetFile(pAudioFileEntry->m_sPath.c_str()));
	oFileEntryInfo.pImplData = pAudioFileEntry->m_pImplData;
	oFileEntryInfo.sFileName = sFileName.c_str();

	pAudioFileEntry->m_sPath = m_pImpl->GetAudioFileLocation(&oFileEntryInfo);
	pAudioFileEntry->m_sPath += sFileName.c_str();
	pAudioFileEntry->m_sPath.MakeLower();

	pAudioFileEntry->m_nSize = gEnv->pCryPak->FGetSize(pAudioFileEntry->m_sPath.c_str());

	assert(pAudioFileEntry->m_nSize > 0);
}

///////////////////////////////////////////////////////////////////////////
bool CFileCacheManager::TryCacheFileCacheEntryInternal(
	CATLAudioFileEntry* const pAudioFileEntry,
	TAudioFileEntryID const nFileID,
	bool const bLoadSynchronously,
	bool const bOverrideUseCount /* = false */,
	size_t const nUseCount /* = 0 */)
{
	bool bSuccess = false;

	if (!pAudioFileEntry->m_sPath.empty() &&
		(pAudioFileEntry->m_nFlags & eAFF_NOTCACHED) != 0 &&
		(pAudioFileEntry->m_nFlags & (eAFF_CACHED|eAFF_LOADING)) == 0)
	{
		if (DoesRequestFitInternal(pAudioFileEntry->m_nSize) && AllocateMemoryBlockInternal(pAudioFileEntry))
		{
			StreamReadParams oStreamReadParams;
			oStreamReadParams.nOffset      = 0;
			oStreamReadParams.nFlags       = IStreamEngine::FLAGS_NO_SYNC_CALLBACK;
			oStreamReadParams.dwUserData   = static_cast<DWORD_PTR>(nFileID);
			oStreamReadParams.nLoadTime    = 0;
			oStreamReadParams.nMaxLoadTime = 0;
			oStreamReadParams.ePriority    = estpUrgent;
			oStreamReadParams.pBuffer      = pAudioFileEntry->m_pMemoryBlock->GetData();
			oStreamReadParams.nSize        = static_cast<int unsigned>(pAudioFileEntry->m_nSize);
			
			pAudioFileEntry->m_nFlags |= eAFF_LOADING;
			pAudioFileEntry->m_pReadStream = gEnv->pSystem->GetStreamEngine()->StartRead(eStreamTaskTypeFSBCache, pAudioFileEntry->m_sPath.c_str(), this, &oStreamReadParams);

			if (bLoadSynchronously)
			{
				pAudioFileEntry->m_pReadStream->Wait();
			}
			
			// Always add to the total size.
			m_nCurrentByteTotal += pAudioFileEntry->m_nSize;
			bSuccess = true;
		}
		else
		{
			// Cannot have a valid memory block!
			assert(pAudioFileEntry->m_pMemoryBlock.get() == NPTR || pAudioFileEntry->m_pMemoryBlock->GetData() == NPTR);

			// This unfortunately is a total memory allocation fail.
			pAudioFileEntry->m_nFlags |= eAFF_MEMALLOCFAIL;

			// The user should be made aware of it.
			g_AudioLogger.Log(eALT_ERROR, "AFCM: could not cache \"%s\" as we are out of memory!", pAudioFileEntry->m_sPath.c_str());
		}
	}
	else if ((pAudioFileEntry->m_nFlags & (eAFF_CACHED|eAFF_LOADING)) != 0)
	{
		// The user should be made aware of it.
		g_AudioLogger.Log(eALT_WARNING, "AFCM: could not cache \"%s\" as it is either already loaded or currently loading!", pAudioFileEntry->m_sPath.c_str());

		bSuccess = true;
	}
	else if ((pAudioFileEntry->m_nFlags & eAFF_NOTFOUND) != 0)
	{
		// The user should be made aware of it.
		g_AudioLogger.Log(eALT_ERROR, "AFCM: could not cache \"%s\" as it was not found at the target location!", pAudioFileEntry->m_sPath.c_str());
	}

	// Increment the used count on GameHints.
	if ((pAudioFileEntry->m_nFlags & eAFF_USE_COUNTED) != 0 && (pAudioFileEntry->m_nFlags & (eAFF_CACHED|eAFF_LOADING)) != 0)
	{
		if (bOverrideUseCount)
		{
			pAudioFileEntry->m_nUseCount = nUseCount;
		}
		else
		{
			++pAudioFileEntry->m_nUseCount;
		}

		// Make sure to handle the eAFCS_REMOVABLE flag according to the m_nUsedCount count.
		if (pAudioFileEntry->m_nUseCount != 0)
		{
			pAudioFileEntry->m_nFlags &= ~eAFF_REMOVABLE;
		}
		else
		{
			pAudioFileEntry->m_nFlags |= eAFF_REMOVABLE;
		}
	}

	return bSuccess;
}
