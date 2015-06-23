#include "StdAfx.h"
#include "Timer.h"


//TODO: Дописать сохранение таймера

CTimer::CTimer()
{
	m_timerId = 0;
	m_fInitTime = 0;
	m_fDuration = 0;
	m_pListener = NULL;

	//Получаем фреймворк и регистрируем наш класс для обработки базовых событий.
	//По большей части здесь нам понадобиться функция Update()/PostUpdate()
	IGameFramework *pGameFramework = g_pGame->GetIGameFramework();
	if (pGameFramework != NULL)
		pGameFramework->RegisterListener(this, "Timer", FRAMEWORKLISTENERPRIORITY_DEFAULT);
}

void CTimer::OnPostUpdate(float fDeltaTime)
{
	if (m_bPause == false)
	{
		if (gEnv->pTimer->GetCurrTime() <= m_fInitTime + m_fDuration && m_pListener)
		{
			m_pListener->OnTick(m_timerId);
		}
		else
		{
			if (m_pListener) m_pListener->OnTimerFinished(m_timerId);
			m_fDuration = 0;
			m_fInitTime = 0;
			Pause();
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
	m_pListener = listener;
}

void CTimer::RemoveEventListener()
{
	SAFE_DELETE(m_pListener);
}

//Timer 
void CTimer::StartTimer(float duration, int id = 0)
{
	m_fInitTime = gEnv->pTimer->GetCurrTime();
	m_fDuration = duration;
	m_timerId = id;
	Continue();
	if (m_pListener) m_pListener->OnTimerStarted(m_timerId);
}

void CTimer::Pause()
{
	m_bPause = true;
}

void CTimer::Continue()
{
	m_bPause = false;
}

void CTimer::Restart()
{
	StartTimer(m_fDuration, m_timerId);
}