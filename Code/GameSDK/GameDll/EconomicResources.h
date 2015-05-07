/****************************************

* File: EconomicResources.h/cpp

* Description: ����� � �������� ��������

* Created by: Dmitriy Miroshnichenko & Andrew Medvedev

* Date: 24.11.2014

* Diko Source File

*****************************************/

#pragma once
#include "Game.h"

enum EEconomicResource {
	e_EconomicResourceWood,
	e_EconomicResourceIron,
	e_EconomicResourceStone
};

struct SResource {
	int m_type; // ��� �������
	int m_count; // ���������� ������� �������
};

class CEconomicResource : public SResource {
	int m_cost; // ���� �� �������
	int m_maxCount; // ����. ���-��
	string m_name; // ��������� ��� �������
	int m_modifier; // ����������� -- ������� ������� ������������ � ������

	// ���. ��������� - ����
public:
	CEconomicResource();
	CEconomicResource(int type, int count, int cost, int maxCount, string name);

	// ������� � �������
	void SetType(int type);
	void SetCount(int count, bool add = false);
	void SetCost(int cost);
	void SetMaxCount(int maxCount);
	void SetName(string name);
	void SetModifier(int modifier);

	int GetType();
	int GetCount();
	int GetCost();
	int GetMaxCount();
	string GetName();
	int GetModifier();
};