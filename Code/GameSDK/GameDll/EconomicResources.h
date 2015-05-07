/****************************************

* File: EconomicResources.h/cpp

* Description: Класс и свойства ресурсов

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
	int m_type; // тип ресурса
	int m_count; // количество данного ресурса
};

class CEconomicResource : public SResource {
	int m_cost; // цена за единицу
	int m_maxCount; // макс. кол-во
	string m_name; // строковое имя ресурса
	int m_modifier; // модификатор -- сколько ресурса прибавляется в минуту

	// ост. параметры - ниже
public:
	CEconomicResource();
	CEconomicResource(int type, int count, int cost, int maxCount, string name);

	// сеттеры и геттеры
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