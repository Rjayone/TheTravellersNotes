/****************************************************************************************************
* Weapon: Sword
* Description: ����� ��� ����������� ��������� ����
  ��� ���������� ����������� ��������� �����(�����������) ��� ����������� ����� CHolding,
  ������� ������������ ����� �������� �����-�����-����
  �� ����� ����� ���� �������������� ���������: � ������� Update() ���������� PerformIntersection()
  ��� ����������� ��������� ������������ �������, ��� ��� ����������� ������������ ������� td_draw 1
  ����� ��������� ����������� ���� ����� �������� � �������� ������.

* Created by: Andrew Medvedev
* Date: 29.09.2014
* Edda Studio
****************************************************************************************************/

#pragma once
#include "Weapon.h"
#include "MouseDirection.h"
#include "Melee.h"
#include "Holding.h"
#include "CombatStance.h"
#include "WeaponProperties.h"


class CSword : public CWeapon
{
public:
	CSword();
	void Release();

	bool Init(IGameObject * pGameObject);
	void Update(SEntityUpdateContext& ctx, int slot);

	//��������:
	//������ ������� ���������� ��� ������ � �������� ������
	//���� ��������� ������ �� - true, ����� false
	void Select(bool select);

	//��������:
	//�������, ������� ������ ������ ���� �� ������������
	void AutoDeselect();
	
	//��������:
	//������ ������� ���������� � ������ ������ ���� ��� �����
	//� ������ ���������� ������ ����� ������������ ��� ����������� ���� �������, ����� ����� ������� ������ �����
	//�����, ���� ������ ������������, �� ��� ������ � ����� ��������� �����
	void StartFire();

	//��������:
	//������ ������� ���������� ����� �� 2 ������ �������, ������� ���������� ��� ���� �� ����� ��� ������ ������ ����
	//� ����������� �� ������ ������� �������� �������, � ������� ����� ������� �����(������������/��������������)
	void StartFire(int position);

	//��������:
	//������� ���������� � ������ ���������� ������ ����
	//� ������ ���������� ������������ ��� ������ ����� �����
	void StopFire();


	//��������:
	//������ ����� ������������ ������ �������� - ����
	//������������ ��� ����������� ������ �������
	void ProcessEvent(SEntityEvent& event);

	//��������:
	//������ ����� �������� ������� � ����������� �����
	//� �������� ����� ���� ��������� �������� �� �������� ���� ����������� ����������� ��������(������������ ��������� 2 �������)
	//����� ���� ������� ����� � ��������. ������������� ������ �����. 
	//����� ������ � ����� ��������, ����������� � ��������� ���������� � Hit()
	void PerfomIntersection();


	//��������:
	//������ ������� ���������� ������, �� ������ ����������, ��� �������� ������ ���������� � ������������� �����
	//pt  - ����� �����������
	//dir - ����������� �����
	//normal - ������� �����
	//pCollider - ���������� ��������� ��������, ������� ����������� ����/�������
	//collidedEntityId - id �������� ����
	//pratId - (?) �������� ������������ ��� ���������� �����, �� ������� ������ ����
	//ipart (?)
	//surfaceIdx - ������ ����������� �� ������� ���� ����(������, �����, ������ � ��)
	void Hit(const Vec3 &pt, const Vec3 &dir, const Vec3 &normal, IPhysicalEntity *pCollider, EntityId collidedEntityId, int partId, int ipart, int surfaceIdx);
	
	//��������:
	//������� ���������� ��� ������������� ���� ��� ���
	//����������: true - ���� ���� �������, ����� false
	bool IsFriendlyHit(IEntity* pShooter, IEntity* pTarget);

	//��������
	//������� ����������� ������ ����� � ����� �����
	//���������: position - ����� �����, noramal - �������, surfaceIdx - ������ �����������
	void PlayHitMaterialEffect(const Vec3 &position, const Vec3 &normal, int surfaceIdx);

	bool IsBusy() const { return m_bBusy; }
	void SetBusy(bool busy){ m_bBusy = busy; }

	//��������
	DEPRICATED int GetComboMoveCount(int combo);

	//��������:
	//������� ���������� ��������� �� ��������, ������ ����������� ���������� ���
	CHolding* GetHoldingStatus(){ return m_Holding; }

	//��������:
	//������� ���������� ��������� �� �����, � ������� ���� �������� ������ ������
	CCombatStance* GetCombatStance(){ return m_CombatStance; }
	int GetWeaponType(){ return e_WeaponTypeSword; }

	//��������:
	//������� ������ ��������� ����������� �������
	void DebugDraw();

private:
	//��������:
	//������� �������� ����, ������� ��������� � �������, ��� ���������� ��������
	void ClearTag(uint tag);
	void ClearAllTags();

	//���� �� ������������
	DEPRICATED CMouseDirection m_MouseDir;

	CHolding* m_Holding;
	CCombatStance* m_CombatStance;

	//������ �������� ������������ ��� ����������� ����������� ���������
	primitives::cylinder* cyl;

	Vec3 m_prevPt; // ��� �������� ����������� �������� ����
	
	bool m_bNowAttack;
	bool m_bBusy;			//Weapon busy
	bool m_bHitSoundPlayed; //��������, �������� �� ���� 

	int m_selectedCombo;	//One of combo move set. ECombo
	int m_comboMoveCount;   //Count of actions in current combo
	int m_performedMoveCount;	//performed move count of current combo
};