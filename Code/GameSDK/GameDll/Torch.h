/****************************************************************************************************
* Weapon: Torch
* Description: ����� ������
* Created by: Dmitriy Miroshnichenko & Andrew Medvedev
* Date: 25.01.2015
* Edda Studio
****************************************************************************************************/

#pragma once
#include "Weapon.h"
#include "WeaponProperties.h"

class CTorch : public CWeapon {
public:
	CTorch();
	void Release();

	bool Init(IGameObject * pGameObject);
	void Update(SEntityUpdateContext& ctx, int slot);

	void Select(bool select);
	void ProcessEvent(SEntityEvent& event);

	//��������:
	//������� ������ ��������� ����������� �������
	void DebugDraw();

	//��������:
	//������� �������� ���� � ������ ���� ������
	void Fire();
private:
	IEntity* m_pParticalEntity; // �����
	CDLight* m_pLight;

	bool m_bIsFired; // ��������� ����� ����� ��� ���
	float m_fFireDuration; // ������������ �������
	float m_fDiffuseMult; //���� ������� ��������
};