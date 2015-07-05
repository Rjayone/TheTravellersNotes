#include "StdAfx.h"
#include "Events.h"
#include "EditorGame.h"

//-------------------------------------------------------------------
CEventsDispatcher::CEventsDispatcher()
{
	//Получаем фреймворк и регистрируем наш класс для обработки базовых событий.
	//По большей части здесь нам понадобиться функция Update()/PostUpdate()
	IGameFramework *pGameFramework = g_pGame->GetIGameFramework();
	if (pGameFramework != NULL)
		pGameFramework->RegisterListener(this, "Events", FRAMEWORKLISTENERPRIORITY_DEFAULT);
}

//-------------------------------------------------------------------
CEventsDispatcher::~CEventsDispatcher()
{
	//Удаляем калсс из слушателей, дабы не вылетело, если класс удален
	IGameFramework *pGameFramework = g_pGame->GetIGameFramework();
	if (pGameFramework != NULL)
		pGameFramework->UnregisterListener(this);
}

//-------------------------------------------------------------------
void CEventsDispatcher::AddListener(IEvents* pListener)
{
	for (int i = 0; i < m_pListeners.size(); i++)
		if (m_pListeners[i] == pListener)
			return;
	m_pListeners.push_back(pListener);
}

//-------------------------------------------------------------------
void CEventsDispatcher::RemoveListener(IEvents* pListener)
{
	for (int i = 0; i < m_pListeners.size(); i++)
	{
		if (m_pListeners[i] != NULL)
			if (m_pListeners[i] == pListener)
				m_pListeners.erase(m_pListeners.begin() + i);
	}
}


//-------------------------------------------------------------------
void CEventsDispatcher::OnPostUpdate(float fDeltaTime)
{
}

void CEventsDispatcher::OnPlayerEnterInEditoriGameMode()
{
	for (int i = 0; i < m_pListeners.size(); i++)
		m_pListeners[i]->OnPlayerEnterInEditoriGameMode();
}
void CEventsDispatcher::OnPlayerExitFromEditorGameMode()
{
	for (int i = 0; i < m_pListeners.size(); i++)
		m_pListeners[i]->OnPlayerExitFromEditorGameMode();
}