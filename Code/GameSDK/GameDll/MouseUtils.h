/****************************************
* File: MouseUtils.h
* Description: Методы работы с мышкой
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

	//Описание:
	//Функция возвращает указатель на сущность, на которую смотрит игрок,
	//Иначе NULL если сущность не в зоне досягаемсоти или отсутсвует.
	static IEntity* GetMouseEntity(float rayLength);

	//Описание:
	//Функция возвращает структуру ray_hit с параметрами точки, в которую смотрел игрок.
	//В структуре идет описание точки пересечения луча(ray) и какого-либо объекта/сущности/земли.
	static ray_hit  GetHitData(float rayLength = 10);


	//Группа методов для определения шорт-кликов
	//
	//Описание:
	//Метод инициализирует момент нажатия левой кнопки мыши
	void InitClickTime();

	//Описание:
	//Метод срабатывает после поднятия левой кнопки мыши
	void OnClickUp();
	float OnClickUp(bool force);

	//Описание:
	//Метод рассчитывает время клика
	float CalculateClickTime();

	float m_fInitialTime;
	float m_fClickUpTime;
	//
	//~
};

#endif