/****************************************************************************************************
* Class: CTimer
* Description: Позволяет использовать простой таймер для различных нужд
* Created by: Andrew Medvedev
* Date: 29.09.2014
* Edda Studio
****************************************************************************************************/

#pragma once

#include "IGameFramework.h"

class ITimerEvents
{
public:
	virtual void OnTimerStarted(int id) = 0;
	virtual void OnTick(int id) = 0;
	virtual void OnTimerFinished(int id) = 0;
};

class CTimer : public IGameFrameworkListener
{
public:
	CTimer();

	//IGameFrameworkListener
	void OnPostUpdate(float fDeltaTime);
	void OnSaveGame(ISaveGame* pSaveGame);
	void OnLoadGame(ILoadGame* pLoadGame);
	void OnLevelEnd(const char* nextLevel){}
	void OnActionEvent(const SActionEvent& event){}
	//~

	//Listener Methodes
	void AttachEventListener(ITimerEvents* listener);
	void RemoveEventListener();
	//~

	//Timer 
	void StartTimer(float duration, int id = 0);
	void Pause();
	void Continue();
	void Restart();
private:
	int m_timerId;
	float m_fInitTime;
	float m_fDuration;
	bool m_bPause;
	ITimerEvents* m_pListener;
};