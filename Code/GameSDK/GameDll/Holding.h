/****************************************************************************************************
* Firemode: Holding
* Description: ���� ��� ��� ��������� ����������� ����������� ���� ��� ������
* Created by: Andrew Medvedev
* Date: 29.09.2014
* Diko Source File
****************************************************************************************************/

#pragma once

#include "Game.h"
#include "MouseDirection.h"
#include "MouseUtils.h"
#include "FireMode.h"

enum EHoldingStatus
{
	e_HoldingTypeIdle,
	e_HoldingStatusSwing,
	e_HoldingStatusHold,
	e_HoldingStatusRelease,
	e_HoldingStatusCancel,
	e_HoldingStatusShortClick
};

//================================
class CHolding : public IActionListener
{
public:
	CHolding();
	~CHolding();

	//��������:
	//����� �������������� ����
	bool Init(CItem *pItem);

	//��������:
	//���������� ����������
	void Update();
	void ProcessEvent(SEntityEvent& event);


	//��������:
	//������������� ����� �� IActionListener ��� ����������� �������
	void OnAction(const ActionId& action, int activationMode, float value);
	IEntity* GetEntity();

	//������ ������� ����������� �������� ����� ������
	//
	//��������:
	//����� ����������� �������� ������ �������
	void PerfomSwing(CItem *pItem, int stance);	//���� �� ����� ������ ���� �������� ������ �� ������������� ������ �����

	//��������:
	//����� ��������� ���-�������� ����������� ���� � ������ ������
	void OnHold(int stance);

	//��������:
	//����� ����������� �������� ����� � ���������� ������, ����� ���� ������ �����
	void PerfomRelease();

	//��������:
	//����� ����������, ����� ����� �� ����� ����������� ���� - �������� ������ ������
	void CancelHolding();

	//~


	//��������:
	//����� �������������� ������ ������ ������ �������
	void StartFire(CItem *pItem);
	void StopFire(CItem *pItem);

	//��������:
	//����� ��� ���� �����. ������ ���� ��������� ��� ������� ������
	void DefaultAttack(int stance = 0);

	//��������:
	//������ �� ������� � ������� ���������/��������� �������� ������� ��������� ����
	//��. EHoldingStatus
	void SetStatus(EHoldingStatus status);
	int GetStatus(){ return m_nStatus; }
	//~

	//��������:
	//���������� ����� ��������� ������ ����� �������
	float GetHoldingTime(){ return m_pMouseUtils ? m_pMouseUtils->CalculateClickTime() : 0.f; }

	//��������:
	//���������� ������������ ��������� � ���������
	float GetFragmentPlayDuration(IActionPtr pAction);
	
	//��������:
	//������� ������������� ���� � ��������
	void SetTag(uint32 id, bool set);

	//��������:
	//������� �������� ���� � ��������� ���������
	void ClearAllHoldingTags();
private:
	int  m_nStatus;
	bool m_bShortAttack;
	bool m_bHolding;

	CMouseUtils* m_pMouseUtils;
	CItem *m_pItem;
};