#include "StdAfx.h"
#include "DialogDispatcher.h"
#include "DialogSystem.h"
#include "MouseUtils.h"

//-------------------------------------------------------------------
CDialogDispatcher::CDialogDispatcher()
{
	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pAmMgr != NULL)
		pAmMgr->AddExtraActionListener(this);

	m_bCanStartDialog = false;
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
	IsAI();
	if (m_bCanStartDialog == true)
		g_pGame->GetDialogSystem()->CanDialog(GetTarget());
	
	


	//IScriptTable *pAIStript = GetTarget()->GetScriptTable();
	//if (!pAIStript || !IsAI())
	//	return;

	//SmartScriptTable propertiesTable;
	//SmartScriptTable dialogPropertiesTable;
	//bool hasProperties = pAIStript->GetValue("Properties", propertiesTable);
	//if (!hasProperties)
	//	return;

	//const bool hasPropertiesTable = propertiesTable->GetValue("DialogProperties", dialogPropertiesTable);
	//if (!hasPropertiesTable)
	//	return;

	//bool bDialogEnable, bEnemy;
	//const char* sDialogType;
	//const char* sAIName;

	//dialogPropertiesTable->GetValue("bDialogEnable", bDialogEnable);
	//dialogPropertiesTable->GetValue("sAIName", sAIName);
	//dialogPropertiesTable->GetValue("eDialogType", sDialogType);
	//dialogPropertiesTable->GetValue("Faction", bEnemy);

	//if (!bDialogEnable /*|| bEnemy */ || !sAIName || !sDialogType) //≈сли диалог недоступен то прерываем
	//{
	//	CryLog("Dialog not available");
	//	return;
	//}
}

//-------------------------------------------------------------------
void CDialogDispatcher::OnAction(const ActionId& action, int activationMode, float value)
{
	const CGameActions &actions = g_pGame->Actions();
	if (actions.use == action && IsAI() == true)
	{
		g_pGame->GetDialogSystem()->StartDialog(GetTarget(), true);
	}
}