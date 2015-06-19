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
		if (clickTime > 0.0f && clickTime <= 0.12f)// && !m_pItem->IsBusy()) //Клик тайм меньше 0.12 и оружие не занято
		{											  //Не проходит условие занятости
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
		if (event.nParam[0] == _RELEASE_STATUS && m_pItem)
		{
			m_pItem->SetBusy(false);
		}
		if (event.nParam[0] == _HOLDING_RESET_STATUS && m_pItem)
		{
			SetStatus(e_HoldingTypeIdle);
			ClearAllHoldingTags();
			m_pItem->SetBusy(false);
		}
		if (event.nParam[0] == _START_HOLDING_FRAGMENT)
		{
			//Если длительность клика < чем таймер на включение холда.
			//В этом случаи релиз пройдет раньше холдинга, поэтому ретурн
			if (m_nStatus == e_HoldingStatusRelease) 
				return;
			//CryLog("Holding");
			//OnHold(0);
		}
		break;
	};
}

//-------------------------------------------------------------------
void CHolding::PerfomSwing(CItem *pItem, int stance)
{
	//Занято ли оружие
	if (!pItem || pItem->IsBusy())
		return;

	int weaponType = pItem->GetWeaponType();
	m_pItem = pItem;
	m_pItem->SetBusy(true);
	SetStatus(e_HoldingStatusSwing);
	SetTag(PlayerMannequin.tagIDs.swing, true);

	if (weaponType == e_WeaponTypeSword || weaponType == e_WeaponTypeAxe)
	{
		CMeleeAttackTable table;
		SetTag(table.GetRandomTagDirection(pItem), true);
	}

	IActionPtr pAction = new TAction<SAnimationContext>(PP_PlayerAction, m_pItem->GetFragmentIds().swing);
	CPlayer* pPlayer = (CPlayer*)g_pGame->GetIGameFramework()->GetClientActor();
	m_pItem->PlayAction(m_pItem->GetFragmentIds().swing, 0, true);

	float fragmentDuration, transitionDuration;
	pPlayer->GetAnimatedCharacter()->GetActionController()->QueryDuration(*pAction, fragmentDuration, transitionDuration);
	if (fragmentDuration * 100 + transitionDuration <= 0)
		fragmentDuration = 6.0f;

	//По таймеру будет вызван холдинг да бы не было перекрытия анимок
	pItem->GetEntity()->SetTimer(_START_HOLDING_FRAGMENT, fragmentDuration*100 + transitionDuration);
	CryLog("Swing");

}

//-------------------------------------------------------------------
void CHolding::OnHold(int stance)
{
	//if (m_nStatus == e_HoldingStatusShortClick)
	//	return;

	//
	//SetStatus(e_HoldingStatusHold);
	//SetTag(PlayerMannequin.tagIDs.swing, false);//отключаем тег свинг
	//SetTag(PlayerMannequin.tagIDs.hold, true);
	//CPlayer* pPlayer = (CPlayer*)g_pGame->GetIGameFramework()->GetClientActor();
	//m_pItem->PlayAction(m_pItem->GetFragmentIds().holding);
}


//-------------------------------------------------------------------
void CHolding::PerfomRelease()
{
	//float clickTime = m_pMouseUtils->OnClickUp(true);
	//if (clickTime <= 0.1f)
	//	SetStatus(e_HoldingStatusShortClick);

	if (m_nStatus == e_HoldingStatusCancel || !GetEntity())
	{
		SetStatus(e_HoldingTypeIdle);
		return;
	}
	if (!m_pItem->IsBusy() || m_nStatus == e_HoldingStatusRelease)
		return;

	CryLog("Release");
	SetStatus(e_HoldingStatusRelease);
	SetTag(PlayerMannequin.tagIDs.release, true);
	SetTag(PlayerMannequin.tagIDs.swing, false);//отключаем тег свинг
	SetTag(PlayerMannequin.tagIDs.hold, false);

	IActionPtr pAction = new TAction< SAnimationContext >(PP_PlayerAction, m_pItem->GetFragmentIds().release);
	CPlayer* pPlayer = (CPlayer*)g_pGame->GetIGameFramework()->GetClientActor();
	m_pItem->PlayAction(m_pItem->GetFragmentIds().release);

	float fragmentDuration, transitionDuration;
	pPlayer->GetAnimatedCharacter()->GetActionController()->QueryDuration(*pAction, fragmentDuration, transitionDuration);
	if (fragmentDuration * 100 + transitionDuration <= 0)
		fragmentDuration = 6.0f;
	GetEntity()->SetTimer(_HOLDING_RESET_STATUS, fragmentDuration * 100 + transitionDuration);/*300 это время в милисек в течении которого идет урон*/
}


//-------------------------------------------------------------------
void CHolding::CancelHolding()
{
	m_bHolding = false;
	m_nStatus = e_HoldingStatusCancel;

	//2 строчки ниже не удаляю, ибо могут использоваться в дальшейнем
	SetTag(PlayerMannequin.tagIDs.hold, false);
	SetTag(PlayerMannequin.tagIDs.cancel, true);

	m_pItem->PlayAction(m_pItem->GetFragmentIds().cancel);
}

//-------------------------------------------------------------------
void CHolding::DefaultAttack(int stance)
{
	if (m_pItem->IsBusy())
		return;

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
	IActionPtr pAction = new TAction< SAnimationContext >(PP_PlayerAction, m_pItem->GetFragmentIds().hit);
	CPlayer* pPlayer = (CPlayer*)g_pGame->GetIGameFramework()->GetClientActor();

	float fragmentDuration, transitionDuration;
	pPlayer->GetAnimatedCharacter()->GetActionController()->QueryDuration(*pAction, fragmentDuration, transitionDuration);
	if (fragmentDuration * 100 + transitionDuration <= 0)
		fragmentDuration = 6.0f;
	GetEntity()->SetTimer(_HOLDING_RESET_STATUS, fragmentDuration * 100 + transitionDuration);/*300 это время в милисек в течении которого идет урон*/
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