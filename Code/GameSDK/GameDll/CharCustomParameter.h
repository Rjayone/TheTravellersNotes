// Char Custom Parameter
//  ласс кастомной характеристики персонажа
// Dmitriy Miroshnichenko
// 12.03.15
#pragma once
#include "StdAfx.h"

class CCharCustomParameter {
public:
	CCharCustomParameter(std::string name, int value);

	std::string GetName();
	int GetValue();
	int GetFullValue();
	int GetBonus();
	float GetValueCoefficient();
	float GetBonusCoefficient();

	void SetValue(int value);
	void SetBonus(int bonus);
	void SetValueCoefficient(float coefficient);
	void SetBonusCoefficient(float coefficient);

	void askAccept();

private:
	std::string m_name;
	int m_value;
	int m_bonus;
	float m_valueCoefficient;
	float m_bonusCoefficient;
};