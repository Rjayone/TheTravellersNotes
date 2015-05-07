/****************************************************************************************************
* Weapon: Bow
* Description: �����, ����������� ��������� ����
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
	//��������:
	//������� ���������� ������ ������.
	//��������� �������� projectile - ������ � � ������
	//������� ����� ���� ������� ������ ������� CBow
	void Launch();

	CHolding* m_Holding;	//��������� �� ����� ����������� ���������.
	CNock* m_Nock;			//���������
	bool m_bBusy;			//Weapon's busy
	bool m_bDeselect;		//�� ����� ���������
	bool m_bLaunched;		//���� ��� �������

	int m_currentFiremode;
};