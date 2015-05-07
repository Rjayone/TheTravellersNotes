/****************************************************************************************************
* Mouse Direction 
* Description: ����� ���������� � ����� ����������� ���� �������� ����� ����� ����� � ���������� ������
* Created by: Andrew Medvedev
* Date: 29.09.2014
* Diko Source File
****************************************************************************************************/
#pragma once

#include "Game.h"

class CMouseDirection
{
	Vec2 m_Direction;

	Vec2 m_FirstPoint;
	Vec2 m_SecondPoint;

	bool m_bDragEnable;
	float m_fLastAngle;
public:
	CMouseDirection();

	//������� ������ ��������� ��������� ����� �� ������(�, �)
	void SetPoint();

	//������� ������������� ������ ����� �� ������(�, �)
	//�����. �������� ����
	float ClosePath();

	//������� ���������� ���� �������� �� ����������� � � �.
	float GetAngle(float _x, float _y);
	float GetAngle(){ return m_fLastAngle; }

	//true ���� ����� �� �������
	bool IsDragging(){ return m_bDragEnable; }

	Vec2 GetDir(){ return m_Direction; }

	//������� ������������� ��� ����������� ������.
	//�������� �� ������ ����� ��������� ����������� ���� � ������������ �����������.
	Vec2 PerfomClosePath();
};