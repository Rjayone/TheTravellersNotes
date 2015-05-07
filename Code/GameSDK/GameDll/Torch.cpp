#include "StdAfx.h"
#include "Torch.h"
#include "Weapon.h"
#include "Player.h"
#include "SpecialFunctions.h"

#define DEFAULT_PARTICLE "smoke_and_fire.VS2_Fire.smallfire_nobase_noglow"
#define PARTICLE_SLOT 4
#define LIGHT_SLOT	  5

//-------------------------------------------------------------------
CTorch::CTorch()
{
	m_bIsFired = false;
	m_fDiffuseMult = 0;
	m_fFireDuration = 1000; //in miliseconds
	m_pParticalEntity = NULL; 
	m_pLight = NULL;
}

//-------------------------------------------------------------------
bool CTorch::Init(IGameObject * pGameObject){
	if (!CWeapon::Init(pGameObject))
		return false;

	SetGameObject(pGameObject);
	return true;
}

//-------------------------------------------------------------------
void CTorch::Release()
{
	delete this;
}

//-------------------------------------------------------------------
void CTorch::Update(SEntityUpdateContext& ctx, int slot)
{
	DebugDraw();
	Vec3 firePosition = CSpecialFunctions::GetBonePos("fire", false, GetEntity());
	if (m_pParticalEntity != NULL && m_pLight != NULL)
	{
		m_pParticalEntity->SetPos(firePosition);
		m_pLight->SetPosition(firePosition);
	}

	CWeapon::Update(ctx, slot);
}

//-------------------------------------------------------------------
void CTorch::Select(bool select)
{
	CWeapon::Select(select);
	if (select == true)
		Fire();
	else
	{
		if (m_pParticalEntity)
		{
			GetEntity()->FreeSlot(4);
			gEnv->pEntitySystem->RemoveEntity(m_pParticalEntity->GetId(), true);
			m_pParticalEntity = NULL;
		}
		if (m_pLight)
		{
			delete m_pLight;
			m_pLight = NULL;
		}
	}
}

//-------------------------------------------------------------------
void CTorch::ProcessEvent(SEntityEvent& event)
{
	if (event.event == ENTITY_EVENT_RESET)
		Reset();

	CWeapon::ProcessEvent(event);
}

//-------------------------------------------------------------------
void CTorch::Fire()
{
	Vec3 firePosition = CSpecialFunctions::GetBonePos("fire", false, GetEntity());
	SEntitySpawnParams spawn;
	IParticleEffect *pFireParticle = gEnv->pParticleManager->FindEffect(DEFAULT_PARTICLE);

	spawn.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("ParticleEffect");
	if (spawn.pClass)
	{
		if (m_pParticalEntity == nullptr)
			m_pParticalEntity = gEnv->pEntitySystem->SpawnEntity(spawn);

		m_pParticalEntity->SetPos(firePosition);
		if (pFireParticle && m_pParticalEntity);
			//GetEntity()->LoadParticleEmitter(PARTICLE_SLOT, pFireParticle);
	}

	if (m_pLight == NULL)
		m_pLight = new CDLight();

	m_pLight->SetPosition(GetEntity()->GetPos());
	m_pLight->SetLightColor(ColorF(128 * 0.03, 79 * 0.03, 24 * 0.03, 1.f));
	m_pLight->SetSpecularMult(1.0f);
	m_pLight->m_fRadius = 3.f;
	m_pLight->m_ProbeExtents(3, 3, 3);
	m_pLight->m_fBaseRadius = 1.0f;
	m_pLight->m_Flags |= DLF_CASTSHADOW_MAPS + DLF_LIGHTBOX_FALLOFF;
	m_pLight->m_nShadowMinResolution = 0;
	m_pLight->m_nLightStyle = (uint8)34;
	m_pLight->SetAnimSpeed(1.0f);
	m_pLight->m_fShadowUpdateMinRadius = 6.0f;

	GetEntity()->LoadLight(LIGHT_SLOT, m_pLight);
}

//-------------------------------------------------------------------
void CTorch::DebugDraw()
{
	//Отрисовка хэлпера факела, с которого идет огонь
	if (g_pGameCVars->td_draw == 1)
	{
		Vec3 helperPos = CSpecialFunctions::GetBonePos("fire", false, GetEntity());
		gEnv->pRenderer->GetIRenderAuxGeom()->DrawSphere(helperPos, 0.08f, ColorB(128, 128, 0));

		Vec3 helperPos2 = CSpecialFunctions::GetBonePos("fire", false, GetOwner());
		gEnv->pRenderer->GetIRenderAuxGeom()->DrawSphere(helperPos2, 0.08f, ColorB(0, 128, 0));
	}
}