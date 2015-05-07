#include "StdAfx.h"
#include "CameraMode.h"
#include "GameCVars.h"
#include "Player.h"
#include "GameActions.h"

#define OFFSET 0.5

CCameraMode::CCameraMode() : m_CurrentDist(0), m_CurrentXOffset(0), m_CurrentYOffset(0), m_DistToZoom(0), m_nRollType(ERollNULL)
{
	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pAmMgr != NULL)
		pAmMgr->AddExtraActionListener(this);

	IGameFramework *pFramework = g_pGame->GetIGameFramework();
	if (pFramework)
		pFramework->RegisterListener(this, "CameraMode", EFRAMEWORKLISTENERPRIORITY::FRAMEWORKLISTENERPRIORITY_DEFAULT);
}

CCameraMode::~CCameraMode()
{
	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pAmMgr != NULL)
		pAmMgr->RemoveExtraActionListener(this);
}

void CCameraMode::OnAction(const ActionId& action, int activationMode, float value)
{
	const CGameActions &actions = g_pGame->Actions();

	if (actions.CameraRollIn == action);
		//SetCameraZoom(OFFSET, ERollIn, true);

	if (actions.CameraRollOut == action);
		//SetCameraZoom(OFFSET, ERollOut, true);
}

//Implement camera zoom
void CCameraMode::SetCameraZoom(float value, ECameraRollType type, bool add)
{
	CPlayer *pPlayer = (CPlayer*)g_pGame->GetIGameFramework()->GetClientActor();
	if (!pPlayer)
		return;

	SStanceInfo *stanceInfo = (SStanceInfo*)pPlayer->GetStanceInfo(EStance::STANCE_STAND);
	if (!stanceInfo)
		return;

	if (add == true && m_CurrentDist > 1 && type == ERollIn)
	{
		m_DistToZoom += value;
		m_nRollType = ERollIn;
	}

	if (add == true && m_CurrentDist < 7 && type == ERollOut)
	{
		m_DistToZoom += value;
		m_nRollType = ERollOut;
	}

	m_CurrentDist = g_pGameCVars->cl_tpvDist;
	CryLogAlways("Zoom = %f", m_CurrentDist);
}

void CCameraMode::SetCameraXOffset(float value, bool add)
{
	CPlayer *pPlayer = (CPlayer*)g_pGame->GetIGameFramework()->GetClientActor();
	if (!pPlayer)
		return;

	SStanceInfo *stanceInfo = (SStanceInfo*)pPlayer->GetStanceInfo(EStance::STANCE_STAND);
	if (!stanceInfo)
		return;

	if (m_CurrentXOffset <= 1 && m_CurrentDist <= 2 && m_nRollType == ERollIn)
		stanceInfo->viewOffset.x = m_CurrentXOffset + value / 2;

	if (m_CurrentXOffset >= 0 && m_CurrentDist >= 2 && m_nRollType == ERollOut)
		stanceInfo->viewOffset.x = m_CurrentXOffset - value / 2;

	m_CurrentXOffset = stanceInfo->viewOffset.x;
}

void CCameraMode::SetCameraYOffset(float value, bool add)
{
}

void CCameraMode::OnPostUpdate(float fDeltaTime)
{
	//Если очередь зума есть и разница кадра меньше 0.25 и тип рола - приближение
	if (m_DistToZoom > 0 && fDeltaTime <= 0.25 && m_nRollType == ERollIn && (m_CurrentDist - m_DistToZoom) >= 1)
	{
		if (m_DistToZoom > 0)//Если дистанция больше 0 то если она больше 1 то ускоряем
		{
			if (m_DistToZoom > 0.5)
			{
				m_DistToZoom -= fDeltaTime*m_DistToZoom * 2;
				g_pGameCVars->cl_tpvDist -= fDeltaTime*m_DistToZoom * 2;

				//SetCameraXOffset(m_DistToZoom);
			}
			else
			{
				m_DistToZoom -= fDeltaTime*m_DistToZoom;
				g_pGameCVars->cl_tpvDist -= fDeltaTime*m_DistToZoom;

				//SetCameraXOffset(m_DistToZoom);
			}
		}
	}

	if (m_DistToZoom > 0 && fDeltaTime <= 0.25 && m_nRollType == ERollOut)
	{
		if (m_DistToZoom > 0)
		{
			if (m_DistToZoom > 0.5)
			{
				m_DistToZoom -= fDeltaTime*m_DistToZoom * 2;
				g_pGameCVars->cl_tpvDist += fDeltaTime*m_DistToZoom * 2;

				SetCameraXOffset(m_DistToZoom);

			}
			else
			{
				m_DistToZoom -= fDeltaTime*m_DistToZoom;
				g_pGameCVars->cl_tpvDist += fDeltaTime*m_DistToZoom;

				SetCameraXOffset(m_DistToZoom);
			}
		}
	}

	if (m_DistToZoom <= 0)
	{
		m_nRollType = ERollNULL;
		m_DistToZoom = 0;
	}
	if ((m_CurrentDist - m_DistToZoom) <= 1)
		m_DistToZoom = 0;
}

void CCameraMode::OnSaveGame(ISaveGame* pSaveGame)
{
}

void CCameraMode::OnLoadGame(ILoadGame* pLoadGame)
{
}

// Дописать смещение по Х
// Пошатывание камеры*
// Отключить возможность зума если мы в фп виде