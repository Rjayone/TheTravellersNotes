#include <StdAfx.h>
#include "Sword.h"
#include "Weapon.h"
#include "MouseDirection.h"
#include "AttackTable.h"
#include "Player.h"
#include "MouseUtils.h"
#include "ITargetTrackManager.h"
#include "SpecialFunctions.h"
#include "MathUtils.h"
#include "PlayerStatsManager.h"
#include "IMaterialEffects.h"

#define _MELEE_RANGE_ 1.95

#define _MELEE_HELPER_

#define _TIMER_ATTACK 1
#define _TIMER_DRAGGING 2
#define _TIMER_COMBO_ATTACK 3
#define _HOLDING_RESET_STATUS 4

CSword::CSword()
{
	m_Holding = NULL;
	m_CombatStance = NULL;
	cyl = NULL;

	m_bBusy = false;
	m_bNowAttack = false;
	m_bHitSoundPlayed = false;

	m_selectedCombo = 0;
	m_comboMoveCount = 0;
	m_performedMoveCount = 0;

	m_prevPt = Vec3(0);
}

//-------------------------------------------------------------------
bool CSword::Init(IGameObject * pGameObject)
{
	if (!CWeapon::Init(pGameObject))
		return false;

	SetGameObject(pGameObject);
	return true;
}

//-------------------------------------------------------------------
void CSword::Release()
{
	delete this;
}

//-------------------------------------------------------------------
void CSword::Select(bool select)
{
	if (select == true && IsOwnerClient())
	{
		if (!m_CombatStance)
			m_CombatStance = new CCombatStance();
		m_CombatStance->Init(this);
		m_CombatStance->ApplyStance(e_CombatStanceVertical);
	}
	else
	{
		if (IsOwnerClient())
		{
			if (m_CombatStance)
				SAFE_DELETE(m_CombatStance);
			ClearAllTags();
		}
	}

	CWeapon::Select(select);
}

//-------------------------------------------------------------------
void  CSword::AutoDeselect()
{
	IActionPtr pAction = new TAction< SAnimationContext >(PP_PlayerActionUrgent, GetFragmentIds().deselect);
	CPlayer* pPlayer = (CPlayer*)g_pGame->GetIGameFramework()->GetClientActor();
	pPlayer->GetAnimatedCharacter()->GetActionController()->Queue(*pAction);

	IActionController *pActionController = GetActionController();
	if (!pActionController)
		return;

	SAnimationContext &animContext = pActionController->GetContext();
	animContext.state.Set(PlayerMannequin.tagIDs.nw, true);
}

//-------------------------------------------------------------------
void CSword::Update(SEntityUpdateContext& ctx, int slot)
{
	CWeapon::Update(ctx, slot);

	if (m_Holding)
		m_Holding->Update();
	if (m_CombatStance)
		m_CombatStance->Update();

	//Debug draw
	if (g_pGameCVars->td_draw == 1)
	{
		DebugDraw();
	}

	if (m_Holding && IsOwnerClient())
	{
		//Если начат удар
		int holdingStatus = m_Holding->GetStatus();
		if (holdingStatus == e_HoldingStatusRelease)// || holdingStatus == e_HoldingStatusShortClick)
		{
			//Покадрово просчитываем удар
			PerfomIntersection();
		}
	}

	//Измеряем, если смещение более чем на 15 пикселей 
	//if (m_MouseDir.IsDragging())
	//{
	//	m_MouseDir.PerfomClosePath();
	//	if (m_MouseDir.GetDir().GetLength() > 15)
	//	{
	//		m_MouseDir.ClosePath();//Закрываем дабы больше не просчитывалось
	//		StopFire();
	//		gEnv->pInput->EnableDevice(eDI_Mouse, true);
	//	}
	//}
}

//-------------------------------------------------------------------
void CSword::ProcessEvent(SEntityEvent& event)
{
	if (m_Holding)
		m_Holding->ProcessEvent(event);

	switch (event.event)
	{
	case ENTITY_EVENT_TIMER:
	{
		GetEntity()->KillTimer(event.nParam[0]);
		if (event.nParam[0] == _TIMER_ATTACK || event.nParam[0] == _TIMER_COMBO_ATTACK)
		{
		   m_bBusy = false;
		   if (event.nParam[0] == _TIMER_COMBO_ATTACK)
			   m_performedMoveCount++;
		}
		if (event.nParam[0] == _TIMER_DRAGGING)
		{
		   m_MouseDir.ClosePath();//Закрываем дабы больше не просчитывалось
		   StopFire();
		   //gEnv->pInput->EnableDevice(eDI_Mouse, true);
		}
		if (event.nParam[0] == _HOLDING_RESET_STATUS)//Евент на окончание проигрывания анимок
		{
		   m_bNowAttack = false;
		   m_bHitSoundPlayed = false;
		   CryLog("End attack");
		}

		break;
	}
	};
}

//-------------------------------------------------------------------
void CSword::StartFire()
{
	if (!IsOwnerClient() && IsBusy())
		return;

	CPlayerStatsManager* pStatsManager = g_pGame->GetPlayerStatsManager();
	if (pStatsManager != NULL)
	{
		CPlayerStat* pStrength = pStatsManager->GetStat(EPS_Strength);
		if (pStrength)
		{
			float currentValue = pStrength->GetStatBonus() + pStrength->GetStatValue();
			if (currentValue < 5)
				return;
		}
	}

	if (!m_Holding)
	{
		m_Holding = new CHolding();
		m_Holding->Init(this);
	}
	m_MouseDir.SetPoint();
	m_Holding->StartFire(this);
	//damageIgnore.clear();
	//GetEntity()->SetTimer(_TIMER_DRAGGING, 50); //Через это время должна сработать закрывающая точка		
	//gEnv->pInput->EnableDevice(eDI_Mouse, false);
	//
	//Код ниже нужен для определения направления удара при помощи мышки
	/* КОД ДЛЯ КОМБО УДАРОВ
	if (IsBusy() || !IsOwnerClient())
	return;

	//Если комбо не задано
	if (m_performedMoveCount > GetComboMoveCount(m_selectedCombo)-1 || m_selectedCombo == e_ComboNull)
	{
	m_selectedCombo = Random(1, 4);
	m_performedMoveCount = 0;
	}

	CSwordAttackTable attackTable;
	int type = attackTable.GetCurrentComboAction(1 ,m_selectedCombo, m_performedMoveCount);
	FragmentIDAutoInit fragment = attackTable.GetAttackFragmentID(this, type);
	PlayAction(fragment, 0, false, 0, -1, 1);


	m_bBusy = true;
	GetEntity()->SetTimer(1, 700);
	*/

	CWeapon::StartFire();
}


void CSword::StartFire(int position)
{
	switch (position)
	{
	case 0: m_CombatStance->ApplyStance(e_CombatStanceVertical); break;
	case 1: m_CombatStance->ApplyStance(e_CombatStanceHorizontal); break;
	case 2: m_CombatStance->ApplyStance(e_CombatStancePricking); break;
	default: m_CombatStance->ApplyStance(e_CombatStanceHorizontal); break;
	}

	CSword::StartFire();
}


//-------------------------------------------------------------------
void CSword::StopFire()
{
	if (!IsOwnerClient() || !m_Holding || !m_bBusy || m_bNowAttack)
		return;

	CWeapon::StopFire();
	m_Holding->StopFire(this);
	float duration = m_Holding->GetFragmentPlayDuration(new TAction<SAnimationContext>(PP_PlayerAction, GetFragmentIds().release));
	GetEntity()->SetTimer(_HOLDING_RESET_STATUS, duration * 10);
	m_bNowAttack = true;

	//Код ниже нужен для определения направления удара при помощи мышки
	CMeleeAttackTable attackTable;
	float ang = m_MouseDir.GetAngle();
	int type = attackTable.GetAttackType(1, ang);// 1 - sword

	//Если силы не достаточно для удара
	CPlayerStatsManager* pStatsManager = g_pGame->GetPlayerStatsManager();
	if (pStatsManager != NULL)
	{
		CPlayerStat* pStrength = pStatsManager->GetStat(EPS_Strength);
		if (pStrength)
		{
			float currentValue = pStrength->GetStatBonus() + pStrength->GetStatValue();
			if (currentValue < 5)
				return;
			pStrength->SetValue(pStrength->GetStatValue() - 0);
		}
	}
}


//-------------------------------------------------------------------
void CSword::PerfomIntersection()
{
	//Если владелец не игрок то сразу выходим, дабы аи не юзали этот код
	if (!g_pGame->GetIGameFramework()->GetClientActor()->GetEntity() || !IsOwnerClient())
		return;

	m_bNowAttack = true;
	IEntity *pOwner = ((CPlayer*)g_pGame->GetIGameFramework()->GetClientActor())->GetEntity();

	//Vec3 vWeapon_dir_helper_top = CSpecialFunctions::GetBonePos("weapon_dir_helper_top", false, GetEntity());
	//Vec3 vWeaponPos = CSpecialFunctions::GetBonePos("MASTER_CON", false, GetEntity());
	Vec3 vWeaponPos = CSpecialFunctions::GetBonePos("weapon_bone", false, pOwner);

	ray_hit hit;
	Vec3 dir = gEnv->pSystem->GetViewCamera().GetViewdir();
	Vec3 camPos = gEnv->pSystem->GetViewCamera().GetPosition();
	gEnv->pPhysicalWorld->RayWorldIntersection(camPos, dir * _MELEE_RANGE_, ent_all, rwi_stop_at_pierceable | rwi_colltype_any | rwi_ignore_back_faces, &hit, 1, pOwner->GetPhysics());

	//Данный блок нужен для того, что бы избежать возможность атаки за стеной
	//Определяется, если до стены больше 1.2 метра, то длинна цилиндра = 1.2
	float length = Vec3(hit.pt - vWeaponPos).len();
	if (length > 1.2)
		length = 1.2;

	//Задаем параметры цилинда, определяющего удары
	if (cyl == NULL) cyl = new primitives::cylinder();
	cyl->axis = dir;//(vWeapon_dir_helper_top - vWeaponPos).normalize();
	//cyl.axis = (vWeaponPos - vWeapon_dir_helper_top).normalize();
	cyl->hh = length;
	cyl->center = vWeaponPos + cyl->axis*cyl->hh;
	cyl->r = 0.1f;

	//Отрисовка цилинда, если включен режим отрисовки в дебаге
	if (g_pGameCVars->td_draw == 1)
		gEnv->pRenderer->GetIRenderAuxGeom()->DrawCylinder(cyl->center, cyl->axis, cyl->r, cyl->hh, Vec3(255, 0, 0));

	int flags = ent_rigid | ent_sleeping_rigid | ent_independent | ent_static | ent_terrain | ent_water | ent_all;
	IPhysicalEntity *pIgnore = pOwner ? pOwner->GetPhysics() : 0; //Игнорируем себя
	geom_contact *contacts;
	intersection_params params;
	params.time_interval = 5000;

	//Данный метод определяет те сущности, которые вошли в цилиндр
	float n = gEnv->pPhysicalWorld->PrimitiveWorldIntersection(primitives::cylinder::type, cyl, Vec3(ZERO), flags, &contacts, 0,
		geom_colltype_foliage | geom_colltype_player, &params, 0, 0, &pIgnore, pIgnore ? 1 : 0);

	int intersectedEntitiesCount = 0;
	int surface = 0;

	//Бежим по циклу из пересеченных сущностей
	geom_contact *currentc = contacts;
	for (int i = 0; i < n; i++)
	{
		geom_contact *contact = currentc;
		if (contact != NULL)
		{
			int primId = contact->iPrim[0];
			IPhysicalEntity *pCollider = gEnv->pPhysicalWorld->GetPhysicalEntityById(primId);
			if (pCollider)
			{
				pe_action_impulse ai;
				ai.partid = currentc->iPrim[0];
				ai.point = currentc->pt;
				ai.ipart = currentc->id[1];
				ai.impulse = dir * 3;
				pCollider->Action(&ai);

				IEntity *pHitedEntity = gEnv->pEntitySystem->GetEntityFromPhysics(pCollider);
				if (pHitedEntity != NULL)
				{
					EntityId  collidedEntityId = pHitedEntity->GetId();
					if (collidedEntityId == LOCAL_PLAYER_ENTITY_ID) //если затронут игрок то игонорим
					{
						++currentc;
						continue;
					}

					if (hit.surface_idx != 0)
						surface = hit.surface_idx;
					else surface = 105;

					dir = currentc->pt - m_prevPt;
					dir.NormalizeSafe();
					CryLog("Dir %f %f %f", dir.x, dir.y, dir.z);
					Hit(currentc->pt, dir, currentc->n, pCollider, collidedEntityId, currentc->iPrim[1], 0, surface);

					intersectedEntitiesCount++;
					m_prevPt = contact->pt;
				}
			}
		}
		++currentc;
	}

	//Другие случаи удара: браш, боиды
	if (intersectedEntitiesCount == 0)
	{
		IPhysicalEntity *pCollider = hit.pCollider;
		if (pCollider != NULL)
		{
			pe_action_impulse ai;
			ai.partid = hit.ipart;
			ai.point = hit.pt;
			ai.impulse = hit.n;
			pCollider->Action(&ai);
		}

		PlayHitMaterialEffect(hit.pt, hit.n, hit.surface_idx);

		//Generate Hit
		CGameRules *pGameRules = g_pGame->GetGameRules();
		CRY_ASSERT_MESSAGE(pGameRules, "No game rules! Melee can not apply hit damage");

		IEntity* pTarget = hit.pCollider ? gEnv->pEntitySystem->GetEntityFromPhysics(hit.pCollider) : NULL;
		if (pGameRules && pTarget)
		{
			CryLog("Sword, name of collided entity: %s", pTarget->GetName());

			//Получаем статы для определени урона
			CPlayerStatsManager *pStatsManager = g_pGame->GetPlayerStatsManager();
			CPlayerStat* pStrength = pStatsManager->GetStat(EPS_Strength);
			float damage = pStrength ? pStrength->GetStatValue() : 300;

			int hitTypeID = 1;
			HitInfo info(GetOwnerId(), pTarget->GetId(), GetEntityId(),
				damage, 0.0f, hit.surface_idx, hit.ipart, hitTypeID, hit.pt, dir, hit.n);
			info.hitViaProxy = true;
			pGameRules->ClientHit(info);
		}


		m_Holding->SetStatus(e_HoldingStatusCancel);
	}
}

//-------------------------------------------------------------------
void CSword::Hit(const Vec3 &pt, const Vec3 &dir, const Vec3 &normal, IPhysicalEntity *pCollider, EntityId collidedEntityId,
	int partId, int ipart, int surfaceIdx)
{
	if (collidedEntityId == LOCAL_PLAYER_ENTITY_ID || !IsOwnerClient())
		return;

	int hitTypeID = 1;
	CActor *pOwnerActor = GetOwnerActor();

	if (pOwnerActor)
	{
		IActor* pTargetActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(collidedEntityId);
		IEntity* pTarget = pTargetActor ? pTargetActor->GetEntity() : gEnv->pEntitySystem->GetEntity(collidedEntityId);
		IEntity* pOwnerEntity = pOwnerActor->GetEntity();
		IAIObject* pOwnerAI = pOwnerEntity->GetAI();
		float damageScale = cos_tpl(DEG2RAD(g_pGameCVars->pl_melee.angle_limit_from_behind)) * 2;

		// Send target stimuli
		if (!gEnv->bMultiplayer)
		{
			IAISystem *pAISystem = gEnv->pAISystem;
			ITargetTrackManager *pTargetTrackManager = pAISystem ? pAISystem->GetTargetTrackManager() : NULL;
			if (pTargetTrackManager && pOwnerAI)
			{
				IAIObject *pTargetAI = pTarget ? pTarget->GetAI() : NULL;
				if (pTargetAI)
				{
					const tAIObjectID aiOwnerId = pOwnerAI->GetAIObjectID();
					const tAIObjectID aiTargetId = pTargetAI->GetAIObjectID();

					TargetTrackHelpers::SStimulusEvent eventInfo;
					eventInfo.vPos = pt;
					eventInfo.eStimulusType = TargetTrackHelpers::eEST_Generic;
					eventInfo.eTargetThreat = AITHREAT_AGGRESSIVE;
					pTargetTrackManager->HandleStimulusEventForAgent(aiTargetId, aiOwnerId, "MeleeHit", eventInfo);
					pTargetTrackManager->HandleStimulusEventInRange(aiOwnerId, "MeleeHitNear", eventInfo, 5.0f);
				}
			}
		}

		//Check if is a friendly hit, in that case FX and Hit will be skipped
		bool isFriendlyHit = (pOwnerEntity && pTarget) ? IsFriendlyHit(pOwnerEntity, pTarget) : false;

		if (!isFriendlyHit && pTarget)
		{
			CPlayer * pAttackerPlayer = pOwnerActor->IsPlayer() ? static_cast<CPlayer*>(pOwnerActor) : NULL;

			//Получаем статы для определени урона
			CPlayerStatsManager *pStatsManager = g_pGame->GetPlayerStatsManager();
			CPlayerStat* pStrength = pStatsManager->GetStat(EPS_Strength);
			float damage = pStrength ? pStrength->GetStatValue() : 20;

			//Generate Hit
			CGameRules *pGameRules = g_pGame->GetGameRules();
			CRY_ASSERT_MESSAGE(pGameRules, "No game rules! Melee can not apply hit damage");

			if (pGameRules)
			{
				if (GetOwnerId() == pTarget->GetId()) //Если урон самому себе
					return;

				HitInfo info(GetOwnerId(), pTarget->GetId(), GetEntityId(), damage, 0.0f, surfaceIdx, partId, hitTypeID, pt, dir, normal);
				pGameRules->ClientHit(info);
				//CryLog("[Combat system]: Shooter: %d target: %d", GetOwnerId(), pTarget->GetId());
			}

			float mass = GetParams().mass;
			pe_action_impulse action;
			action.impulse = dir;
			action.angImpulse.Set(mass, mass, 0);
			pCollider->Action(&action);

			if (pAttackerPlayer && pAttackerPlayer->IsClient())
			{
				const Vec3 posOffset = (pt - pTarget->GetWorldPos());
				SMeleeHitParams params;
				params.m_boostedMelee = false;
				params.m_hitNormal = normal;
				params.m_hitOffset = posOffset;
				params.m_surfaceIdx = surfaceIdx;
				params.m_targetId = pTarget->GetId();
				//pAttackerPlayer->OnMeleeHit(params); //Здесь идет левое проигрывание эффекта
			}
			PlayHitMaterialEffect(pt, normal, surfaceIdx);
		}

		if (pTarget)
		{
			CActor *pCTargetActor = static_cast<CActor*>(pTargetActor);
			CPlayer* pTargetPlayer = (pTargetActor && pTargetActor->IsPlayer()) ? static_cast<CPlayer*>(pTargetActor) : NULL;
			if (pTargetPlayer && pTargetPlayer->IsClient())
			{
				pTargetPlayer->TriggerMeleeReaction();
			}
		}
	}
}

//-------------------------------------------------------------------
bool CSword::IsFriendlyHit(IEntity* pShooter, IEntity* pTarget)
{
	IActor* pAITarget = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pTarget->GetId());
	if (pAITarget && pTarget->GetAI() && !pTarget->GetAI()->IsHostile(pShooter->GetAI(), false))
	{
		return true;
	}
	return false;
}

//-------------------------------------------------------------------
void CSword::PlayHitMaterialEffect(const Vec3 &position, const Vec3 &normal, int surfaceIdx)
{
	//Play Material FX
	const char* meleeFXType = "melee";//"melee_combat";

	IMaterialEffects* pMaterialEffects = gEnv->pGame->GetIGameFramework()->GetIMaterialEffects();
	//если сурф индекс это проксси
	if (surfaceIdx == 129)
		surfaceIdx = 136; //То здесь нужно задать нейтральный эффект

	TMFXEffectId effectId = pMaterialEffects->GetEffectId(meleeFXType, surfaceIdx);

	if (effectId != InvalidEffectId && m_bHitSoundPlayed == false)
	{
		m_bHitSoundPlayed = true;
		SMFXRunTimeEffectParams params;
		params.pos = position;
		params.normal = normal;
		//params.playflags = MFX_PLAY_ALL | MFX_DISABLE_DELAY;
		pMaterialEffects->ExecuteEffect(effectId, params);		
	}
}



//-------------------------------------------------------------------
int CSword::GetComboMoveCount(int combo)
{
	//switch (combo)
	//{
	//case e_Left_Right_Horizontal_Top:return 3; //3 так как удар состоит из 3 действий
	//case e_Left_Right_Horizontal_Bottom: return 3;
	//case e_Front_Left_Horizontal: return 2;
	//}
	return 0;
}

//-------------------------------------------------------------------
void CSword::DebugDraw()
{
	IEntity *pOwner = ((CPlayer*)g_pGame->GetIGameFramework()->GetClientActor())->GetEntity();
	if (!pOwner || !GetOwner())// || !IsOwnerClient())
		return;

	// Данная кость устанавливается на конце оружия "weapon_dir_helper_top"
	//Vec3 vWeaponPos = CSpecialFunctions::GetBonePos("MASTER_CON", false, GetEntity()); 
	Vec3 vWeaponPos = CSpecialFunctions::GetBonePos("weapon_bone", false, pOwner);
	Vec3 dir = gEnv->pSystem->GetViewCamera().GetViewdir();
	Vec3 camPos = gEnv->pSystem->GetViewCamera().GetPosition();
	ray_hit hit;

	gEnv->pPhysicalWorld->RayWorldIntersection(camPos, dir * _MELEE_RANGE_, ent_all, rwi_stop_at_pierceable | rwi_colltype_any, &hit, 1);

	//Перераспределяем длинну 
	float length = Vec3(hit.pt - vWeaponPos).len();
	if (length > 1.3)
		length = 1.3;

	primitives::cylinder cyl;
	cyl.axis = dir;
	cyl.hh = length;
	cyl.center = vWeaponPos + cyl.axis*cyl.hh;
	cyl.r = 0.1f;

	//Отображение в сеточном виде
	SAuxGeomRenderFlags flags;
	flags.SetFillMode(e_FillModeWireframe);
	flags.SetCullMode(e_CullModeNone);

	//Ставим флаги и отрисовываем
	gEnv->pRenderer->GetIRenderAuxGeom()->SetRenderFlags(flags);
	gEnv->pRenderer->GetIRenderAuxGeom()->DrawCylinder(cyl.center, cyl.axis, cyl.r, cyl.hh, Vec3(255, 0, 0));
}


void CSword::ClearTag(uint tag)
{
	IActionController *pActionController = GetActionController();
	if (!pActionController)
		return;

	SAnimationContext &animContext = pActionController->GetContext();
	if (animContext.state.IsSet(tag) == true)
		animContext.state.Set(tag, false);
}

void CSword::ClearAllTags()
{
	ClearTag(PlayerMannequin.tagIDs.swing);
	ClearTag(PlayerMannequin.tagIDs.hold);
	ClearTag(PlayerMannequin.tagIDs.release);
	ClearTag(PlayerMannequin.tagIDs.cancel);
	ClearTag(PlayerMannequin.tagIDs.leftSide);
	ClearTag(PlayerMannequin.tagIDs.rightSide);
	ClearTag(PlayerMannequin.tagIDs.up);
	ClearTag(PlayerMannequin.tagIDs.down);
	ClearTag(PlayerMannequin.tagIDs.front);
	ClearTag(PlayerMannequin.tagIDs.hit);
	ClearTag(PlayerMannequin.tagIDs.miss);
}


void CSword::SetBusy(bool busy)
{
	m_bBusy = busy;
	CWeapon::SetBusy(busy);
	if (busy == true)
		CryLog("Set Weapon Busy True");
	else CryLog("Set Weapon Busy False");
}