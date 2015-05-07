#include "StdAfx.h"
#include "Game.h"
#include "CombatStance.h"
#include "Player.h"
#include "Holding.h"
#include "GameActions.h"

float timer = 0;
void StartTimer()
{
	timer = gEnv->pTimer->GetCurrTime();
}

CCombatStance::CCombatStance()
{
	m_Stance = e_CombatStanceVertical;
	m_pItem = NULL;

	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pAmMgr) pAmMgr->AddExtraActionListener(this);

	m_pCombatTarget = new CCombatTarget();
}

CCombatStance::~CCombatStance()
{
	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pAmMgr)	pAmMgr->RemoveExtraActionListener(this);
}

//----------------------------------------------------------------
void CCombatStance::Init(CItem* pItem)
{
	if (pItem != NULL)
		m_pItem = pItem;
	ClearAllStanceTags();
}

//----------------------------------------------------------------
void CCombatStance::Update()
{
}

//----------------------------------------------------------------
void CCombatStance::OnAction(const ActionId& action, int activationMode, float value)
{
	m_pItem = (CItem*)g_pGame->GetIGameFramework()->GetClientActor()->GetCurrentItem();
	if (m_pItem != NULL)
	{
		if (m_pItem->GetHoldingStatus() != NULL)
		{
			//Проверка, если мы в статусе удержания меча, то стойку сменить нельзя
			int holdingStatus = m_pItem->GetHoldingStatus()->GetStatus();
			if (holdingStatus == e_HoldingStatusHold)
				return;

			if (action == g_pGameActions->stance_verical)
				ApplyStance(e_CombatStanceVertical);
			if (action == g_pGameActions->stance_horizontal)
				ApplyStance(e_CombatStanceHorizontal);
		}
	}
	if (action == g_pGameActions->lookAt && m_pCombatTarget)
	{
		if (activationMode == eAAM_OnPress)
		{
			m_pCombatTarget->GetScreenTargets();
			m_pCombatTarget->LookAtClosestTarget();
			g_pGameActions->FilterNoMouseMove()->Enable(true);

			StartTimer();

		}
		if (activationMode == eAAM_OnHold)
		{
			if (timer + 0.5 < gEnv->pTimer->GetCurrTime())
			{
				m_pCombatTarget->ForceLookAt(0);
				g_pGameActions->FilterNoMouseMove()->Enable(false);
			}
		}
	}
	if (action == g_pGameActions->defensive_stance)
	{
		if (activationMode == eAAM_OnPress || activationMode == eAAM_OnHold)
		{
			ApplyStance(e_CombatStanceDefence);
		}
		else if (activationMode == eAAM_OnRelease)
		{
			ApplyStance(e_CombatStanceVertical);
		}
	}
}


//----------------------------------------------------------------
void CCombatStance::ApplyStance(int stance)
{
	if (!g_pGame->GetIGameFramework()->GetClientActor())
		return;

	CItem* pItem = (CItem*)g_pGame->GetIGameFramework()->GetClientActor()->GetCurrentItem();
	if (!pItem)
		return;

	if (pItem->GetOwnerId() == LOCAL_PLAYER_ENTITY_ID)
	{
		//Здесь нужно дописать проверки на то, может ли игрок защищаться прямо сейчас
		//Защиты нет, если игрок безоружен, потерял равновесие от полученного удара или игрок производит атаку

		SetStance(stance);
		m_pItem = pItem;
		FragmentIDAutoInit fragment = pItem->GetFragmentIds().stance;
		pItem->PlayAction(fragment, 0, true, 0, -1, 1);
	}
}

//----------------------------------------------------------------
void CCombatStance::SetTag(uint32 id, bool set)
{
	if (!g_pGame->GetIGameFramework()->GetClientActor())
		return;

	CItem* pItem = (CItem*)g_pGame->GetIGameFramework()->GetClientActor()->GetCurrentItem();
	if (!pItem)
		return;

	m_pItem = pItem;
	IActionController *pActionController = m_pItem ? m_pItem->GetActionController() : NULL;
	if (!pActionController)
		return;

	SAnimationContext &animContext = pActionController->GetContext();
	animContext.state.Set(id, set);
	m_pItem->SetFragmentTags(animContext.state);
}

//----------------------------------------------------------------
void CCombatStance::SetStance(int stance)
{
	m_Stance = stance;

	if (stance == e_CombatStanceVertical)	SetTag(PlayerMannequin.tagIDs.vertical, true);
	if (stance == e_CombatStanceHorizontal)	SetTag(PlayerMannequin.tagIDs.horizontal, true);
	if (stance == e_CombatStancePricking)	SetTag(PlayerMannequin.tagIDs.pricking, true);
	if (stance == e_CombatStanceDefence)	SetTag(PlayerMannequin.tagIDs.defence, true);
}

//----------------------------------------------------------------
void CCombatStance::ClearAllStanceTags()
{
	IActionController *pActionController = m_pItem ? m_pItem->GetActionController() : NULL;
	if (!pActionController)
		return;

	SAnimationContext &animContext = pActionController->GetContext();
	animContext.state.Set(PlayerMannequin.tagIDs.vertical, false);
	animContext.state.Set(PlayerMannequin.tagIDs.diagonal, false);
	animContext.state.Set(PlayerMannequin.tagIDs.horizontal, false);
	animContext.state.Set(PlayerMannequin.tagIDs.pricking, false);
}