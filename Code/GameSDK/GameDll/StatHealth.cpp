#include "StdAfx.h"
#include "StatHealth.h"
#include "PlayerStatsManager.h"
#include "Game.h"
#include "Player.h"

CStatHealth::CStatHealth()
{
	CPlayerStat();
	InitStat(EPS_Health);

	if (g_pGame->GetIGameFramework())
		g_pGame->GetIGameFramework()->RegisterListener(this,"StatHealth", FRAMEWORKLISTENERPRIORITY_DEFAULT);

	CPlayer *pPlayer = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (!pPlayer)
		return;
}

CStatHealth::~CStatHealth()
{
	CPlayerStatsManager *pStatsManager = g_pGame->GetPlayerStatsManager();
	if (!pStatsManager)
		return;

	pStatsManager->DeleteStat(EPS_Health);
	if (g_pGame->GetIGameFramework())
		g_pGame->GetIGameFramework()->UnregisterListener(this);
}

void CStatHealth::OnPostUpdate(float fDeltaTime)
{
	if (!g_pGame->GetIGameFramework())
		return;

	CPlayer *pPlayer = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (!pPlayer)
		return;

	if (pPlayer->GetHealth() != GetStatValue())
		SetValue(pPlayer->GetHealth());
}

void CStatHealth::OnActionEvent(const SActionEvent& event)
{
}