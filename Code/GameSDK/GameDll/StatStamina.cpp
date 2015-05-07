#include "StdAfx.h"
#include "StatStamina.h"
#include "PlayerMovementController.h"
#include "PlayerStatsManager.h"

CStatStamina::CStatStamina()
{
	CPlayerStat();
	InitStat(EPS_Stamina);

	if (g_pGame->GetIGameFramework())
		g_pGame->GetIGameFramework()->RegisterListener(this, "StatStamina", FRAMEWORKLISTENERPRIORITY_DEFAULT);
}

CStatStamina::~CStatStamina()
{
	CPlayerStatsManager *pStatsManager = g_pGame->GetPlayerStatsManager();
	if (!pStatsManager)
		return;

	pStatsManager->DeleteStat(EPS_Stamina);
	if (g_pGame->GetIGameFramework())
		g_pGame->GetIGameFramework()->UnregisterListener(this);
}

void CStatStamina::OnPostUpdate(float fDeltaTime)
{
	//Закоменчено, ибо в лаунчере вылетает
	/*CPlayer *pl = (CPlayer*)g_pGame->GetIGameFramework()->GetClientActor();
	if (pl->IsJumping())
	{
		CryLog("Jump");
	}*/
	//CPlayerMovementController *pMoveController = (CPlayerMovementController*)g_pGame->GetIGameFramework()->GetClientActor()->GetMovementController();
	//if (!pMoveController)
	//	return;

	//SMovementState state;
	//pMoveController->GetMovementState(state);
	//state.maxSpeed = 5;
}