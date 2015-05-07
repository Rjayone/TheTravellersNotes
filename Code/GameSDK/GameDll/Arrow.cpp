#include "StdAfx.h"
#include "Arrow.h"
#include "Game.h"
#include "IHardwareMouse.h"
#include "Holding.h"
#include "SpecialFunctions.h"

//-----------------------------------------------------------

static int Counter = 0;

CArrow::CArrow()
{
	pArrowEntity = NULL;
	m_pAmmoParams = NULL;
}

//-----------------------------------------------------------
CArrow::~CArrow()
{
}


//------------------------------------------------------------------------
void CArrow::SetParams(const SProjectileDesc& projectileDesc)
{
	CProjectile::SetParams(projectileDesc);
}

//----------------------------------------------------------
void CArrow::Pickalize(bool pick, Vec3 pos, Quat rot)
{
	if (pick)
	{
		const char* pickup_name = "ArrowPickup";
		SEntitySpawnParams spawnParams;
		spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(pickup_name);
		if (!spawnParams.pClass)
			return;

		spawnParams.vPosition = pos;
		spawnParams.qRotation = rot;

		pArrowEntity = gEnv->pEntitySystem->SpawnEntity(spawnParams);
		GetGameObject()->GetEntity()->Hide(true);
	}
}


//---------------------------------------------------------------------

void CArrow::Launch(const Vec3 &pos, const Vec3 &dir, const Vec3 &velocity, float speedScale)
{
	m_pAmmoParams = g_pGame->GetWeaponSystem()->GetAmmoParams(GetEntity()->GetClass());
	Vec3 _dir = gEnv->pSystem->GetViewCamera().GetViewdir();
	
	if (m_pAmmoParams != NULL)
		GetEntity()->LoadGeometry(1, m_pAmmoParams->fpGeometryName);
	GetEntity()->AddFlags(ENTITY_FLAG_CASTSHADOW);

	//”читываем врем€ удержани€ на скорость полета стрелы
	CHolding* pHolding = ((CItem*)g_pGame->GetIGameFramework()->GetClientActor()->GetCurrentItem())->GetHoldingStatus();
	if (pHolding)
		speedScale *= 1 + (pHolding->GetHoldingTime() / 2);

	//Ќадо отрисовать точку, с которой идет вылет стрелы
	CProjectile::Launch(pos, _dir, _dir, speedScale);
	launchRotation = GetEntity()->GetRotation();
}

void CArrow::HandleEvent(const SGameObjectEvent &event)
{
	if (event.event == eGFE_OnCollision)
	{
		EventPhysCollision *pCollision = reinterpret_cast<EventPhysCollision *>(event.ptr);
		
		if (!pCollision)
			return;

		CProjectile::HandleEvent(event);
		int colliderId = pCollision->idCollider;
		IPhysicalEntity *temp = reinterpret_cast<IPhysicalEntity *>(pCollision->pEntity);
		//Pickalize(true, pCollision->pt, launchRotation);
		
		
		CGameRules *pGameRules = g_pGame->GetGameRules();
		IEntity *pTarget = pCollision->iForeignData[1] == PHYS_FOREIGN_ID_ENTITY ? (IEntity*)pCollision->pForeignData[1] : 0;
		float finalDamage = 500;
		const int hitMatId = pCollision->idmat[1];   	   
		const bool bProcessCollisionEvent = ProcessCollisionEvent(pTarget);
		if (bProcessCollisionEvent)
		{
			//================================= Process Hit =====================================
			//Only process hits that have a target
			if (pTarget)
			{
				Vec3 dir = gEnv->pSystem->GetViewCamera().GetViewdir();
				const char* BoneName = ProcessHit(*pGameRules, *pCollision, *pTarget, finalDamage, hitMatId, dir);				
				//Pickalize(pCollision, pTarget, hitMatId);
				AttachArrow(pTarget, BoneName);
			}//~
			

			//==================================== Notify AI    ======================================
			if (gEnv->pAISystem)
			{
				if (gEnv->pEntitySystem->GetEntity(m_ownerId))
				{
					ISurfaceType *pSurfaceType = gEnv->p3DEngine->GetMaterialManager()->GetSurfaceType(hitMatId);
					const ISurfaceType::SSurfaceTypeAIParams* pParams = pSurfaceType ? pSurfaceType->GetAIParams() : 0;
					const float radius = pParams ? pParams->fImpactRadius : 2.5f;
					const float soundRadius = pParams ? pParams->fImpactSoundRadius : 20.0f;

					SAIStimulus stim(AISTIM_BULLET_HIT, 0, m_ownerId, pTarget ? pTarget->GetId() : 0, pCollision->pt, pCollision->vloc[0].GetNormalizedSafe(ZERO), radius);
					gEnv->pAISystem->RegisterStimulus(stim);

					SAIStimulus stimSound(AISTIM_SOUND, AISTIM_BULLET_HIT, m_ownerId, 0, pCollision->pt, ZERO, soundRadius);
					gEnv->pAISystem->RegisterStimulus(stimSound);
				}
			}//~
			
		}

		//========================================= Surface Pierceability ==============================
		if (pCollision->pEntity[0]->GetType() == PE_PARTICLE)
		{
			const SPierceabilityParams& pierceabilityParams = m_pAmmoParams->pierceabilityParams;
		
			//If collided water
			if (s_materialLookup.IsMaterial(pCollision->idmat[1], CProjectile::SMaterialLookUp::eType_Water))
			{
				if (pierceabilityParams.DestroyOnWaterImpact())
				{
					GetEntity()->SetPos(pCollision->pt);
					Destroy();
				}
				else
				{
					const STrailParams* pTrail = m_pAmmoParams->pTrailUnderWater;
					if (!pTrail) return;

					CTracerManager::STracerParams params;
					params.position = pCollision->pt;
					params.destination = pCollision->pt + (pCollision->vloc[0].GetNormalizedSafe() * 100.0f);
					params.geometry = NULL;
					params.effect = pTrail->effect.c_str();
					params.speed = 35.0f;
					params.lifetime = 2.5f;
					params.delayBeforeDestroy = 0.f;

					g_pGame->GetWeaponSystem()->GetTracerManager().EmitTracer(params);
				}
			}
			else if (m_pAmmoParams->bounceableBullet == 0)
			{
				float bouncy, friction;
				uint32 pierceabilityMat;
				gEnv->pPhysicalWorld->GetSurfaceParameters(pCollision->idmat[1], bouncy, friction, pierceabilityMat);
				pierceabilityMat &= sf_pierceable_mask;
		
				const bool terrainHit = (pCollision->idCollider == -1);
		
				bool thouShallNotPass = terrainHit;
				if (!CheckAnyProjectileFlags(ePFlag_ownerIsPlayer))
					thouShallNotPass = thouShallNotPass || pierceabilityParams.DestroyOnImpact(pierceabilityMat);
		
				if (!thouShallNotPass);
					//HandlePierceableSurface(pCollision, pTarget, hitDir, bProcessCollisionEvent);
			}
		}
		if (pTarget != NULL && pArrowEntity != NULL)
		{
			if (ISurfaceType* pSurfaceType = gEnv->p3DEngine->GetMaterialManager()->GetSurfaceType(pCollision->idmat[1]))
			{
				
			}
			//pTarget->AttachChild(pArrowEntity);
		}
	}



}

//--------------------------------------------------------------------

void CArrow::Update(SEntityUpdateContext &ctx, int updateSlot)
{
}

//---------------------------------------------------------------------

void CArrow::OnHit(const HitInfo& hit)
{
	CryLog("Hited");
}

//------------------------------------------------------------------------
const char* CArrow::ProcessHit(CGameRules& gameRules, const EventPhysCollision& collision, IEntity& target, float damage, int hitMatId, const Vec3& hitDir)
{
	if (damage > 0.f)
	{
		EntityId targetId = target.GetId();
		HitInfo hitInfo(m_ownerId ? m_ownerId : m_hostId, targetId, m_weaponId,
			damage, 0.0f, hitMatId, collision.partid[1],
			m_hitTypeId, collision.pt, hitDir, collision.n);

		hitInfo.remote = IsRemote();
		hitInfo.projectileId = GetEntityId();
		hitInfo.bulletType = m_pAmmoParams->bulletType;
		hitInfo.knocksDown = CheckAnyProjectileFlags(ePFlag_knocksTarget) && (damage > m_minDamageForKnockDown);
		hitInfo.knocksDownLeg = m_chanceToKnockDownLeg > 0 && damage > m_minDamageForKnockDownLeg && m_chanceToKnockDownLeg > (int)Random(100);
		hitInfo.hitViaProxy = CheckAnyProjectileFlags(ePFlag_firedViaProxy);
		hitInfo.aimed = CheckAnyProjectileFlags(ePFlag_aimedShot);
		
		gameRules.ClientHit(hitInfo);		
		ReportHit(targetId);

		if (ICharacterInstance *pCharacterInstance = target.GetCharacter(0))
			return pCharacterInstance->GetIDefaultSkeleton().GetJointNameByID(hitInfo.partId);
		return NULL;
	}
}

void CArrow::AttachArrow(IEntity* pTarget, const char* boneName)
{	
	ICharacterInstance* charInst = pTarget->GetCharacter(0);
	IAttachmentManager* pAttachmentManager = NULL;
	if (!charInst || !(pAttachmentManager = charInst->GetIAttachmentManager()))
		return;			
	Counter ++;
	char buffer[33];
	if (IAttachment* attachment = pAttachmentManager->CreateAttachment("AttachArrowToChar" + string(itoa(Counter, buffer, 10)), CA_BONE, boneName, true))
		{
			//Spawn a copy of projectile
			/*SEntitySpawnParams params;
			params.sName = GetEntity()->GetName();
			params.pClass = GetEntity()->GetClass();
			*/

			const char* pickup_name = "ArrowPickup";
			SEntitySpawnParams spawnParams;
			spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(pickup_name);
			if (!spawnParams.pClass)
				return;

			if (IEntity* pEntity = gEnv->pEntitySystem->SpawnEntity(spawnParams)){

				int32 boneId = 0;

				IDefaultSkeleton& rIDefaultSkeleton = charInst->GetIDefaultSkeleton();
				boneId = rIDefaultSkeleton.GetJointIDByName(boneName);

				ISkeletonPose* pSkeleton = charInst->GetISkeletonPose();
				const QuatT boneSpace = QuatT(pTarget->GetWorldTM()) * pSkeleton->GetAbsJointByID(boneId);


				QuatT relProj;				
				relProj.t = CSpecialFunctions::GetBonePos(boneName, false, pTarget)+(GetEntity()->GetPos() - CSpecialFunctions::GetBonePos(boneName, false, pTarget));
				relProj.q = GetEntity()->GetRotation();
				GetEntity()->SetWorldTM(Matrix34(relProj));

				relProj = boneSpace.GetInverted() * relProj;
				auto relProjPos = relProj.t;
				auto relProjRot = relProj.q;


				attachment->SetAttRelativeDefault(QuatT(relProjRot, relProjPos));

				CEntityAttachment* pEntityAttachment = new CEntityAttachment();
				pEntityAttachment->SetEntityId(pEntity->GetId());
				attachment->AddBinding(pEntityAttachment);
				//destroy original projectile
				Destroy();
			}
		}
	
}