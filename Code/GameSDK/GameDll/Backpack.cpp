#include "StdAfx.h"
#include "Backpack.h"
#include "RPGInventory.h"

#include "PlayerStatsManager.h"

#define FILE_PATH "GameSDK/Objects/box.cgf"
#define UI_NAME "Inventory"

CBackpack::CBackpack()
{
	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pAmMgr != NULL)
		pAmMgr->AddExtraActionListener(this);
}

CBackpack::~CBackpack()
{
	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pAmMgr != NULL)
		pAmMgr->RemoveExtraActionListener(this);
}


bool CBackpack::Init(IGameObject * pGameObject)
{
	SetGameObject(pGameObject); //
	return true;
}

void CBackpack::PostInit(IGameObject * pGameObject)
{
	//инициализация при вытаскивании на сцену на случай если мы будем использовать рюкзак просто как объект
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

	Physicalize(PE_RIGID);
}

void CBackpack::Physicalize(int type)
{
	// Физикализация рюкзака. Масса берется из луа
	IEntity *pEntity = GetEntity();
	if (pEntity == NULL) return;

	float mass = 1;
	SmartScriptTable propertiesTable;
	pScriptable->GetValue("PhysParams", propertiesTable);
	propertiesTable->GetValue("mass", mass);

	SEntityPhysicalizeParams pp;
	pp.type = type;
	pp.nSlot = -1;
	pp.mass = mass;
	pp.nFlagsOR = pef_monitor_poststep;
	pp.fStiffnessScale = 5;

	pEntity->Physicalize(pp);
}

void CBackpack::OnAction(const ActionId& action, int activationMode, float value)
{
	
}

void CBackpack::Update(SEntityUpdateContext& ctx, int slot)
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
			if (eventName && !strcmp(eventName, "Used"))  // Ловит эвенте что игрок использует наш рюкзак
			{
				CRPGInventory *pUIInventory = g_pGame->GetRPGInventory();	
				if (IUIElement* pInventory = gEnv->pFlashUI->GetUIElement(UI_NAME))
				{
					pUIInventory->ShowInventory(pInventory); // открывает инвентарь через рюкзак на земеле( будто мы шаримся в сумке)
				}
			}

		}
	}
}