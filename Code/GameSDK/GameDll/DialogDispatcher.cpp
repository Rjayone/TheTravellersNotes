#include "StdAfx.h"
#include "DialogDispatcher.h"
#include "DialogSystem.h"
#include "MouseUtils.h"
#include "DialogSystem.h"
#include "IAIObject.h"

//-------------------------------------------------------------------
CDialogDispatcher::CDialogDispatcher()
{
	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pAmMgr != NULL)
		pAmMgr->AddExtraActionListener(this);

	m_bCanStartDialog = false;
	m_pDialogSystem = g_pGame->GetDialogSystem();
}

//-------------------------------------------------------------------
CDialogDispatcher::~CDialogDispatcher()
{
	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pAmMgr != NULL)
		pAmMgr->RemoveExtraActionListener(this);
}

//-------------------------------------------------------------------
bool CDialogDispatcher::IsAI()
{
	IEntity *pEntity = CMouseUtils::GetMouseEntity(2.0f);
	IUIElement* pDot = gEnv->pFlashUI->GetUIElement("Dot");
	if (m_pDialogSystem == NULL)
		m_pDialogSystem = g_pGame->GetDialogSystem();
	if (pEntity != NULL)
	{
		if (pEntity->GetAI())
		{
			m_bCanStartDialog = true;
			return true;
		}
	}
	else
	{
		m_bCanStartDialog = false;
		if (pDot)
			pDot->CallFunction("MouseOut");
		return false;
	}
}

//-------------------------------------------------------------------
IEntity* CDialogDispatcher::GetTarget()
{
	return CMouseUtils::GetMouseEntity(2.0f);
}

//-------------------------------------------------------------------
void CDialogDispatcher::Update()
{
	if(!IsAI())
		return;
	if (m_bCanStartDialog == true && !GetTarget()->GetAI()->IsHostile(gEnv->pEntitySystem->GetEntity(LOCAL_PLAYER_ENTITY_ID)->GetAI(), false))
		g_pGame->GetDialogSystem()->CanDialog(GetTarget());
}

//-------------------------------------------------------------------
void CDialogDispatcher::OnAction(const ActionId& action, int activationMode, float value)
{
	const CGameActions &actions = g_pGame->Actions();
	if (actions.use == action && activationMode == eAAM_OnRelease && IsAI() == true)
	{
		g_pGame->GetDialogSystem()->WillStartDialog(GetTarget());
	}
}