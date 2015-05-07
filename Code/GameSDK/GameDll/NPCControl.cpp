#include "StdAfx.h"
#include "NPCControl.h"
#include "IAIActor.h"
#include "IAIObject.h"
#include "IPathfinder.h"
#include "GameActions.h"
#include "IGoalPipe.h"
#include "IAIObjectManager.h" 
#include "MouseDirection.h"

CNPCControl::CNPCControl()
{
	IGameFramework *pGameFramework = g_pGame->GetIGameFramework();
	if (pGameFramework)
		pGameFramework->RegisterListener(this, "NPCControl", FRAMEWORKLISTENERPRIORITY_DEFAULT);

	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pAmMgr != NULL)
	{
		pAmMgr->AddExtraActionListener(this);
	}
}

IEntity* CNPCControl::GetEntity()
{
	ray_hit ray;
	Vec3  cameraPos = gEnv->pSystem->GetViewCamera().GetPosition() + gEnv->pSystem->GetViewCamera().GetViewdir();
	Vec3 cameraDir = gEnv->pSystem->GetViewCamera().GetViewdir();

	gEnv->pPhysicalWorld->RayWorldIntersection(cameraPos, cameraDir * 5, ent_all, rwi_stop_at_pierceable | rwi_colltype_any, &ray, 1);

	if (IEntity *pEntity = gEnv->pEntitySystem->GetEntityFromPhysics(ray.pCollider))
	{
		return pEntity;
	}
}

void CNPCControl::Execute()
{
	IEntity *pEntity = gEnv->pEntitySystem->GetEntity(CurrentNPC);
	if (pEntity)
	{
		IAIActor *npc = pEntity->GetAI()->CastToIAIActor();
		if (npc)
		{

			IAISignalExtraData * exd = gEnv->pAISystem->CreateSignalExtraData();
			exd->point = GetMovePoint();

			IPipeUser *pPipe = pEntity->GetAI()->CastToIPipeUser();


			pPipe->SelectPipe(0, "_first_", pEntity->GetAI());
			IGoalPipe *o = pPipe->InsertSubPipe(1, "ACT_GOTO", pEntity->GetAI());
			npc->SetSignal(AISIGNAL_DEFAULT, "ACT_GOTO");
		}
	}
}

Vec3 CNPCControl::GetMovePoint()
{
	ray_hit ray;
	Vec3  cameraPos = gEnv->pSystem->GetViewCamera().GetPosition() + gEnv->pSystem->GetViewCamera().GetViewdir();
	Vec3 cameraDir = gEnv->pSystem->GetViewCamera().GetViewdir();

	gEnv->pPhysicalWorld->RayWorldIntersection(cameraPos, cameraDir * 5, ent_all, rwi_stop_at_pierceable | rwi_colltype_any, &ray, 1);
	return ray.pt;
}

void CNPCControl::OnPostUpdate(float fDeltaTime)
{
}

CMouseDirection md;
void CNPCControl::OnAction(const ActionId& action, int activationMode, float value)
{
	const CGameActions &actions = g_pGame->Actions();
	if (actions.MoveTo == action)
	{
		//Execute();
		
		md.SetPoint();
	}
	if (actions.GetNPC == action)
	{
		//IEntity *p = GetEntity();
		//if (p)
		//	CurrentNPC = p->GetId();
		md.ClosePath();
	}
}


