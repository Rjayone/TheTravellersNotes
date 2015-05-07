#include "StdAfx.h"
#include "Game.h"
#include "PlayerStats.h"
#include "PlayerStatsManager.h"
#include "Globals.h"

CPlayerStat::CPlayerStat()
{
	m_fValue = 0.0F;
	m_fBonus = 0.0F;
	m_nType = EPS_NULL;
	m_sName = "";
	m_sDescription = "";
}

void CPlayerStat::SetValue(float value)
{
	m_fValue = value;
	OnStatChange(this);
}


void CPlayerStat::SetBonus(float bonus)
{
	m_fBonus = bonus;
	OnStatChange(this);
}

bool CPlayerStat::InitStat(int type)
{	
	XmlNodeRef StatsDescriptionFile = gEnv->pSystem->LoadXmlFromFile(PathUtil::GetGameFolder() + "/Libs/PlayerStats/PlayerStatsDescription.xml");
	if (StatsDescriptionFile == NULL)
		return NULL;

	for (int i = 0; i<StatsDescriptionFile->getChildCount(); i++)
	{
		XmlNodeRef cStat = StatsDescriptionFile->getChild(i);//<item ... />
		if (cStat != NULL)
		{
			const char *statName = cStat->getAttr("name");
			int _type = atoi(cStat->getAttr("type"));
			if (_type == type)
			{
				m_nType = type;
				m_sName = statName;
				m_sDescription = cStat->getAttr("descr");				
				m_fValue = atoi(cStat->getAttr("baseValue"));
				return true;
			}
		}
	}
	return false;
}


void CPlayerStat::OnStatChange(CPlayerStat* stat)
{
	CPlayerStatsManager *pStatsManager = g_pGame->GetPlayerStatsManager();
	if (pStatsManager != NULL)
	{
		pStatsManager->OnStatChanged(stat);
	}
}