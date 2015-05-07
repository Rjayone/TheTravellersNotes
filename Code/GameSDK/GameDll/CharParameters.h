// Char parameters
// Класс базовых характеристик персонажа
// Dmitriy Miroshnichenko
// 9.03.15

#pragma once
#include "StdAfx.h"

class CCharParameters {
public:
	CCharParameters();
	CCharParameters(int health, int agility, int stamina, int strength);

	int getHealth(string type = "full"); // 0=base+bonus, 1=base, 2=bonus
	int getAgility(string type = "full");
	int getStamina(string type = "full");
	int getStrength(string type = "full");
	void setHealth(int value, string type = "base", bool add = false);
	void setAgility(int value, string type = "base", bool add = false);
	void setStamina(int value, string type = "base", bool add = false);
	void setStrength(int value, string type = "base", bool add = false);

	// вычисляемые параметры
	float getDamage(int damage);
	float getSprintSpeed(float sprint);

private:
	int m_health; int m_bonusHealth;
	int m_agility; int m_bonusAgility;
	int m_stamina; int m_bonusStamina;
	int m_strength; int m_bonusStrength;
};