/****************************************
* File: MouseUtils.h
* Description: ������ ������ � ������
* Created by: Medvedev Andrew
* Date: 25.01.2014
* Update: 04.10.2014
* Edda Source File
* ToDo:
*****************************************/

#ifndef _MOUSE_UTILS_
#define _MOUSE_UTILS_

#include "Game.h"

class IGetMouseEntity
{
public:
	virtual IEntity* GetMouseEntity(int rayLength) = 0;
};


//-----------------------------------------------------------
class CMouseUtils : public IActionListener
{
public:
	CMouseUtils();
	void OnAction(const ActionId& action, int activationMode, float value);

	//��������:
	//������� ���������� ��������� �� ��������, �� ������� ������� �����,
	//����� NULL ���� �������� �� � ���� ������������ ��� ����������.
	static IEntity* GetMouseEntity(float rayLength);

	//��������:
	//������� ���������� ��������� ray_hit � ����������� �����, � ������� ������� �����.
	//� ��������� ���� �������� ����� ����������� ����(ray) � ������-���� �������/��������/�����.
	static ray_hit  GetHitData(float rayLength = 10);


	//������ ������� ��� ����������� ����-������
	//
	//��������:
	//����� �������������� ������ ������� ����� ������ ����
	void InitClickTime();

	//��������:
	//����� ����������� ����� �������� ����� ������ ����
	void OnClickUp();
	float OnClickUp(bool force);

	//��������:
	//����� ������������ ����� �����
	float CalculateClickTime();

	float m_fInitialTime;
	float m_fClickUpTime;
	//
	//~
};

#endif