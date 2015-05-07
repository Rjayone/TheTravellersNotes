#include "StdAfx.h"
#include "CombatTargetManager.h"
#include "Player.h"
#include "IAIActor.h"
#include "SpecialFunctions.h"
#include "GameActions.h"

CCombatTarget::CCombatTarget()
{
	m_pHelper = NULL;
	g_pGame->GetIGameFramework()->RegisterListener(this, "CombatTarget", EFRAMEWORKLISTENERPRIORITY::FRAMEWORKLISTENERPRIORITY_GAME);
}

CCombatTarget::~CCombatTarget()
{
	g_pGame->GetIGameFramework()->UnregisterListener(this);
}

void CCombatTarget::ForceLookAt(EntityId id)
{
	CPlayer* pPlayer = (CPlayer*)g_pGame->GetIGameFramework()->GetClientActor();
	if (pPlayer == NULL)
		return;

	m_pHelper = NULL;
	CreateTagPointHelper();

	//IEntity* pTarget = gEnv->pEntitySystem->GetEntity(id);
	//if (pTarget)
	//{
	//	Vec3 finalPos = pTarget->GetPos() + Vec3(0, 0, 1.6);
	//	if (m_pHelper)
	//	{
	//		m_pHelper->SetPos(finalPos);
	//		pPlayer->SetForceLookAtTargetId(m_pHelper->GetId(), 0);

	//		//pTarget->AttachChild(m_pHelper);
	//	}
	//}
	IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(id);
	if (pActor)
	{
		Vec3 vWeaponPos = CSpecialFunctions::GetBonePos("Bip01 LookTarget", false, pActor->GetEntity());
		m_pHelper->SetPos(vWeaponPos);
		pPlayer->SetForceLookAtTargetId(m_pHelper->GetId(), 0);
	}
	else if (id == 0)
	{
		pPlayer->SetForceLookAtTargetId(0);
		g_pGameActions->FilterNoMouseMove()->Enable(false);
	}
}

void CCombatTarget::LookAtClosestTarget()
{
	EntityId closestTarget = GetClosestTarget();
	currentTarget = closestTarget;
	ForceLookAt(closestTarget);
}

EntityId CCombatTarget::GetClosestTarget()
{
	if (m_availableTarget.size() > 0)
		return m_availableTarget[0];
	return 0;
}


void CCombatTarget::GetScreenTargets()
{
	m_availableTarget.clear();

	float length = 50.f;
	Vec3 dir = gEnv->pSystem->GetViewCamera().GetViewdir();
	Vec3 camPos = gEnv->pSystem->GetViewCamera().GetPosition();

	primitives::cylinder cyl;
	cyl.axis = dir;
	cyl.hh = length;
	cyl.center = camPos + cyl.axis*cyl.hh;
	cyl.r = 20.f;

	IActor* pOwner = g_pGame->GetIGameFramework()->GetClientActor();
	int flags = ent_rigid | ent_sleeping_rigid | ent_independent | ent_static | ent_terrain | ent_water | ent_all;
	IPhysicalEntity *pIgnore = pOwner ? pOwner->GetEntity()->GetPhysics() : 0; //Игнорируем себя
	geom_contact *contacts;
	intersection_params params;


	IPhysicalWorld::SPWIParams pp;
	pp.itype = cyl.type; pp.pprim = &cyl; pp.sweepDir = Vec3(0); pp.entTypes = flags;
	pp.ppcontact = &contacts; pp.geomFlagsAll = 0; pp.geomFlagsAny = geom_colltype_foliage | geom_colltype_player; pp.pip = &params;
	pp.pForeignData = 0; pp.iForeignData = 0; pp.pSkipEnts = &pIgnore; pp.nSkipEnts = 1;
	float n = gEnv->pPhysicalWorld->PrimitiveWorldIntersection(pp);
	//int n = gEnv->pPhysicalWorld->PrimitiveWorldIntersection(primitives::cylinder::type, cyl, Vec3(ZERO), flags, &contacts, 0, geom_colltype_foliage | geom_colltype_player, &params, 0, 0, &pIgnore, 1);


	geom_contact *currentc = contacts;
	for (int i = 0; i < n; i++)
	{
		geom_contact *contact = currentc;
		if (contact != NULL)
		{
			int primId = contact->iPrim[0];
			IPhysicalEntity *pCollider = gEnv->pPhysicalWorld->GetPhysicalEntityById(primId);
			if (pCollider != NULL)
			{
				IEntity *pHitedEntity = gEnv->pEntitySystem->GetEntityFromPhysics(pCollider);
				if (pHitedEntity != NULL)
				{

					string entityName = pHitedEntity->GetName();
					IAIObject* pOwnerAI = pHitedEntity->GetAI();
					if (pOwnerAI != NULL && entityName != "Dude")
						m_availableTarget.push_back(pHitedEntity->GetId());
				}
			}
		}
		++currentc;
	}
}

void CCombatTarget::OnPostUpdate(float fDeltaTime)
{
	if (g_pGameCVars->td_draw == 2)
	{
		float length = 50.f;
		Vec3 dir = gEnv->pSystem->GetViewCamera().GetViewdir();
		Vec3 camPos = gEnv->pSystem->GetViewCamera().GetPosition();

		primitives::cylinder cyl;
		cyl.axis = dir + Vec3(0, 1, 0);
		cyl.hh = length;
		cyl.center = camPos + cyl.axis;//*cyl.hh;
		cyl.r = 20.f;

		//Отображение в сеточном виде
		SAuxGeomRenderFlags flags;
		flags.SetFillMode(e_FillModeWireframe);
		flags.SetCullMode(e_CullModeNone);

		//Ставим флаги и отрисовываем
		gEnv->pRenderer->GetIRenderAuxGeom()->SetRenderFlags(flags);
		gEnv->pRenderer->GetIRenderAuxGeom()->DrawCylinder(cyl.center, cyl.axis, cyl.r, cyl.hh, Vec3(255, 0, 0));

		IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(currentTarget);
		if (pActor)
		{
			Vec3 vWeaponPos = CSpecialFunctions::GetBonePos("Bip01 LookTarget", false, pActor->GetEntity());
			//Vec3 finalPos = pTarget->GetPos() + Vec3(0, 0, 1.6);
			gEnv->pRenderer->GetIRenderAuxGeom()->DrawSphere(vWeaponPos, 0.1, Vec3(0, 255, 0));

			//Проверка жива ли цель

			IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(currentTarget);
			if (pActor ? pActor->IsDead() : false)
			{
				ForceLookAt(0);
				g_pGameActions->FilterNoMouseMove()->Enable(false);
			}
		}

	}

	if (m_pHelper)
	{
		IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(currentTarget);
		if (pActor)
		{
			Vec3 vWeaponPos = CSpecialFunctions::GetBonePos("Bip01 LookTarget", false, pActor->GetEntity());
			//Vec3 finalPos = pTarget->GetPos() + Vec3(0, 0, 1.6);
			m_pHelper = gEnv->pEntitySystem->FindEntityByName("TargetCamHelper");
			if (m_pHelper)
			{
				m_pHelper->SetPos(vWeaponPos);
			}
		}
	}
}

void CCombatTarget::CreateTagPointHelper()
{
	if (m_pHelper == NULL)
	{
		SEntitySpawnParams ss;
		ss.sName = "TargetCamHelper";
		ss.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("TagPoint");
		if (ss.pClass)
		{
			m_pHelper = gEnv->pEntitySystem->SpawnEntity(ss);
		}
	}
}