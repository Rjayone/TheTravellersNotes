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
#include "SimpleSearchStruct.h"

#define DEFAULT_PARTICLE "smoke_and_fire.VS2_Fire.smallfire_nobase_noglow"
#define FIRE_PARTICLE_SLOT 0

//--------------------------------------
CFirePlace::CFirePlace() :
	m_pParticalEntity(nullptr),
	state(IDLE),
	m_noColor(0.0, 0.0, 0.0),
	m_fullColor(0.0, 0.0, 0.0),
	m_fColorMultipier(1.0)
{
	IActionMapManager* pAmMgr = g_pGame->GetIGameFramework()->GetIActionMapManager();
	if (pAmMgr != nullptr)
		pAmMgr->AddExtraActionListener(this);
}

//--------------------------------------
CFirePlace::~CFirePlace()
{
	if (m_pLightSource != nullptr)
	{
		gEnv->p3DEngine->UnRegisterEntityDirect(m_pLightSource);
		gEnv->p3DEngine->DeleteLightSource(m_pLightSource);
	}

	// Removing fireplace the search pool in case it wasn`t removed before
	SearchItem searchableItem(GetEntityId(), GetEntity()->GetPos());
	searchStruct->RemoveItem(searchableItem);
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
		if (!props->GetValue("clrLightColor", lightc))
			return;

		if (
			!lightc->GetValue("r", red) ||
			!lightc->GetValue("g", green) ||
			!lightc->GetValue("b", blue) ||
			!lightc->GetValue("a", alpha)
		) {
			return;
		}

		if (!props->GetValue("fLightColorMultiplier", m_fColorMultipier))
			return;

		m_fullColor.set(red / 255.0 * m_fColorMultipier, green / 255.0 * m_fColorMultipier, 
			blue / 255.0 * m_fColorMultipier, alpha / 255.0 * m_fColorMultipier);
		
		if (!props->GetValue("fFadeLightTime", m_fFadeTimeLimit))
			return;

		// Creating light source
		m_pLightSource = gEnv->p3DEngine->CreateLightSource();

		// Initializing it's transform matrix with frieplace matrix
		// Note: Seems to be that lightSource must be initialized through SetMatrix in order for later light.SetPosition to work
		// Note: Identity matrix do not work for this!
		m_pLightSource->SetMatrix(GetEntity()->GetWorldTM());

		// Initializing light properties
		m_pLightSource->GetLightProperties().SetLightColor(m_noColor);

		// Registering light source for render
		gEnv->p3DEngine->RegisterEntity(m_pLightSource);
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
	}
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

	// Update light position
	m_pLightSource->GetLightProperties().SetPosition(GetEntity()->GetPos());

	// Physicalize the object as static entity
	SEntityPhysicalizeParams pp;
	pp.type = PE_STATIC;
	GetEntity()->Physicalize(pp);

	SmartScriptTable propertiesTable;
	if (m_pScriptTable && m_pScriptTable->GetValue("Properties", propertiesTable))
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
			// Registering time change
			m_fFadeTimeAcc += updateContext.fFrameTime;

			// Calculating proportion of increment
			float increment = m_fFadeTimeAcc / m_fFadeTimeLimit;

			// Getting light properties
			CDLight& light = m_pLightSource->GetLightProperties();

			// Detecting if fade time is over
			if (m_fFadeTimeAcc >= m_fFadeTimeLimit) {

				// Full color and switch state
				light.SetLightColor(m_fullColor);
				SetState(BURNING);
			}
			else {

				// Calculating new interpolated color
				ColorF newColor(light.GetFinalColor(m_noColor));
				newColor.lerpFloat(m_noColor, m_fullColor, increment);

				light.SetLightColor(newColor);
			}
		}
		else if (GetState() == FLAME_FADE_OUT)
		{
			// Registering time change
			m_fFadeTimeAcc += updateContext.fFrameTime;

			// Calculating proportion of increment
			float increment = m_fFadeTimeAcc / m_fFadeTimeLimit;

			// Getting light properties
			CDLight& light = m_pLightSource->GetLightProperties();

			// Detecting if fade time is over
			if (m_fFadeTimeAcc >= m_fFadeTimeLimit) {

				// Put off the lights and switch state
				SetState(BURNED);
				light.SetLightColor(m_noColor);
			}
			else {

				// Calculating new interpolated color
				ColorF newColor(light.GetFinalColor(m_noColor));
				newColor.lerpFloat(m_fullColor, m_noColor, increment);

				light.SetLightColor(newColor);
			}
		}
		else if (GetState() == BURNED)
		{
			// Make the fireplace unusable
			SmartScriptTable propertiesTable;
			if (m_pScriptTable.GetPtr() != nullptr &&
				m_pScriptTable->GetValue("Properties", propertiesTable))
			{
				propertiesTable->SetValue("bUsable", false);
			}

			// Deactivate
			GetEntity()->Activate(false);
		}
	}
}

//--------------------------------------
void CFirePlace::StartFire()
{
	assert(m_pLightSource != nullptr);

	SetState(FLAME_FADE_IN);

	// Adding our fireplace to the search pool
	SearchItem searchable(GetEntityId(), GetEntity()->GetPos());
	searchStruct->AddItem(searchable);

	// Reseting time accumulator
	m_fFadeTimeAcc = 0.0;

	// Starting from zero brightness
	m_pLightSource->GetLightProperties().SetLightColor(m_noColor);

	// Update light position
	// Note: Since it is non-directional we do not need to use transform matrix, just position
	m_pLightSource->GetLightProperties().SetPosition(GetEntity()->GetPos());

	// Read particle name from lua
	SmartScriptTable propertiesTable;
	const char* particleName = nullptr;

	if (m_pScriptTable.GetPtr() == nullptr ||
		!m_pScriptTable->GetValue("Properties", propertiesTable) ||
		!propertiesTable->GetValue("sParticle", particleName) || 
		strcmp(particleName, "") == 0)
	{		
		particleName = DEFAULT_PARTICLE;
	}

	// Spawn particle
	SEntitySpawnParams spawn;
	IParticleEffect *pFireParticle = gEnv->pParticleManager->FindEffect(particleName);

	spawn.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("ParticleEffect");
	if (spawn.pClass)
	{
		if (m_pParticalEntity == nullptr)
			m_pParticalEntity = gEnv->pEntitySystem->SpawnEntity(spawn);

		m_pParticalEntity->SetPosRotScale(GetEntity()->GetPos(), Quat(0.7071, 0.7071, 0, 0), Vec3(0.5, 0.5, 0.5));
		if (pFireParticle && m_pParticalEntity)
		{
			m_pParticalEntity->LoadParticleEmitter(FIRE_PARTICLE_SLOT, pFireParticle);
		}
	}
}

//--------------------------------------
void CFirePlace::StopFire()
{
	assert(m_pLightSource != nullptr);

	SetState(FLAME_FADE_OUT);

	// Removing fireplace the search pool
	SearchItem searchableItem(GetEntityId(), GetEntity()->GetPos());
	searchStruct->RemoveItem(searchableItem);

	// Reseting time accumulator
	m_fFadeTimeAcc = 0.0;

	// Delete particles
	m_pParticalEntity->FreeSlot(FIRE_PARTICLE_SLOT);
}

//--------------------------------------
void CFirePlace::Reset()
{
	SetState(IDLE);
	m_pParticalEntity = nullptr;
	m_fColorMultipier = 1.0;

	// Reseting time accumulator
	m_fFadeTimeAcc = 0.0;

	const char* modelPath = nullptr;
	SmartScriptTable propertiesTable;
	m_pScriptTable = GetEntity()->GetScriptTable();

	if (m_pScriptTable && 
		m_pScriptTable->GetValue("Properties", propertiesTable) && 
		propertiesTable->GetValue("objModel", modelPath)) 
	{
		GetEntity()->LoadGeometry(0, modelPath);
	}
	else {
		CryLog("Failed to load fireplace model");
	}

	propertiesTable->SetValue("bUsable", true);
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

// Search struct initialization
// This variable exist until the programm exits, thus resource release can be omitted
ISearchDataStruct* CFirePlace::searchStruct = new SimpleSearchStruct();