// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#ifndef FILE_CACHE_MANAGER_H_INCLUDED
#define FILE_CACHE_MANAGER_H_INCLUDED

#include "ATLEntities.h"
#include <IStreamEngine.h>

// Forward declarations
class CCustomMemoryHeap;
class CATLAudioFileEntry;

// Filter for drawing debug info to the screen
enum EAudioFileCacheManagerDebugFilter
{
	eAFCMDF_ALL              = 0,
	eAFCMDF_GLOBALS          = BIT(6),  // a
	eAFCMDF_LEVEL_SPECIFICS  = BIT(7),  // b
	eAFCMDF_USE_COUNTED      = BIT(8),  // c
};

class CFileCacheManager : public IStreamCallback
{
public:

	explicit CFileCacheManager(TATLPreloadRequestLookup& rPreloadRequests);
	~CFileCacheManager();

	// Public methods
	void								Init(IAudioSystemImplementation* const pImpl);
	void								Release();
	void								Update();
	TAudioFileEntryID		TryAddFileCacheEntry(XmlNodeRef const pFileNode, EATLDataScope const eDataScope, bool const bAutoLoad);
	bool								TryRemoveFileCacheEntry(TAudioFileEntryID const nAudioFileID, EATLDataScope const eDataScope);
	void								UpdateLocalizedFileCacheEntries();
	void								DrawDebugInfo(IRenderer* const pRenderer, float const fPosX, float const fPosY);

	EAudioRequestStatus	TryLoadRequest(TAudioPreloadRequestID const nPreloadRequestID, bool const bLoadSynchronously, bool const bAutoLoadOnly);
	EAudioRequestStatus	TryUnloadRequest(TAudioPreloadRequestID const nPreloadRequestID);
	EAudioRequestStatus	UnloadDataByScope(EATLDataScope const eDataScope);

private:

	CFileCacheManager(CFileCacheManager const&);						// Copy protection
	CFileCacheManager& operator=(CFileCacheManager const&);	// Copy protection

	// Internal type definitions.
	typedef std::map<TAudioFileEntryID, CATLAudioFileEntry*, std::less<TAudioFileEntryID>,
		STLSoundAllocator<std::pair<TAudioFileEntryID, CATLAudioFileEntry*> > >	TAudioFileEntries;

	// IStreamCallback
	virtual void									StreamAsyncOnComplete(IReadStream* pStream, unsigned int nError);
	virtual void									StreamOnComplete(IReadStream* pStream, unsigned int nError){}
	// ~IStreamCallback

	// Internal methods
	void													AllocateHeap(size_t const nSize, char const* const sUsage);
	bool													UncacheFileCacheEntryInternal(CATLAudioFileEntry* const pAudioFileEntry, bool const bNow, bool const bIgnoreUsedCount = false);
	bool													DoesRequestFitInternal(size_t const nRequestSize);
	bool													FinishStreamInternal(IReadStreamPtr const pStream, int unsigned const nError);
	bool													AllocateMemoryBlockInternal(CATLAudioFileEntry* const __restrict pAudioFileEntry);
	void													UncacheFile(CATLAudioFileEntry* const pAudioFileEntry);
	void													TryToUncacheFiles();
	void													UpdateLocalizedFileEntryData(CATLAudioFileEntry* const pAudioFileEntry);
	bool													TryCacheFileCacheEntryInternal(CATLAudioFileEntry* const pAudioFileEntry, TAudioFileEntryID const nFileID, bool const bLoadSynchronously, bool const bOverrideUseCount = false, size_t const nUseCount = 0);

	// Internal members
	IAudioSystemImplementation*		m_pImpl;
	TATLPreloadRequestLookup&			m_rPreloadRequests;
	TAudioFileEntries							m_cAudioFileEntries;

	_smart_ptr<CCustomMemoryHeap>	m_pMemoryHeap;
	size_t												m_nCurrentByteTotal;
	size_t												m_nMaxByteTotal;
}; 

#endif // FILE_CACHE_MANAGER_H_INCLUDED
