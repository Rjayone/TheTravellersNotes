// Character development system
// Класс описывающий систему развития персонажей
// Andrew Medvedev & Dmitriy Miroshnichenko
// Diko
// 07.04.13

#pragma once
#include <IGameFramework.h>
#include "CharParameters.h"
#include "CharCustomParameter.h"
// перенести в класс еды?
enum EFoodType {
	EFTLight = 0,
	EFTNormal,
	EFTSatiety,
	EFTHPFlask,			// еда, увеличивающая
	EFTStrengthFlask,	// характеристики
	EFTAgilityFlask,
	EFTStaminaFlask
};

enum EHungry {
	EHNormal = 3,
	EHSatiety = 4,
	EHHungry = 2,
	EHVeryHungry = 1
};

class CCharacterDevelopmentSystem : public IGameFrameworkListener {
public:
	CCharacterDevelopmentSystem();

	int GetLevel();
	int GetExp();
	int GetExpForKilling();
	int GetHungry();
	int GetHungryWater();

	void SetLevel(int level, bool add = false);
	void SetExp(int exp, bool add=false);
	void SetHungry(int hungry);
	void SetHungryWater(int hungry);
	void UpLevel();
	void CheckHungry();
	void Regeneration();
	virtual int ChangeHungryStance(int type);

	// IGameFrameworkListener
	VIRTUAL void OnPostUpdate(float fDeltaTime);
	VIRTUAL void OnSaveGame(ISaveGame* pSaveGame);
	VIRTUAL void OnLoadGame(ILoadGame* pLoadGame);
	VIRTUAL void OnLevelEnd(const char* nextLevel);
	VIRTUAL void OnActionEvent(const SActionEvent& event);
	// ~IGameFrameworkListener
private:
	int m_level; // уровень
	int m_exp; // опыт
	int m_currentHealth; // текущее здоровье
	int m_hungry;
	int m_hungryWater;
	int m_hungryTimeLimit;
	int m_hungryWaterTimeLimit;
	int m_regen; // сколько хп регенится за раз
	int m_regenLimit; // через какое время происходит реген

	float m_hungryTime; // время последнего обновления голода
	float m_hungryWaterTime;
	float m_regenTime;

	IGameFramework* pGameFramework;
	CCharParameters* pParameters;
	std::vector<CCharCustomParameter*> pCustomParameters;
//	SCDSPlayerParams *parameters;
};


// количество опыта, нужное для уровня
#define level_exp(k) k*(42*k*k + 259) - 78
// до упрощения: 3 * (K*K*K) + (7 * K * 37) + 39 * ((K*K)*K - 2)

// сколько прибавить характеристике по достижении уровня
#define level_health(k) k*10
#define level_agility(k) k*10
#define level_stamina(k) k*10
#define level_strength(k) k*10

#define hungryTimeLimit 10