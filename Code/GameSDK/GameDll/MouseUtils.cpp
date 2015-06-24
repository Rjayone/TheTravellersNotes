#include <StdAfx.h>
#include "MouseUtils.h"
#include "GameActions.h"

CMouseUtils::CMouseUtils()
{
	IActionMapManager* pActionManager = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pActionManager != NULL)
		pActionManager->AddExtraActionListener(this);

	m_fInitialTime = 0;
	m_fClickUpTime = 0;
}

IEntity* CMouseUtils::GetMouseEntity(float rayLength)
{
	if (!g_pGame->GetIGameFramework()->GetClientActor())
		return NULL;

	ray_hit ray;
	Vec3  cameraPos = gEnv->pSystem->GetViewCamera().GetPosition() + gEnv->pSystem->GetViewCamera().GetViewdir();
	Vec3 cameraDir = gEnv->pSystem->GetViewCamera().GetViewdir();

	if (cameraPos.x == 0)
		return NULL;

	//��������� �������:
	// 1. ��������� ������ ������ ���������� ���.
	// 2. ����������� ������ ���������� �� ������ ����.
	// 3. �����, ��������� ������� ����� ������ ��������� ������������ ����, ������������ ������� � ��.
	// 4. ����� �������� �����������
	// 5. ������ �� ���������, ������� ����� ��������� ������� ��� ������������ ���� � ��������
	// 6. ������������ ���������� ������������ ����.
	gEnv->pPhysicalWorld->RayWorldIntersection(cameraPos, cameraDir * rayLength, ent_all, rwi_stop_at_pierceable | rwi_colltype_any, &ray, 1);

	if (IEntity *pEntity = gEnv->pEntitySystem->GetEntityFromPhysics(ray.pCollider))
		return pEntity;

	return NULL;
}

ray_hit  CMouseUtils::GetHitData(float rayLength)
{
	if (!g_pGame->GetIGameFramework()->GetClientActor() || !gEnv)
		return ray_hit();

	ray_hit ray;
	Vec3  cameraPos = gEnv->pSystem->GetViewCamera().GetPosition() + gEnv->pSystem->GetViewCamera().GetViewdir();
	Vec3 cameraDir = gEnv->pSystem->GetViewCamera().GetViewdir();

	if (cameraPos.x == 0)
		return ray_hit();

	gEnv->pPhysicalWorld->RayWorldIntersection(cameraPos, cameraDir * rayLength, ent_all, rwi_stop_at_pierceable | rwi_colltype_any, &ray, 1);
	return ray;
}


void CMouseUtils::OnAction(const ActionId& action, int activationMode, float value)
{
	if (g_pGameActions->attack1 == action)
	{
		if (activationMode == 1)
		{
			if (m_fInitialTime != 0.f)
				InitClickTime();
		}
		if (activationMode == 2)
		{
			if (m_fClickUpTime != 0.f)
				OnClickUp();
		}
	}
}

void CMouseUtils::InitClickTime()
{
	m_fInitialTime = gEnv->pTimer->GetCurrTime();
}


void CMouseUtils::OnClickUp()
{
	m_fClickUpTime = gEnv->pTimer->GetCurrTime();
#ifdef __TDEBUG__
	CryLog("Click time: %.2f", CalculateClickTime());
#endif
}


float CMouseUtils::OnClickUp(bool force)
{
	m_fClickUpTime = gEnv->pTimer->GetCurrTime();
#ifdef __TDEBUG__
	CryLog("Click time: %.2f", CalculateClickTime());
#endif

	return CalculateClickTime();
}


float CMouseUtils::CalculateClickTime()
{
	//������� �������� ���� ����� - 0.1 �������
	float clickTime =  m_fClickUpTime - m_fInitialTime;
	//m_fClickUpTime = m_fInitialTime = 0;
	return clickTime;
}