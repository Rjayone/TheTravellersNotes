#include "StdAfx.h"
#include "Timer.h"


//TODO: �������� ���������� �������

CTimer::CTimer()
{
	m_timerId = 0;
	m_fInitTime = 0;
	m_fDuration = 0;
	m_bPause = false;
	m_bStoped = false;
	m_pListener = NULL;

	//�������� ��������� � ������������ ��� ����� ��� ��������� ������� �������.
	//�� ������� ����� ����� ��� ������������ ������� Update()/PostUpdate()
	IGameFramework *pGameFramework = g_pGame->GetIGameFramework();
	if (pGameFramework != NULL)
		pGameFramework->RegisterListener(this, "Timer", FRAMEWORKLISTENERPRIORITY_DEFAULT);
}

CTimer::~CTimer()
{
	m_pListener = NULL;
}

void CTimer::OnPostUpdate(float fDeltaTime)
{
	if (m_bPause == false && m_bStoped == false)
	{
		if (gEnv->pTimer->GetCurrTime() <= m_fInitTime + m_fDuration && m_pListener)
		{
			m_pListener->OnTick(m_timerId);
		}
		else
		{
			m_fDuration = 0;
			m_fInitTime = 0;
			Pause();

			if (m_pListener)
				m_pListener->OnTimerFinished(m_timerId);
		}
	}
}

void CTimer::OnSaveGame(ISaveGame* pSaveGame)
{
}

void CTimer::OnLoadGame(ILoadGame* pLoadGame)
{
}

//Listener Methodes
void CTimer::AttachEventListener(ITimerEvents* listener)
{
	if (m_pListener == listener)
		return;
	m_pListener = listener;
}

void CTimer::RemoveEventListener()
{
	//SAFE_DELETE(m_pListener);
	m_pListener = NULL;
}

//Timer 
void CTimer::StartTimer(float duration, int id)
{
	m_fInitTime = gEnv->pTimer->GetCurrTime();
	m_fDuration = duration;
	m_timerId = id;

	Continue();

	if (m_pListener)
		m_pListener->OnTimerStarted(m_timerId);
}

void CTimer::StopTimer()
{
	m_fDuration = 0;
	m_fInitTime = 0;
	m_bStoped = true;
}

void CTimer::Pause()
{
	m_bPause = true;
}

void CTimer::Continue()
{
	m_bStoped = false;
	m_bPause = false;
}

void CTimer::Restart()
{
	StartTimer(m_fDuration, m_timerId);
}