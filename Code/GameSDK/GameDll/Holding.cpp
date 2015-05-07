#include "StdAfx.h"
#include "Holding.h"
#include "FireMode.h"
#include "Player.h"
#include "AttackTable.h"
#include "GameActions.h"


#ifndef _HOLDING_RESET_STATUS
 #define _HOLDING_RESET_STATUS 4
#endif
#define _RELEASE_STATUS 1 << 4
#define _START_HOLDING_FRAGMENT 1 << 5


#define CLEAR_TAG(tag){\
	IActionController *pActionController = m_pItem ? m_pItem->GetActionController() : NULL; \
	if (!pActionController) \
		return; \
	SAnimationContext &animContext = pActionController->GetContext(); \
	if (animContext.state.IsSet(tag)) \
		animContext.state.Set(tag, false); \
}

CHolding::CHolding()
{
	SetStatus(e_HoldingTypeIdle);
	m_bShortAttack = true;
	m_bHolding = false;
	m_pItem = NULL;

	m_pMouseUtils = new CMouseUtils();

	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pAmMgr)
	{
		pAmMgr->AddExtraActionListener(this);
	}
}

//-------------------------------------------------------------------
CHolding::~CHolding()
{
	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pAmMgr)	pAmMgr->RemoveExtraActionListener(this);
}

//-------------------------------------------------------------------
IEntity* CHolding::GetEntity()
{
	CPlayer* pPlayer = static_cast<CPlayer*>(g_pGame->GetIGameFramework()->GetClientActor());
	if (pPlayer != NULL)
	{
		EntityId itemId = pPlayer->GetInventory()->GetCurrentItem();
		return gEnv->pEntitySystem->GetEntity(itemId);
	}
	return NULL;
}

//-------------------------------------------------------------------
bool CHolding::Init(CItem *pItem)
{
	m_pItem = pItem;
	return true;
}

//-------------------------------------------------------------------
void CHolding::OnAction(const ActionId& action, int activationMode, float value)
{
	//activateMode
	//4 - hold
	//1 - click
	//2 - release
	if (action == g_pGameActions->reload && m_bHolding)
	{
		CancelHolding();
	}
}

//-------------------------------------------------------------------
void CHolding::Update()
{
	if (m_bHolding == true)
	{
		float clickTime = m_pMouseUtils->CalculateClickTime();
		if (clickTime > 0.0f && clickTime <= 0.12f)// && !m_pItem->IsBusy()) // лик тайм меньше 0.12 и оружие не зан€то
		{											  //Ќе проходит условие зан€тости
			m_bShortAttack = true;
			DefaultAttack();
		}
		else if (m_bShortAttack)
		{
			PerfomSwing(m_pItem, 0);
			m_bShortAttack = false;
		}
	}
}

//-------------------------------------------------------------------
void CHolding::ProcessEvent(SEntityEvent& event)
{
	switch (event.event)
	{
	case ENTITY_EVENT_TIMER:
		if(GetEntity()) GetEntity()->KillTimer(event.nParam[0]);
		if (event.nParam[0] == _RELEASE_STATUS)
		{
			m_pItem->SetBusy(false);
		}
		if (event.nParam[0] == _HOLDING_RESET_STATUS)
		{
			SetTag(PlayerMannequin.tagIDs.release, false);
			SetStatus(e_HoldingTypeIdle);
			ClearAllHoldingTags();
			m_pItem->SetBusy(false);
			//CryLog("Busy = false");
		}
		if (event.nParam[0] == _START_HOLDING_FRAGMENT)
		{
			OnHold(0);
		}
		break;
	};
}

//-------------------------------------------------------------------
void CHolding::PerfomSwing(CItem *pItem, int stance)
{
	//“ак же стоит проверить был ли это шорт клик
	if (!pItem || pItem->IsBusy())
		return;

	int weaponType = pItem->GetWeaponType();

	pItem->SetBusy(true);
	m_pItem = pItem;
	SetStatus(e_HoldingStatusSwing);
	SetTag(PlayerMannequin.tagIDs.swing, true);

	if (weaponType == e_WeaponTypeSword || weaponType == e_WeaponTypeAxe)
	{
		CMeleeAttackTable table;
		SetTag(table.GetRandomTagDirection(pItem), true);
	}

	SMannequinItemParams::FragmentIDs fragments = pItem->GetFragmentIds();
	pItem->PlayAction(fragments.swing, 0);
	int playTime = pItem->GetCurrentAnimationTime(2);
	CryLog("Swing Play time: %d", playTime);
	pItem->GetEntity()->SetTimer(_START_HOLDING_FRAGMENT, playTime-30);
	//OnHold(0);

}

//-------------------------------------------------------------------
void CHolding::OnHold(int stance)
{
	if (m_nStatus == e_HoldingStatusShortClick)// || !m_pItem->IsBusy())
		return;

	m_nStatus = e_HoldingStatusHold;
	SetTag(PlayerMannequin.tagIDs.swing, false);//отключаем тег свинг
	m_pItem->PlayAction(m_pItem->GetFragmentIds().holding, 0, true);
}


//-------------------------------------------------------------------
void CHolding::PerfomRelease()
{
	float clickTime = m_pMouseUtils->OnClickUp(true);
	if (clickTime <= 0.1f)
		SetStatus(e_HoldingStatusShortClick);

	if (m_nStatus == e_HoldingStatusCancel /*|| m_nStatus == e_HoldingStatusShortClick*/ || !GetEntity())
	{
		SetStatus(e_HoldingTypeIdle);
		return;
	}
	if (!m_pItem->IsBusy() || m_nStatus == e_HoldingStatusRelease)
		return;

	SetStatus(e_HoldingStatusRelease);
	SetTag(PlayerMannequin.tagIDs.release, true);

	IActionPtr pAction = new TAction< SAnimationContext >(PP_PlayerActionUrgent, m_pItem->GetFragmentIds().release);
	CPlayer* pPlayer = (CPlayer*)g_pGame->GetIGameFramework()->GetClientActor();
	pPlayer->GetAnimatedCharacter()->GetActionController()->Queue(*pAction);

	float fragmentDuration, transitionDuration;
	pPlayer->GetAnimatedCharacter()->GetActionController()->QueryDuration(*pAction, fragmentDuration, transitionDuration);
	GetEntity()->SetTimer(_HOLDING_RESET_STATUS, fragmentDuration * 100 + transitionDuration);/*300 это врем€ в милисек в течении которого идет урон*/
}


//-------------------------------------------------------------------
void CHolding::CancelHolding()
{
	m_bHolding = false;
	m_nStatus = e_HoldingStatusCancel;

	//2 строчки ниже не удал€ю, ибо могут использоватьс€ в дальшейнем
	SetTag(PlayerMannequin.tagIDs.hold, false);
	SetTag(PlayerMannequin.tagIDs.cancel, true);

	m_pItem->PlayAction(m_pItem->GetFragmentIds().cancel);
}

//-------------------------------------------------------------------
void CHolding::DefaultAttack(int stance)
{
	if (m_pItem->IsBusy())
		return;

	CryLog("[CHolding]: Short Click");
	m_pItem->SetBusy(true);
	m_bHolding = false;
	SetStatus(e_HoldingStatusShortClick);

	int weaponType = m_pItem->GetWeaponType();
	if (weaponType == e_WeaponTypeSword || weaponType == e_WeaponTypeAxe)
	{
		CMeleeAttackTable table;
		SetTag(table.GetRandomTagDirection(m_pItem), true);
	}
	m_pItem->PlayAction(m_pItem->GetFragmentIds().hit, 0, false, 0, -1, 1);
	m_pItem->GetEntity()->SetTimer(_HOLDING_RESET_STATUS, 700);
}

//-------------------------------------------------------------------
void CHolding::SetStatus(EHoldingStatus status)
{
	m_nStatus = status;
}

//-------------------------------------------------------------------
void CHolding::StartFire(CItem *pItem)
{
	m_pItem = pItem;
	m_bHolding = true;
	m_pMouseUtils->InitClickTime();

	int weaponType = pItem->GetWeaponType();
	PerfomSwing(pItem, weaponType);
}

//-------------------------------------------------------------------
void CHolding::StopFire(CItem *pItem)
{
	m_pItem = pItem;
	m_bHolding = false;
	m_pMouseUtils->OnClickUp();
	PerfomRelease();
}

//-------------------------------------------------------------------
void CHolding::SetTag(uint32 id, bool set)
{
	m_pItem = (CItem*)g_pGame->GetIGameFramework()->GetClientActor()->GetCurrentItem();
	IActionController *pActionController = m_pItem ? m_pItem->GetActionController() : NULL;
	if (!pActionController)
		return;

	SAnimationContext &animContext = pActionController->GetContext();
	animContext.state.Set(id, set);
	m_pItem->SetFragmentTags(animContext.state);
}

//-------------------------------------------------------------------
void CHolding::ClearAllHoldingTags()
{
	CLEAR_TAG(PlayerMannequin.tagIDs.swing);
	CLEAR_TAG(PlayerMannequin.tagIDs.hold);
	CLEAR_TAG(PlayerMannequin.tagIDs.release);
	CLEAR_TAG(PlayerMannequin.tagIDs.cancel);
	CLEAR_TAG(PlayerMannequin.tagIDs.leftSide);
	CLEAR_TAG(PlayerMannequin.tagIDs.rightSide);
	CLEAR_TAG(PlayerMannequin.tagIDs.up);
	CLEAR_TAG(PlayerMannequin.tagIDs.down);
	CLEAR_TAG(PlayerMannequin.tagIDs.front);
	CLEAR_TAG(PlayerMannequin.tagIDs.hit);
	CLEAR_TAG(PlayerMannequin.tagIDs.miss);
}