/****************************************************************************************************
* Weapon: Torch
* Description: Класс факела
* Created by: Dmitriy Miroshnichenko & Andrew Medvedev
* Date: 25.01.2015
* Edda Studio
****************************************************************************************************/

#pragma once
#include "Weapon.h"
#include "WeaponProperties.h"

class CTorch : public CWeapon {
public:
	CTorch();
	void Release();

	bool Init(IGameObject * pGameObject);
	void Update(SEntityUpdateContext& ctx, int slot);

	void Select(bool select);
	void ProcessEvent(SEntityEvent& event);

	//Описание:
	//Функция задает параметры отображения хелпера
	void DebugDraw();

	//Описание:
	//Функция включает свет и эффект огня факела
	void Fire();
private:
	IEntity* m_pParticalEntity; // Огонь
	CDLight* m_pLight;

	bool m_bIsFired; // Указывает горит факел или нет
	float m_fFireDuration; // Длительность горения
	float m_fDiffuseMult; //Коэф яркости свечения
};