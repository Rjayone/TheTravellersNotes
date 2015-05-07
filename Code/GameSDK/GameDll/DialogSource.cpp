#include "StdAfx.h"
#include "DialogSource.h"
#include "Player.h"
#include "GameActions.h"
#include "IFlashUI.h"

CDialogAbility::CDialogAbility()
{
	bStartDialog = false;

	IGameFramework *pGameFramework = g_pGame->GetIGameFramework();
	if (pGameFramework)
		pGameFramework->RegisterListener(this, "DialogAbility", FRAMEWORKLISTENERPRIORITY_DEFAULT);

	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pAmMgr != NULL)
		pAmMgr->AddExtraActionListener(this);
}

void CDialogAbility::OnPostUpdate(float fDeltaTime)
{
	IEntity *pEntity = GetEntity();
	if (!pEntity)
		return;

	//CryLog("You can start dialog");
	//Из ST нужно получить тип диалога(Стандартный, торговец и тд), имя нпс с которым взаимодействуем, и возможно ли взаимодействие
	IScriptTable *pAIStript = pEntity->GetScriptTable();
	if (!pAIStript)
		return;

	SmartScriptTable propertiesTable;
	SmartScriptTable dialogPropertiesTable;


	bool hasProperties = pAIStript->GetValue("Properties", propertiesTable);
	if (!hasProperties)
		return;

	const bool hasPropertiesTable = propertiesTable->GetValue("DialogProperties", dialogPropertiesTable);
	if (!hasPropertiesTable)
		return;

	bool bDialogEnable;
	const char* sDialogType;
	const char* sAIName;
	bool bEnemy;

	dialogPropertiesTable->GetValue("bDialogEnable", bDialogEnable);
	dialogPropertiesTable->GetValue("sAIName", sAIName);
	dialogPropertiesTable->GetValue("eDialogType", sDialogType);
	dialogPropertiesTable->GetValue("Faction", bEnemy);

	if (!bDialogEnable /*|| bEnemy */ || !sAIName || !sDialogType) //Если диалог недоступен то прерываем
	{
		CryLog("Dialog not available");
		return;
	}

	if (bStartDialog)
	{
		//pDialogSystem->StartDialog(sAIName, sDialogType);
		CryLog("Dialog has been started");
		bStartDialog = false;
	}

	if (gEnv->IsEditorGameMode())
	{
		IUIElement *pUI = gEnv->pFlashUI->GetUIElement("DialogMenu");
		if (pUI)
		{
			pUI->SetVisible(true);
			pUI->SetVisible(false);
		}
	}
}

IEntity* CDialogAbility::GetEntity()
{
	ray_hit ray;
	Vec3  cameraPos = gEnv->pSystem->GetViewCamera().GetPosition() + gEnv->pSystem->GetViewCamera().GetViewdir();
	Vec3 cameraDir = gEnv->pSystem->GetViewCamera().GetViewdir();

	gEnv->pPhysicalWorld->RayWorldIntersection(cameraPos, cameraDir * 3, ent_all, rwi_stop_at_pierceable | rwi_colltype_any, &ray, 1);

	if (IEntity *pEntity = gEnv->pEntitySystem->GetEntityFromPhysics(ray.pCollider))
		return pEntity;
	return NULL;
}

void CDialogAbility::OnAction(const ActionId& action, int activationMode, float value)
{
	const CGameActions &actions = g_pGame->Actions();
	if (actions.use == action)
	{
		bStartDialog = true;
	}
}