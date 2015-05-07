// Char parameters
// Класс базовых характеристик персонажа
// Dmitriy Miroshnichenko
// 9.03.15

#include "StdAfx.h"
#include "CharParameters.h"
#include "Player.h"


CCharParameters::CCharParameters(){
	m_health = 0; m_bonusHealth = 0;
	m_agility = 0; m_bonusAgility = 0;
	m_stamina = 0; m_bonusStamina = 0;
	m_strength = 0; m_bonusStrength = 0;
}

CCharParameters::CCharParameters(int health, int agility, int stamina, int strength){
	m_health = health; m_bonusHealth = 0;
	m_agility = agility; m_bonusAgility = 0;
	m_stamina = stamina; m_bonusStamina = 0;
	m_strength = strength; m_bonusStrength = 0;
}

// -------------------------------------
// геттеры

int CCharParameters::getHealth(string type){
	if (type == "full")
		return m_health + m_bonusHealth;
	if (type == "base")
		return m_health;
	if (type == "bonus")
		return m_bonusHealth;
}

int CCharParameters::getAgility(string type){
	if (type == "full")
		return m_agility + m_bonusAgility;
	if (type == "base")
		return m_agility;
	if (type == "bonus")
		return m_bonusAgility;
}

int CCharParameters::getStamina(string type){
	if (type == "full")
		return m_stamina + m_bonusStamina;
	if (type == "base")
		return m_stamina;
	if (type == "bonus")
		return m_bonusStamina;
}

int CCharParameters::getStrength(string type){
	if (type == "full")
		return m_strength + m_bonusStrength;
	if (type == "base")
		return m_strength;
	if (type == "bonus")
		return m_bonusStrength;
}

float CCharParameters::getDamage(int damage){
	return damage + m_strength * 0.37 + m_bonusStrength * 0.49;
}

float CCharParameters::getSprintSpeed(float sprint){
	return sprint + m_stamina * 0.23 + m_agility * 0.46;
}

// -------------------------------------
// сеттеры

void CCharParameters::setHealth(int value, string type, bool add){
	if (add)
		value += getHealth(type);

	if (type == "base")
		m_health = value;
	if (type == "bonus")
		m_bonusHealth = value;

	// устанавливаем персонажу хп
	CPlayer *player = (CPlayer*)g_pGame->GetIGameFramework()->GetClientActor();
	if (player != NULL)
		player->SetMaxHealth( (float) getHealth("full") );
}

void CCharParameters::setAgility(int value, string type, bool add){
	if (add)
		value += getAgility(type);

	if (type == "base")
		m_agility = value;
	if (type == "bonus")
		m_bonusAgility = value;
}
void CCharParameters::setStamina(int value, string type, bool add){
	if (add)
		value += getStamina(type);

	if (type == "base")
		m_stamina = value;
	if (type == "bonus")
		m_bonusStamina = value;
}
void CCharParameters::setStrength(int value, string type, bool add){
	if (add)
		value += getStrength(type);

	if (type == "base")
		m_strength = value;
	if (type == "bonus")
		m_bonusStrength = value;
}