/****************************************
* File: FirePlace.cpp
* Description: Игровой объект - костер
* Created by: Andrew Medvedev
* Date: 22.08.2013
* Diko Source File
*****************************************/
#include "StdAfx.h"
#include "FirePlace.h"
#include "Game.h"
#include "Actor.h"
#include "GameActions.h"
#include "IParticles.h"
#include "RPGInventory.h"

#define DEFAULT_PARTICLE "smoke_and_fire.VS2_Fire.smallfire_nobase_noglow"
#define FIRE_PARTICLE_SLOT 0
#define FIRE_LIGHT_SLOT 1



//--------------------------------------
CFirePlace::CFirePlace():
	m_pParticalEntity(nullptr),
	state(IDLE),
	bonusRadius(2.0),
	noAnimalsRadius(5.0)
{
	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pAmMgr != nullptr)
		pAmMgr->AddExtraActionListener(this);
}

//--------------------------------------
CFirePlace::~CFirePlace()
{
}

//--------------------------------------
bool CFirePlace::Init(IGameObject* pGameObject)
{
	SetGameObject(pGameObject);
	return true;
}

//--------------------------------------
void CFirePlace::PostInit(IGameObject* pGameObject)
{
	m_pScriptTable = GetEntity()->GetScriptTable();

	if (m_pScriptTable)
	{
		// Load flame color
		float red, green, blue, alpha;
		SmartScriptTable props, lightc;
		m_pScriptTable->GetValue("Properties", props);
		if (!props->GetValue("clrLightColor", lightc)) //Безопасность превыше всего! :)
			return;

		if (
			!lightc->GetValue("r", red) ||
			!lightc->GetValue("g", green) ||
			!lightc->GetValue("b", blue) ||
			!lightc->GetValue("a", alpha)
		) {
			return;
		}

		flameLightColor.set(red / 255.0, green / 255.0, blue / 255.0, alpha / 255.0);
		peakLightAlpha = alpha / 255.0;
		
		if (!props->GetValue("fadeLightTime", fadeLightTime))
			return;
	}
	else
		CryLog("Can not find script table for Fireplace");

	pGameObject->EnableUpdateSlot(this, 0);

	Reset();
}

//--------------------------------------
void CFirePlace::ProcessEvent(SEntityEvent& event)
{
	switch (event.event)
	{
		case ENTITY_EVENT_RESET:
		{
			Reset();
			break;
		}
		case ENTITY_EVENT_SCRIPT_EVENT:
		{
			const char* eventName = nullptr;
			eventName = reinterpret_cast<const char*>(event.nParam[0]);
			if (eventName) {
				if (strcmp(eventName, "Used") == 0)
				{
					OnUse();
				}
			}
			break;
		}
	};
}

//--------------------------------------
void CFirePlace::OnUse()
{
	switch (GetState())
	{
	case IDLE: {
		StartPlacing();
		break;
	}
	case PLACED: {
		StartFire();
		break;
	}
	case BURNING: {
		StopFire();
		break;
	}
	default:
		break;
	}
}

//--------------------------------------
void CFirePlace::StartPlacing()
{
	if (gEnv->pSystem != nullptr && GetEntity() != nullptr)
	{
		// Switching to placing state
		SetState(PLACING);
		GetEntity()->Activate(true);

		// Dephysicalize the object
		SEntityPhysicalizeParams pp;
		pp.type = PE_NONE;
		GetEntity()->Physicalize(pp);

		// Disable further usage during placing
		SmartScriptTable propertiesTable;
		if (m_pScriptTable.GetPtr() != nullptr &&
			m_pScriptTable->GetValue("Properties", propertiesTable))
		{
			propertiesTable->SetValue("bUsable", false);
		}
	}
}

//--------------------------------------
void CFirePlace::UpdatePlacing() 
{
	ray_hit ray;
	Vec3 cameraDir = gEnv->pSystem->GetViewCamera().GetViewdir();
	Vec3 cameraPos = gEnv->pSystem->GetViewCamera().GetPosition() + cameraDir;

	float castDistanceLimit = 2;
	gEnv->pPhysicalWorld->RayWorldIntersection(cameraPos, cameraDir * castDistanceLimit,
		ent_all, rwi_stop_at_pierceable | rwi_colltype_any, &ray, 1);

	GetEntity()->SetPos(ray.pt);
}

//--------------------------------------
void CFirePlace::StopPlacing()
{
	SetState(PLACED);

//	GetEntity()->Activate(false);

	// Physicalize the object as static entity
	SEntityPhysicalizeParams pp;
	pp.type = PE_STATIC;
	GetEntity()->Physicalize(pp);

	SmartScriptTable propertiesTable;
	if (m_pScriptTable.GetPtr() != nullptr && 
		m_pScriptTable->GetValue("Properties", propertiesTable))
	{
		propertiesTable->SetValue("bUsable", true);
	}

	CRPGInventory* pInv = g_pGame->GetRPGInventory();
	if (pInv)
	{
		pInv->DeleteItem(GetEntity()->GetId());
		CryLogAlways("Fireplace deleted");
	}
}

//--------------------------------------
void CFirePlace::Update(SEntityUpdateContext& updateContext, int updateSlot)
{
	if (GetEntity() != nullptr)
	{
		if (GetEntity()->IsActive() && GetState() == PLACING)
			UpdatePlacing();
		else if (GetState() == FLAME_FADE_IN)
		{
			// Increasing light brightness
			flameLightColor.a += (updateContext.fFrameTime / fadeLightTime) * peakLightAlpha;

			// Check if fade phase is over
			if (flameLightColor.a >= peakLightAlpha) {
				flameLightColor.a = peakLightAlpha;
				SetState(BURNING);
			}

			lightProperties.SetLightColor(flameLightColor);
			m_pParticalEntity->FreeSlot(FIRE_LIGHT_SLOT);
			m_pParticalEntity->LoadLight(FIRE_LIGHT_SLOT, &lightProperties);
		}

		else if (GetState() == FLAME_FADE_OUT)
		{
			// Decreasing light brightness
			flameLightColor.a -= (updateContext.fFrameTime / fadeLightTime) * peakLightAlpha;

			// Check if fade phase is over
			if (flameLightColor.a <= 0.0) {
				flameLightColor.a = 0.0;
				SetState(BURNED);
			}

			lightProperties.SetLightColor(flameLightColor);
			m_pParticalEntity->FreeSlot(FIRE_LIGHT_SLOT);
			m_pParticalEntity->LoadLight(FIRE_LIGHT_SLOT, &lightProperties);
		}
	}
}

//--------------------------------------
void CFirePlace::StartFire()
{
	SetState(FLAME_FADE_IN);

	SmartScriptTable propertiesTable;
	const char* particleName = nullptr;
	flameLightColor.a = 0;

	if (m_pScriptTable.GetPtr() == nullptr ||
		!m_pScriptTable->GetValue("Properties", propertiesTable) ||
		!propertiesTable->GetValue("sParticle", particleName) || 
		strcmp(particleName, "") == 0)
	{		
		particleName = DEFAULT_PARTICLE;
	}

	lightProperties.SetPosition(GetEntity()->GetPos());
	lightProperties.SetLightColor(flameLightColor); //Здесь стоит добавить множитель дифуза. Вектор умножить на 0.01, что соответствует 1
											   //При этом цвет не будет таким ярким, вырвиглаз
	lightProperties.m_fRadius = 10;
	lightProperties.m_Flags |= DLF_CASTSHADOW_MAPS; //С тенью что-то и у меня не выходит задать :(

	SEntitySpawnParams spawn;
	IParticleEffect *pFireParticle = gEnv->pParticleManager->FindEffect(particleName);

	spawn.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("ParticleEffect");
	if (spawn.pClass)
	{
		if (m_pParticalEntity == nullptr)
			m_pParticalEntity = gEnv->pEntitySystem->SpawnEntity(spawn);

		m_pParticalEntity->SetPos(GetEntity()->GetPos());
		m_pParticalEntity->SetRotation(Quat(0.7071, 0.7071, 0, 0));
		if (pFireParticle && m_pParticalEntity)
		{
			m_pParticalEntity->LoadParticleEmitter(FIRE_PARTICLE_SLOT, pFireParticle);
			m_pParticalEntity->LoadLight(FIRE_LIGHT_SLOT, &lightProperties);
		}
	}

}

//--------------------------------------
void CFirePlace::StopFire()
{
	SetState(FLAME_FADE_OUT);
	m_pParticalEntity->FreeSlot(FIRE_PARTICLE_SLOT);
	m_pParticalEntity->FreeSlot(FIRE_LIGHT_SLOT);
}

//--------------------------------------
void CFirePlace::Reset()
{
	SetState(IDLE);
	const char* modelPath = nullptr;
	m_pParticalEntity = nullptr;
	SmartScriptTable propertiesTable;

	m_pScriptTable = GetEntity()->GetScriptTable();

	if (m_pScriptTable && 
		m_pScriptTable->GetValue("Properties", propertiesTable) && 
		propertiesTable->GetValue("objModel", modelPath))
	{
		GetEntity()->LoadGeometry(0, modelPath);
	}
	else
	{
		CryLog("Failed to load fireplace model");
	}
}


void CFirePlace::OnAction(const ActionId& action, int activationMode, float value)
{
	const CGameActions &actions = g_pGame->Actions();
	// If action button "Use" is pressed during placing
	if (action == actions.use && (activationMode & eAAM_OnPress) == 1 && GetState() == PLACING)
	{
		StopPlacing();
	}

	//if (actions.cancel_init_fire == action && GetState() == PLACING)
	//{
	//	SetState(PLACED);
	//	GetEntity()->Hide(true);
	//}
}