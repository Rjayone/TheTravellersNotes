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
	/*if (m_pParticalEntity != NULL && m_pLight != NULL)
	{
		m_pParticalEntity->SetPos(firePosition);
		m_pLight->SetPosition(firePosition);
	}*/

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
		/*if (m_pParticalEntity)
		{
			GetEntity()->FreeSlot(4);
			gEnv->pEntitySystem->RemoveEntity(m_pParticalEntity->GetId(), true);
			m_pParticalEntity = NULL;
		}
		if (m_pLight)
		{
			delete m_pLight;
			m_pLight = NULL;
		}*/
		RemoveEntityHelper();
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
	m_pPlayer = ((CPlayer*)g_pGame->GetIGameFramework()->GetClientActor())->GetEntity();

	pAttachmentManager = m_pPlayer->GetCharacter(0)->GetIAttachmentManager();

	SpawnEntityHelper();
	
	CreateAttachment();
	
	CreateLight();

	IParticleEffect *pFireParticle = gEnv->pParticleManager->FindEffect(DEFAULT_PARTICLE);

	if (pFireParticle)
		m_pBasicEntity->LoadParticleEmitter(PARTICLE_SLOT, pFireParticle);
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

void CTorch::SpawnEntityHelper()
{
	SEntitySpawnParams spawn;
	Vec3 positionFire = CSpecialFunctions::GetBonePos("fire", false, GetEntity());

	spawn.pClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass("BasicEntity");
	spawn.vPosition = positionFire;
	spawn.vScale = Vec3(0.1, 0.1, 0.1); //Уменьшение размера
	spawn.sName = "LightHelper";          // Имя хелпера, понадобится при удалении 
	m_pBasicEntity = gEnv->pEntitySystem->SpawnEntity(spawn);

	SEntityPhysicalizeParams params;
	params.type = PE_NONE;
	m_pBasicEntity->Physicalize(params);

	if (IEntityRenderProxy* pProxy = (IEntityRenderProxy *)m_pBasicEntity->GetProxy(ENTITY_PROXY_RENDER))
	{
		pProxy->SetOpacity(0); // прозрачность хэлпера (0 - невидим)
	}
}

void CTorch::RemoveEntityHelper()
{
	pAttachmentManager->RemoveAttachmentByName("myAttachment");
	gEnv->pEntitySystem->RemoveEntity(m_pBasicEntity->GetId());
}

void CTorch::CreateLight()
{
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

	m_pBasicEntity->LoadLight(LIGHT_SLOT, m_pLight);
}

void CTorch::CreateAttachment()
{
	IAttachment* attachment = pAttachmentManager->CreateAttachment("myAttachment", CA_BONE, "weapon_bone", true);

	int32 boneId = 0;

	IDefaultSkeleton& rIDefaultSkeleton = m_pPlayer->GetCharacter(0)->GetIDefaultSkeleton();
	boneId = rIDefaultSkeleton.GetJointIDByName("weapon_bone");

	ISkeletonPose* pSkeleton = m_pPlayer->GetCharacter(0)->GetISkeletonPose();
	const QuatT boneSpace = QuatT(m_pPlayer->GetWorldTM()) * pSkeleton->GetAbsJointByID(boneId);


	QuatT relProj;
	relProj.t = CSpecialFunctions::GetBonePos("weapon_bone", false, m_pPlayer);
	relProj.q = GetEntity()->GetRotation();
	GetEntity()->SetWorldTM(Matrix34(relProj));

	relProj = boneSpace.GetInverted() * relProj;
	auto relProjPos = relProj.t;
	auto relProjRot = relProj.q;

	attachment->SetAttRelativeDefault(QuatT(relProjRot, relProjPos));

	CEntityAttachment* pEntityAttachment = new CEntityAttachment();
	pEntityAttachment->SetEntityId(m_pBasicEntity->GetId());
	attachment->AddBinding(pEntityAttachment);
}