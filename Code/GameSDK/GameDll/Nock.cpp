#include "StdAfx.h"
#include "Nock.h"
#include "Projectile.h"
#include "Weapon.h"

CNock::CNock()
: m_fScalePerSecond(1.0f)
, m_fMinDamage(75.0f)
, m_fMaxDamage(100.0f)
, m_fMaxHoldTime(20.0f)
{
	m_pMouseUtils = new CMouseUtils();
}

//-------------------------------------------------------------------
CNock::~CNock()
{
	if (m_pMouseUtils)
	{
		delete m_pMouseUtils;
		m_pMouseUtils = NULL;
	}
}

//-------------------------------------------------------------------
void CNock::StartFire()
{
	m_pMouseUtils->InitClickTime();
	CItem* pItem = (CItem*)g_pGame->GetIGameFramework()->GetClientActor()->GetCurrentItem();
	CProjectile* arrow = new CProjectile;
	//arrow->

	//m_pShared = static_cast<CSingleSharedData*>(m_fireParams.get());
	//m_pShared->fireparams.damage = minDamage;	
}

//-------------------------------------------------------------------
void CNock::StopFire()
{
}

//-------------------------------------------------------------------
void CNock::Nock()
{
	//uint32 currentScale = 0;

	//if(StopTime-StartTime <= 20)
	//{
	//	currentScale = (StopTime - StartTime) * ScalePerSecond;	
	//	m_pShared->fireparams.damage = (int) currentScale +  GetDamage();
	//	CryLogAlways("Current damage = %d", m_pShared->fireparams.damage);
	//}

	//else
	//{
	//	float UnderTime = (StopTime - StartTime) - maxHoldTime;
	//	if(UnderTime > 5)
	//	{
	//		m_pShared->fireparams.damage = minDamage;
	//		CryLogAlways("Current damage = %d", m_pShared->fireparams.damage);
	//	}

	//	if(UnderTime < 5)
	//	{
	//		m_pShared->fireparams.damage = (maxDamage - (ReducePerSecond * UnderTime) );	
	//		CryLogAlways("Current damage = %d", m_pShared->fireparams.damage);
	//	}
	//}
	//minDamage = 75.0f;// need to fix
}
