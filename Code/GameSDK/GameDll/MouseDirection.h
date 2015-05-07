/****************************************************************************************************
* Mouse Direction 
* Description: Класс определяет в каком направлении была сдвинута мышка после клика и отпускания кнопки
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

	//Функция задает начальное положение точки на экране(х, у)
	void SetPoint();

	//Функция устанавливает вторую точку на экране(х, у)
	//Возвр. значение угла
	float ClosePath();

	//Функция возвращает угол поворота по координатам х и у.
	float GetAngle(float _x, float _y);
	float GetAngle(){ return m_fLastAngle; }

	//true если точка не закрыта
	bool IsDragging(){ return m_bDragEnable; }

	Vec2 GetDir(){ return m_Direction; }

	//Функция предназначена для покадрового вызова.
	//Опираясь на первую точку покадрово закрывается путь и определяется направление.
	Vec2 PerfomClosePath();
};