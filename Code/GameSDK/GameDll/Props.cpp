#include "StdAfx.h"
#include "Props.h"
#include "Game.h"
#include <IFlashUI.h>
#include "GameActions.h"


#define FILE_PATH "GameSDK/Objects/box.cgf"

CProps::CProps()
{
	
	IGameFramework *pGameFramework = g_pGame->GetIGameFramework();
	if (pGameFramework)
		pGameFramework->RegisterListener(this, "Chair", FRAMEWORKLISTENERPRIORITY_DEFAULT);

	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pAmMgr != NULL)
		pAmMgr->AddExtraActionListener(this);
	
	pScriptable = NULL;
	bSitting = false;
}

bool CProps::Init(IGameObject * pGameObject)
{

	SetGameObject(pGameObject); //
	return true;

}


void CProps::Reset()
{

	if (!pScriptable)
		return;

	SmartScriptTable propertiesTable;
	const bool hasPropertiesTable = pScriptable->GetValue("Properties", propertiesTable);
	if (!hasPropertiesTable) return;

	const char* model = NULL;
	propertiesTable->GetValue("objModel", model);
	GetEntity()->LoadGeometry(0, model);

}


void CProps::PostInit(IGameObject * pGameObject)
{
	GetEntity()->LoadGeometry(0, FILE_PATH);
	pGameObject->EnableUpdateSlot(this, 0);

	pScriptable = GetEntity()->GetScriptTable();
	pEntity = GetEntity();

	Reset();

}

void CProps::OnAction(const ActionId& action, int activationMode, float value)
{
	const CGameActions &actions = g_pGame->Actions();
	
	if (actions.UseProps == action)
	{
		if (bSitting)
		{
			
			g_pGameActions->FilterNoMove()->Enable(false);
			bSitting = false;
		}
	}

}

void CProps::ProcessEvent(SEntityEvent& entityEvent)
{

	switch (entityEvent.event)
	{
	case ENTITY_EVENT_SCRIPT_EVENT:
		{
			const char* eventName = NULL;
			eventName = reinterpret_cast<const char*>(entityEvent.nParam[0]);
			if (eventName && !strcmp(eventName,"Used"))
			{
				InitOptions();
			}		

		}
	}

}

void CProps::InitOptions()
{
	SmartScriptTable propertiesTable;
	const bool hasPropertiesTable = pScriptable->GetValue("Properties", propertiesTable);
	if (!hasPropertiesTable) return;
	
	bool bSit;
	bool bSleep;

	propertiesTable->GetValue("bSit",bSit);
	propertiesTable->GetValue("bSleep", bSleep);

	if (bSit && !bSleep)
	{
		pChar = g_pGame->GetIGameFramework()->GetClientActor()->GetEntity()->GetCharacter(0);
		pSkeleton = pChar->GetISkeletonAnim();

		pSkeleton->StopAnimationsAllLayers();
		
		g_pGame->GetIGameFramework()->GetClientActor()->GetEntity()->SetRotation(pEntity->GetRotation());
		g_pGame->GetIGameFramework()->GetClientActor()->GetEntity()->SetPos(pEntity->GetPos()/*+Vec3 (0,2,0)*/);


	

		g_pGameActions->FilterNoMove()->Enable(true);
		
		//pSkeleton->StartAnimation("stand_tac_idle_sittingCrate_3p_01_04", Params);
		
		bSitting = true;	
		CryLogAlways("Sitting");
	}

	if (!bSit && bSleep)
	{
		gEnv->p3DEngine->GetTimeOfDay()->SetTime(12, true);
		CryLogAlways("Sleeping");
	}

	if (bSit && bSleep){ return; }

}

void CProps::OnPostUpdate(float fDeltaTime)
{
	
	if (bSitting)
	{
	//	g_pGame->GetIGameFramework()->GetClientActor()->GetEntity()->SetPos(pEntity->GetPos());
		g_pGame->GetIGameFramework()->GetClientActor()->GetEntity()->SetRotation(pEntity->GetRotation());
		CryCharAnimationParams Params;

		Params.m_fAllowMultilayerAnim = 1;
		Params.m_nLayerID = 2;
		//Params.m_nUserToken = 1;
		Params.m_fPlaybackWeight = 1;
		//Params.m_fTransTime = 1;
		//Params.m_fPlaybackSpeed = 1;
		pSkeleton->StopAnimationsAllLayers();
		pSkeleton->StartAnimation("relaxed_tac_sit_nw_3p_01", Params);
	}
	
}