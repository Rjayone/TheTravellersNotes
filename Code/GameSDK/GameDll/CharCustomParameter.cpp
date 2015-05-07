// Char Custom Parameter
// Класс кастомной характеристики персонажа
// Dmitriy Miroshnichenko
// 12.03.15

#include "StdAfx.h"
#include "CharCustomParameter.h"

CCharCustomParameter::CCharCustomParameter(std::string name, int value){
	m_name = name;
	m_value = value;
	m_bonus = 0;
	m_valueCoefficient = 1.0;
	m_bonusCoefficient = 1.0;
}

std::string CCharCustomParameter::GetName(){
	return m_name;
}
int CCharCustomParameter::GetValue(){
	return m_value;
}
int CCharCustomParameter::GetBonus(){
	return m_bonus;
}
int CCharCustomParameter::GetFullValue(){
	return m_value * m_valueCoefficient + m_bonus * m_bonusCoefficient;
}

void CCharCustomParameter::SetValue(int value){
	m_value = value;
}
void CCharCustomParameter::SetBonus(int bonus){
	m_bonus = bonus;
}
void CCharCustomParameter::SetValueCoefficient(float coefficient){
	m_valueCoefficient = coefficient;
}
void CCharCustomParameter::SetBonusCoefficient(float coefficient){
	m_bonusCoefficient = coefficient;
}

void CCharCustomParameter::askAccept(){
	// если игрок совершил какое-то действие,
	// ну, например, убил миллиард врагов,
	// запускается эта функция, которая
	// предлагает ему новую характеристику
}