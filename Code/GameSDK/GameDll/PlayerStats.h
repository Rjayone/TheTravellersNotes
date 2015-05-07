/****************************************

* File: PlayerStats.h

* Description: Класс описывающий базовый функционал любого стата

* Created by: Andrew Medvedev

* Date: 15.04.2014

* Diko Source File

*****************************************/
#ifndef _PLAYER_STATS_
#define _PLAYER_STATS_

#include "StdAfx.h"

class CPlayerStat
{
	float m_fValue; // Значение стата(хп, мана, деньги и тд)
	float m_fBonus; // Бонус к основному значению
	int m_nType; // Тип. См. EPlayerStats
	string m_sName; // Имя для отображения во флеше и пр.
	string m_sDescription; // Описание для отображения во флеше и пр.
public:
	//Описание:
	//Конструктор по умолчанию
	CPlayerStat();

	//Описание:
	//Функция возвращает значение какого-либо стата(базовое)
	float GetStatValue(){ return m_fValue; }

	//Описание:
	//Функция возвращает значение бонуса для данного стата
	float GetStatBonus(){ return m_fBonus; }

	//Описание:
	//Функция возвращает тип стата(хп, сила, ловкость и тд)
	int GetType(){ return m_nType; }

	//Описание:
	//Функция возвращает название стата - "Здоровье", "Сила"...
	string GetStatName(){ return m_sName; }

	//Описание:
	//Функция возвращает описание стата: "Здоровье игрока..."
	string GetStatDescription(){ return m_sDescription; }


	//Группа сеттеров для установки базового и бонусного значения
	void SetValue(float value);
	void SetBonus(float bonus);

	//Описание:
	//Данная функция считывает параметры стата из хмл-файла и заполняет поля класса, определяет тип стата
	bool InitStat(int type);

	//Описание:
	//Функция оповещает стат-менеджера о том, что данный стат был изменен.
	//Это нужно для того, что бы другие классы, использующие некоторые параметры, обновили значения и сделали перерасчет
	void OnStatChange(CPlayerStat* stat);
};

#endif