#include "StdAfx.h"
#include "PlayerStatsManager.h"
#include "StatHealth.h"
#include "StatStamina.h"

#define CREATE_STAT(classname, type)\
{\
	C##classname *stat = new C##classname();\
	stat->InitStat(type);\
	AddStat(stat);\
}

CPlayerStatsManager::CPlayerStatsManager()
{
	CREATE_STAT(StatHealth, EPS_Health);
	CREATE_STAT(StatStamina,EPS_Stamina);
	CREATE_STAT(PlayerStat, EPS_Mana);
	CREATE_STAT(PlayerStat, EPS_Strength);
	CREATE_STAT(PlayerStat, EPS_Agila);
	CREATE_STAT(PlayerStat, EPS_Eloquence);

	//Resources
	CREATE_STAT(PlayerStat, EPS_Money);
	CREATE_STAT(PlayerStat, EPS_Wood);
	CREATE_STAT(PlayerStat, EPS_Stone);
	CREATE_STAT(PlayerStat, EPS_Iron);
}


void CPlayerStatsManager::AddStat(CPlayerStat* stat)
{
	m_pPlayerStats.push_back(stat);
#ifdef __TDEBUG__
	CryLogAlways("[PlayerStatsManager]: Stat %s was added", stat->GetStatName());
#endif
	for (int i = 0; i < m_pListeners.size(); i++)
		m_pListeners[i]->OnStatAdded(stat->GetType());
}
  
bool CPlayerStatsManager::DeleteStat(int type)
{
	for (int i = 0; i < m_pPlayerStats.size(); i++)
	{
		if (m_pPlayerStats[i]->GetType() == type)
		{
			m_pPlayerStats.erase(m_pPlayerStats.begin() + i);
#ifdef __TDEBUG__
			CryLogAlways("[PlayerStatsManager]: Stat %s was added", m_pPlayerStats[i]->GetStatName());
#endif
			for (int i = 0; i < m_pListeners.size(); i++)
				m_pListeners[i]->OnStatDeleted(type);

			return true;
		}
	}
	return false;
}

CPlayerStat* CPlayerStatsManager::GetStat(int type)
{
	for (int i = 0; i < m_pPlayerStats.size(); i++)
	{
		if (m_pPlayerStats[i]->GetType() == type)
		{
			return m_pPlayerStats[i];
		}
	}
	return NULL;
}


void CPlayerStatsManager::AddListener(IPlayerStatsListener *listener)
{
	m_pListeners.push_back(listener);
}


bool CPlayerStatsManager::DeleteListener(IPlayerStatsListener *listener)
{
	for (int i = 0; i < m_pListeners.size(); i++)
	{
		if (m_pListeners[i] == listener)
		{
			m_pListeners.erase(m_pListeners.begin() + i);
			return true;
		}
	}
	return false;
}

void CPlayerStatsManager::OnStatChanged(CPlayerStat* stat)
{
#ifdef __TDEBUG__
	CryLogAlways("[PlayerStatsManager]: Stat %s was chenged", stat->GetStatName());
#endif
	for (int i = 0; i < m_pListeners.size(); i++)
		m_pListeners[i]->OnStatChanged(stat);
}

bool CPlayerStatsManager::GetBackpackStatus()
{
	return m_bBackpackLost;
}

void CPlayerStatsManager::SetBackpackStatus(bool isBackpackLost)
{
	m_bBackpackLost = isBackpackLost;
}
