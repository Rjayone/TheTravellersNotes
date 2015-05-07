// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#ifndef ATLUTILS_H_INCLUDED
#define ATLUTILS_H_INCLUDED

#include <AudioLogger.h>
#include <STLSoundAllocator.h>

#define ATL_FLOAT_EPSILON 1.0e-6

///////////////////////////////////////////////////////////////////////////
template<typename TMap, typename TKey>
bool FindPlace(TMap& map, TKey const& key, typename TMap::iterator& iPlace);

template<typename TMap, typename TKey>
bool FindPlaceConst(TMap const& map, TKey const& key, typename TMap::const_iterator& iPlace);

//////////////////////////////////////////////////////////////////////////
template <typename ObjType, typename IDType = size_t>
class CInstanceManager
{
public:

	~CInstanceManager() {}

	typedef std::vector<ObjType*, STLSoundAllocator<ObjType*> > TPointerContainer;
	
	TPointerContainer m_cReserved;
	IDType						m_nIDCounter;
	IDType const			m_nReserveSize;
	IDType const			m_nMinCounterValue;

	CInstanceManager(size_t const nReserveSize, IDType const nMinCounterValue)
		: m_nIDCounter(nMinCounterValue)
		, m_nReserveSize(nReserveSize)
		, m_nMinCounterValue(nMinCounterValue)
	{
		m_cReserved.reserve(nReserveSize);
	}

	IDType GetNextID()
	{
		if (m_nIDCounter >= m_nMinCounterValue)
		{
			return m_nIDCounter++;
		}
		else
		{
			g_AudioLogger.Log(eALT_ERROR, "An AudioSystem InstanceManager ID counter wrapped around.");
			m_nIDCounter = m_nMinCounterValue;
			return m_nIDCounter;
		}
	}
};

//////////////////////////////////////////////////////////////////////////
class CSmoothFloat
{
public:

	explicit CSmoothFloat(float const fAlpha, float const fPrecision, float const fInitValue = 0.0f)
		: m_fValue(fInitValue)
		, m_fTarget(fInitValue)
		, m_bIsActive(false)
		, m_fAlpha(fabs_tpl(fAlpha))
		, m_fPrecision(fabs_tpl(fPrecision))
	{}

	~CSmoothFloat() {}

	void	Update(float const fUpdateIntervalMS)
	{
		if (m_bIsActive)
		{
			if (fabs_tpl(m_fTarget - m_fValue) > m_fPrecision)
			{
				// still need not reached the target within the specified precision
				m_fValue += (m_fTarget - m_fValue) * m_fAlpha;
			}
			else
			{
				//reached the target within the last update frame
				m_fValue = m_fTarget;
				m_bIsActive = false;
			}
		}
	}

	float GetCurrent() const {return m_fValue;}

	void	SetNewTarget(float const fNewTarget, bool const bReset = false) 
	{
		if (bReset)
		{
			m_fTarget = fNewTarget;
			m_fValue = fNewTarget;
		}
		else if (fabs_tpl(fNewTarget - m_fTarget) > m_fPrecision)
		{
			m_fTarget = fNewTarget; 
			m_bIsActive = true;
		}
	}

	void	Reset(float const fInitValue = 0.0f)
	{
		m_fValue = m_fTarget = fInitValue;
		m_bIsActive = false;
	}

private:

	float				m_fValue;
	float				m_fTarget;
	bool				m_bIsActive;
	float const	m_fAlpha;
	float const m_fPrecision;
};

//--------------------------- Implementations ------------------------------
template<typename TMap, typename TKey>
bool FindPlace(TMap& map, TKey const& key, typename TMap::iterator& iPlace)
{
	iPlace = map.find(key);
	return (iPlace != map.end());
}

///////////////////////////////////////////////////////////////////////////
template<typename TMap, typename TKey>
bool FindPlaceConst(TMap const& map, TKey const& key, typename TMap::const_iterator& iPlace)
{
	iPlace = map.find(key);
	return (iPlace != map.end());
}

#endif // ATLUTILS_H_INCLUDED
