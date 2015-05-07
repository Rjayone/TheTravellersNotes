#include "StdAfx.h"
#include "CharDevSys.h"
//#include "Game.h"
#include "PlayerMovementController.h"
#include "PlayerStatsManager.h"
//#include "ISerialize.h"
#include "ISaveGame.h"
//#include "Player.h"
#include "IFlashUI.h"


CCharacterDevelopmentSystem::CCharacterDevelopmentSystem(){
	pGameFramework = g_pGame->GetIGameFramework();
	if (pGameFramework == NULL)
		return;
	pGameFramework->RegisterListener(this, "CharDevSys", FRAMEWORKLISTENERPRIORITY_DEFAULT);

	m_level = 1;
	m_exp = 0; // getNewExp();
	m_hungry = EHSatiety;
	m_hungryWater = EHSatiety;
	m_hungryTimeLimit = 100;
	m_hungryWaterTimeLimit = 150;
	m_regenLimit = 10;
	m_regenTime = 0;
	pParameters = new CCharParameters(100, 0, 0, 0);
	
	// примеры доп. характеристик :)

	// скрытность
	pCustomParameters.push_back(new CCharCustomParameter("stealth", 1));
	// воровство
	pCustomParameters.push_back(new CCharCustomParameter("theft", 1)); // гг может ведь воровать? :)
	// непробиваемость
	pCustomParameters.push_back(new CCharCustomParameter("impenetrability", 1));
	// красноречие
	pCustomParameters.push_back(new CCharCustomParameter("eloquence", 1));
	// торговля
	pCustomParameters.push_back(new CCharCustomParameter("trade", 1));
	// харизма
	pCustomParameters.push_back(new CCharCustomParameter("charisma", 1));
	// жестокость
	pCustomParameters.push_back(new CCharCustomParameter("cruelty", 1));
	// крафт (бонусы к профам)
	pCustomParameters.push_back(new CCharCustomParameter("craft", 1));
}

// --------------------------------------
int CCharacterDevelopmentSystem::GetLevel(){
	return m_level;
}

int CCharacterDevelopmentSystem::GetExp(){
	return m_exp;
}

int CCharacterDevelopmentSystem::GetExpForKilling(){
	int k;
	if (m_level >= 1 && m_level <= 9)
		k = 7;
	else if (m_level >= 10 && m_level <= 15)
		k = 8;
	else if (m_level >= 16 && m_level <= 19)
		k = 9;
	else if (m_level >= 20)
		k = 11;
	else
		k = 1;

	return k * (k + 13);
}

int CCharacterDevelopmentSystem::GetHungry(){
	return m_hungry;
}

int CCharacterDevelopmentSystem::GetHungryWater(){
	return m_hungryWater;
}

// ----------------------------------------
void CCharacterDevelopmentSystem::SetLevel(int level, bool add){
	if (add)
		m_level += level;
	else
		m_level = level;

	pParameters->setAgility(level_agility(m_level), "base", true);
	pParameters->setStamina(level_stamina(m_level), "base", true);
	pParameters->setStrength(level_strength(m_level), "base", true);
	pParameters->setHealth(level_health(m_level), "base", true);

//	if (g_pGameCVars->pl_movement.sprint_SpeedScale< 2)
//		g_pGameCVars->pl_movement.sprint_SpeedScale += PlayerParam->Stamina*0.007;
	//GetSprintSpeed(1.7);
}

void CCharacterDevelopmentSystem::SetExp(int exp, bool add){
	if (add)
		m_exp += exp;
	else
		m_exp = exp;

	if (m_exp >= level_exp(m_level))
		UpLevel();

	if (gEnv->pFlashUI){
		IUIElement *pUIElement = gEnv->pFlashUI->GetUIElement("ExperienceBar");
		if (!pUIElement)
			return;
		pUIElement->SetVisible(true);

		SUIArguments Set, Add;
		Set.AddArgument<int>(m_exp);
		Set.AddArgument<int>(level_exp(m_level));
		Add.AddArgument(GetExpForKilling());
		pUIElement->CallFunction("SetExperienceValue", Set);
		pUIElement->CallFunction("AddExperienceMovieClip", Add);
	}
}

void CCharacterDevelopmentSystem::SetHungry(int hungry){
	m_hungry = hungry;

	switch (hungry){
	case EHSatiety:
		pParameters->setHealth(30, "bonus", true);
		m_regen = 10;
	case EHNormal:
		pParameters->setHealth(-30, "bonus", true);
		m_regen = 5;
	case EHHungry:
		pParameters->setHealth(-10, "bonus", true);
		m_regen = 0;
	case EHVeryHungry:
		pParameters->setHealth(0, "bonus");
		m_regen = -10;
	}
}

void CCharacterDevelopmentSystem::SetHungryWater(int hungry){
	m_hungryWater = hungry;
}

void CCharacterDevelopmentSystem::UpLevel(){
	SetLevel(m_level + 1);
	m_exp = m_exp - level_exp(m_level);
}

void CCharacterDevelopmentSystem::CheckHungry(){
	float now = gEnv->pTimer->GetCurrTime();
	if (!m_hungryTime){
		m_hungryTime = now;
		return;
	}
	if (!(now - m_hungryTime >= hungryTimeLimit))
		return;

	switch (m_hungry){
	// TODO: должно быть разное время -- сытость проходит быстрее, чем наступает голод
	case EHSatiety: 
		SetHungry(EHNormal);
	case EHNormal:
		SetHungry(EHHungry);
	case EHHungry:
		SetHungry(EHVeryHungry);
	case EHVeryHungry:;
	}
}

// функция вызывается в апдейте, проверяет, не пора ли регенить
void CCharacterDevelopmentSystem::Regeneration()
{
	//Я что-то накосячил. Нужно пофиксить
	return;

	IActor *player = g_pGame->GetIGameFramework()->GetClientActor();
	if (player || player->GetHealth() <= player->GetMaxHealth())//Реген на хп	
		player->SetHealth( player->GetHealth() + m_regen );

	//Реген на силу
	CPlayerStatsManager* manager = g_pGame->GetPlayerStatsManager();
	manager->GetStat(EPS_Strength)->SetValue(manager->GetStat(EPS_Strength)->GetStatValue() + m_regen);
}

int CCharacterDevelopmentSystem::ChangeHungryStance(int type){
	return 0;
}

void CCharacterDevelopmentSystem::OnPostUpdate(float fDeltaTime){
	// проверяем, не проголодался ли перс
	CheckHungry();
	// и не нужно ли ему что-нибудь отрегенить
	Regeneration();

	//Данный код нужно будет чутка по-другому реализовать
	//Пускай будет пока закоменчен
	// HUD
	//if (gEnv->pFlashUI)
	//{
	//	IUIElement *pUIElement = gEnv->pFlashUI->GetUIElement("ExperienceBar");
	//	if (!pUIElement)
	//		return;
	//
	//	if (gEnv->IsEditorGameMode())
	//	{
	//		if (!pUIElement->IsVisible())
	//		{
	//			pUIElement->SetVisible(true);
	//
	//			SUIArguments Set, Add;
	//			Set.AddArgument<int>(m_exp);
	//			Set.AddArgument<int>(level_exp(m_level));
	//			pUIElement->CallFunction("SetExperienceValue", Set);
	//		}
	//	}
	//	else
	//	{
	//		pUIElement->SetVisible(false);
	//		pUIElement->Unload();
	//	}
	//}
}

void CCharacterDevelopmentSystem::OnSaveGame(ISaveGame* pSaveGame){
	TSerialize ser = pSaveGame->AddSection("CharDevSystem");
	bool isEnd = false;
	if (ser.IsWriting() && pParameters){
		ser.BeginGroup("PlayerParams");
		isEnd = true;
	}

	ser.Value("Level", m_level);
	ser.Value("Exp", m_exp);
	ser.Value("Hungry", m_hungry);
	ser.Value("HungryWater", m_hungryWater);

//	pParameters->Serialize(ser);
//	pCustomParameters->Serialize(ser);

	if (isEnd){
		ser.EndGroup();
	}
}
void CCharacterDevelopmentSystem::OnLoadGame(ILoadGame* pLoadGame){}
void CCharacterDevelopmentSystem::OnLevelEnd(const char* nextLevel){}
void CCharacterDevelopmentSystem::OnActionEvent(const SActionEvent& event){
	if (event.m_event == eAE_inGame)
	{
		IUIElement *pMessageBox = gEnv->pFlashUI->GetUIElement("MessageBox_hud");
		if (pMessageBox)
		{
			if (pMessageBox->IsVisible())
				pMessageBox->SetVisible(false);
		}
	}
}

//Эта функция отвечала за скорость пережвижения игрока от ловкости
//Пускай будет закомненчена)
/* из старой системы. Хз, к чему это тут было, но, может, пригодится.
float CCharacterDevelopmentSystem::GetSprintSpeed(float sprint)
{
CPlayerMovementController *pMoveController = (CPlayerMovementController*)g_pGame->GetIGameFramework()->GetClientActor()->GetMovementController();
if (!pMoveController)
return 0;
SMovementState state;
pMoveController->GetMovementState(state);
state.maxSpeed = 5;
return sprint + (PlayerParam->Stamina * 0.23 + PlayerParam->Agility * 0.46);
}
*/

/* Здесь определялся голод игрока
void CCharacterDevelopmentSystem::CheckHungry(bool water)
{
	timerStarted = true;
	float TimeDif = gEnv->pTimer->GetCurrTime() - initTime;
	if (TimeDif >= hungryTimeLimit)
	{
		if (!water)
		{
			IItem *item = g_pGame->GetIGameFramework()->GetClientActor()->GetCurrentItem();
			if (!item)
				return;
			IWeapon *wep = item->GetIWeapon();
			if (!wep)
				return;
			IFireMode *firemode = wep->GetFireMode(wep->GetCurrentFireMode());// Урон увеличится только для текущего фаер мода, а нужно для всех
			switch ((int)PlayerParam->HungryLevel)
			{
			case EHSatiety:
			{
				PlayerParam->StrenghtBonus = 5;
				timerStarted = false;
				initTime = gEnv->pTimer->GetCurrTime();
				UpdateAllParams();
				PlayerParam->HungryLevel = EHNormal;
				break;
			}
			case EHNormal:
			{
				PlayerParam->StrenghtBonus = 0;
				timerStarted = false;
				initTime = gEnv->pTimer->GetCurrTime();
				UpdateAllParams();
				PlayerParam->HungryLevel = EHHungry;
				break;
			}
			case EHHungry:
			{
				PlayerParam->StrenghtBonus = -5;
				timerStarted = false;
				initTime = gEnv->pTimer->GetCurrTime();
				UpdateAllParams();
				PlayerParam->HungryLevel = EHVeryHungry;
				break;
			}

			case EHVeryHungry:
			{
				PlayerParam->StrenghtBonus = -10;
				timerStarted = false;
				initTime = gEnv->pTimer->GetCurrTime();
				UpdateAllParams();
				Regeneration(-3, 10);
				break;
			}
			};//Switch			
		}

		if (water)
		{
		}
	}
} */

/* int CCharacterDevelopmentSystem::ChangeHungryStance(int type)
{
	switch (type)
	{
	case EFTLight:
	{
		//дописать проверки на то не является ли гг уже сыт что бы не увеличивать больше чем сыт
		resValue = 0.3;//здесь дописать получение из хмл или тп
		PlayerParam->HungryLevel += resValue;
		CheckHungry();
		UpdateAllParams(); //уточнить
		break;
	}
	case EFTNormal:
	{
		resValue = 0.6;
		PlayerParam->HungryLevel += resValue;
		CheckHungry();
		UpdateAllParams();
		break;
	}
	case EFTSatiety:
	{
		resValue = 1;
		PlayerParam->HungryLevel += resValue;
		CheckHungry();
		UpdateAllParams();
		break;
	}
	case EFTHPFlask:
	{
		PlayerParam->HealthBonus += HPFlaskBonus;
		UpdateAllParams();
		break;
	}
	case EFTStrenghtFlask:
	{
		PlayerParam->StrenghtBonus += strengthFlaskBonus;
		UpdateAllParams();
		break;
	}
	};
	return IsHungry();
} */