#include "StdAfx.h"
#include "Shield.h"
#include "Actor.h"


CShield::CShield()
: CollisionON(false)
, model(0)
, pEntity(0)
, pInventory(0)
, ShieldSelected(0)
{}

CShield::~CShield()
{
}

void CShield::SetPhysic(bool phys)
{
	if (phys)
	{
		pEntity = GetEntity();
		pEntity = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(LOCAL_PLAYER_ENTITY_ID)->GetInventory()->GetEntity();
		if (!pEntity)
			return;

		SEntityPhysicalizeParams ShieldParams;

		pe_player_dimensions ShieldDim;
		pe_player_dynamics   ShieldDyn;

		ShieldParams.pPlayerDimensions = &ShieldDim;
		ShieldParams.pPlayerDynamics = &ShieldDyn;

		ShieldParams.type = PE_LIVING;
		ShieldParams.nSlot = eIGS_ThirdPerson;

		ShieldDim.sizeCollider(0.0f, 0.0f, 0.0f);

		pEntity->Physicalize(ShieldParams);
	}

}

void CShield::Select(bool select)
{
	if (select)
	{
		SetPhysic(true);
	}

}
void CShield::Reset()
{
	CItem::Reset();
}