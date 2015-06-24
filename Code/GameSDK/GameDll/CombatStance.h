/****************************************************************************************************
* Fiture: Player stances
* Description: Класс описывает стойки игрока
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


//Описание стоек.
//Стоит учесть идругие виды оружжия помимо меча.
//Данная реализация расчитана только на меч
enum ECombatStance
{
	e_CombatStanceVertical,		//Стойка предназначена для вертикальных атак
	e_CombatStanceHorizontal,	//Для горизонтальных атак
	e_CombatStancePricking,		//В данной стойке мы можем производить колющие удары
	e_CombatStanceDefence		//Исключительно защитная стойка.

	//Остальные стойки добавить выше.
};


//Класс реализует смену стоек для различия типов ударов
//Стоит добавить специальные теги в маникен, дабы была возможность изменять анимацию передвижения и тд
//В зависимости от стойки должна менятся идел анимация.
class CCombatStance : public IActionListener
{
public:
	CCombatStance();
	~CCombatStance();

	//Описание:
	//Функция принимает из класса оружия нажатую кнопку, а не из слушателя
	void OnAction(const ActionId& action, int activationMode, float value);

	//Описание:
	//Функция вызывается при выборе оружия и назначает стойку.
	//Принимает: Указатель на итем игрока
	void Init(CItem* pItem);

	//Описание:
	//Функция отвечает за покадровый вызов.
	//В данный момент используется для задания стойки
	void Update();
	
	//Описание
	//Функция задает стойку, задает теги маникена, и переключает анимации.
	//Принимает: тип стойки из ECombatStance.
	void ApplyStance(int stance = 1);

	//Функции работы с тегами
	//Описание:
	//Функция устанавливает теги в маникене
	void SetTag(uint32 id, bool set);

	//Описание:
	//Функция отчищает теги в дефолтное состояние
	void ClearAllStanceTags();
	//~

	int GetStance(){ return m_Stance; }
private:
	void SetStance(int stance);

	int m_Stance;
	CItem* m_pItem;
	CCombatTarget* m_pCombatTarget;
};