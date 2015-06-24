#include "StdAfx.h"
#include "Backpack.h"


#define FILE_PATH "GameSDK/Objects/box.cgf"


CBackpack::CBackpack()
{
	IGameFramework *pGameFramework = g_pGame->GetIGameFramework();
	if (pGameFramework)
		pGameFramework->RegisterListener(this, "CBackpack", FRAMEWORKLISTENERPRIORITY_DEFAULT);

	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pAmMgr != NULL)
		pAmMgr->AddExtraActionListener(this);
}


bool CBackpack::Init(IGameObject * pGameObject)
{
	SetGameObject(pGameObject); //
	return true;
}

void CBackpack::PostInit(IGameObject * pGameObject)
{
	GetEntity()->LoadGeometry(0, FILE_PATH);
	pGameObject->EnableUpdateSlot(this, 0);

	pScriptable = GetEntity()->GetScriptTable();
	pEntity = GetEntity();

	Reset();
}

void CBackpack::Reset()
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

void CBackpack::OnAction(const ActionId& action, int activationMode, float value)
{
	
}

void CBackpack::ProcessEvent(SEntityEvent& entityEvent)
{

	switch (entityEvent.event)
	{
		case ENTITY_EVENT_SCRIPT_EVENT:
		{
			const char* eventName = NULL;
			eventName = reinterpret_cast<const char*>(entityEvent.nParam[0]);
			if (eventName && !strcmp(eventName, "Used"))
			{
			//InitOptions();
			}

		}
	}
}