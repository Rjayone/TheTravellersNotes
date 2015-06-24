/****************************************************************************************************
* Fiture: Player stances
* Description: ����� ��������� ������ ������
* Created by: Andrew Medvedev
* Date: 29.09.2014
* Diko Source File
****************************************************************************************************/
#pragma once

#include "StdAfx.h"
#include "Game.h"
#include "Item.h"
#include "CombatTargetManager.h"


/***********************************************
* List Use
------------------------------------------------
* Melee.cpp / OnHit() line: 795
*/


//�������� �����.
//����� ������ ������� ���� ������� ������ ����.
//������ ���������� ��������� ������ �� ���
enum ECombatStance
{
	e_CombatStanceVertical,		//������ ������������� ��� ������������ ����
	e_CombatStanceHorizontal,	//��� �������������� ����
	e_CombatStancePricking,		//� ������ ������ �� ����� ����������� ������� �����
	e_CombatStanceDefence		//������������� �������� ������.

	//��������� ������ �������� ����.
};


//����� ��������� ����� ����� ��� �������� ����� ������
//����� �������� ����������� ���� � �������, ���� ���� ����������� �������� �������� ������������ � ��
//� ����������� �� ������ ������ ������� ���� ��������.
class CCombatStance : public IActionListener
{
public:
	CCombatStance();
	~CCombatStance();

	//��������:
	//������� ��������� �� ������ ������ ������� ������, � �� �� ���������
	void OnAction(const ActionId& action, int activationMode, float value);

	//��������:
	//������� ���������� ��� ������ ������ � ��������� ������.
	//���������: ��������� �� ���� ������
	void Init(CItem* pItem);

	//��������:
	//������� �������� �� ���������� �����.
	//� ������ ������ ������������ ��� ������� ������
	void Update();
	
	//��������
	//������� ������ ������, ������ ���� ��������, � ����������� ��������.
	//���������: ��� ������ �� ECombatStance.
	void ApplyStance(int stance = 1);

	//������� ������ � ������
	//��������:
	//������� ������������� ���� � ��������
	void SetTag(uint32 id, bool set);

	//��������:
	//������� �������� ���� � ��������� ���������
	void ClearAllStanceTags();
	//~

	int GetStance(){ return m_Stance; }
private:
	void SetStance(int stance);

	int m_Stance;
	CItem* m_pItem;
	CCombatTarget* m_pCombatTarget;
};