/****************************************

* File: PlayerStats.h

* Description: ����� ����������� ������� ���������� ������ �����

* Created by: Andrew Medvedev

* Date: 15.04.2014

* Diko Source File

*****************************************/
#ifndef _PLAYER_STATS_
#define _PLAYER_STATS_

#include "StdAfx.h"

class CPlayerStat
{
	float m_fValue; // �������� �����(��, ����, ������ � ��)
	float m_fBonus; // ����� � ��������� ��������
	int m_nType; // ���. ��. EPlayerStats
	string m_sName; // ��� ��� ����������� �� ����� � ��.
	string m_sDescription; // �������� ��� ����������� �� ����� � ��.
public:
	//��������:
	//����������� �� ���������
	CPlayerStat();

	//��������:
	//������� ���������� �������� ������-���� �����(�������)
	float GetStatValue(){ return m_fValue; }

	//��������:
	//������� ���������� �������� ������ ��� ������� �����
	float GetStatBonus(){ return m_fBonus; }

	//��������:
	//������� ���������� ��� �����(��, ����, �������� � ��)
	int GetType(){ return m_nType; }

	//��������:
	//������� ���������� �������� ����� - "��������", "����"...
	string GetStatName(){ return m_sName; }

	//��������:
	//������� ���������� �������� �����: "�������� ������..."
	string GetStatDescription(){ return m_sDescription; }


	//������ �������� ��� ��������� �������� � ��������� ��������
	void SetValue(float value);
	void SetBonus(float bonus);

	//��������:
	//������ ������� ��������� ��������� ����� �� ���-����� � ��������� ���� ������, ���������� ��� �����
	bool InitStat(int type);

	//��������:
	//������� ��������� ����-��������� � ���, ��� ������ ���� ��� �������.
	//��� ����� ��� ����, ��� �� ������ ������, ������������ ��������� ���������, �������� �������� � ������� ����������
	void OnStatChange(CPlayerStat* stat);
};

#endif