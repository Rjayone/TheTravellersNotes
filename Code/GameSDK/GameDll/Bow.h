/****************************************************************************************************
* Weapon: Bow
* Description: Класс, реализующий поведения лука
* Created by: Andrew Medvedev
* Date: 29.09.2014
* Edda Studio
****************************************************************************************************/

#pragma once
#include "Weapon.h"
#include "Holding.h"
#include "WeaponProperties.h"
#include "Nock.h"

class CBow :public CWeapon
{
public:
	CBow();
	void Release();

	bool Init(IGameObject * pGameObject);
	void Update(SEntityUpdateContext& ctx, int slot);
	void Select(bool select);
	void ProcessEvent(SEntityEvent& event);

	void InitFireModes();
	void StartFire();
	void StartFire(const SProjectileLaunchParams& launchParams);
	void StopFire();
	

	bool IsBusy() const    { return m_bBusy; }
	void SetBusy(bool busy){ m_bBusy = busy; }
	CHolding* GetHoldingStatus(){ return m_Holding; }
	int GetWeaponType(){ return e_WeaponTypeBow; }
private:
	//Описание:
	//Функция производит запуск стрелы.
	//Проиходит создание projectile - стрелы и её запуск
	//Функция может быть вызвана только классом CBow
	void Launch();

	CHolding* m_Holding;	//Указатель на класс реализующий удержание.
	CNock* m_Nock;			//Натяжение
	bool m_bBusy;			//Weapon's busy
	bool m_bDeselect;		//Во время деселекта
	bool m_bLaunched;		//Если был выстрел

	int m_currentFiremode;
};