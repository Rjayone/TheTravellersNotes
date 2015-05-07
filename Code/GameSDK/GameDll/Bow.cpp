#include "StdAfx.h"
#include "Bow.h"
#include "Arrow.h"
#include "GameTypeInfo.h"
#include "Single.h"

CBow::CBow()
{
	m_Holding = NULL;
	m_bBusy = false;
	m_bLaunched = false;
	m_currentFiremode = 0;
}

//-------------------------------------------------------------------
bool CBow::Init(IGameObject * pGameObject)
{
	if (!CWeapon::Init(pGameObject))
		return false;

	SetGameObject(pGameObject);
	return true;
}

//-------------------------------------------------------------------
void CBow::Release()
{
	delete this;
}

//-------------------------------------------------------------------
void CBow::Select(bool select)
{
	if (select == true && IsOwnerClient())
	{
		m_bDeselect = false;
	}
	else
	{
		m_bDeselect = true;
	}

	CWeapon::Select(select);
}

void CBow::ProcessEvent(SEntityEvent& event)
{
	if (m_Holding)
		m_Holding->ProcessEvent(event);
}

//-------------------------------------------------------------------
void CBow::Update(SEntityUpdateContext& ctx, int slot)
{
	CWeapon::Update(ctx, slot);

	if (m_Holding)
		m_Holding->Update();

	if (m_Holding && IsOwnerClient())
	{
		//Если начат удар
		int holdingStatus = m_Holding->GetStatus();
		if (holdingStatus == e_HoldingStatusRelease && m_bLaunched == false)// || holdingStatus == e_HoldingStatusShortClick)
		{
			//Делаем выстрел
			Launch();
		}
	}
}

//-------------------------------------------------------------------
void CBow::InitFireModes()
{
	CWeapon::InitFireModes();
	int firemodeCount = m_firemodes.size();


	for (int i = 0; i < firemodeCount; i++)
	{
		if (crygti_isof<CSingle>(m_firemodes[i]))
		{
			m_currentFiremode = i;
		}
	}

	SetCurrentFireMode(m_currentFiremode);
}

//-------------------------------------------------------------------
void CBow::StartFire()
{
	if (!IsOwnerClient())
		return;

	if (!m_Holding)
	{
		m_Holding = new CHolding();
		m_Holding->Init(this);
	}

	m_bLaunched = false;
	m_Holding->StartFire(this);
}

void CBow::StartFire(const SProjectileLaunchParams& launchParams)
{
	CWeapon::StartFire(launchParams);
}

//-------------------------------------------------------------------
void CBow::StopFire()
{
	if (!IsOwnerClient() || !m_Holding || m_bDeselect)
		return;

	m_Holding->StopFire(this);
	////	
	SetBusy(false);
	//CWeapon::StartFire();
	CWeapon::StopFire();
}

//-------------------------------------------------------------------
void CBow::Launch()
{
	m_Holding->StopFire(this);
	//	
	SetBusy(false);
	CWeapon::StartFire();

	m_bLaunched = true;
}


