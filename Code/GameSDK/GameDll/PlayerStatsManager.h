/****************************************

* File: PlayerStatsManager.h

* Description: Класс для работы с различными параметрами игрока

* Created by: Andrew Medvedev

* Date: 15.04.2014

* Diko Source File

*****************************************/
#ifndef _PLAYER_STATS_MANAGER_
#define _PLAYER_STATS_MANAGER_

#include "StdAfx.h"
#include "PlayerStats.h"

enum EPlayerStats
{
	//Player
	EPS_NULL = 0,
	EPS_Health,
	EPS_Mana,
	EPS_Strength,
	EPS_Agila,
	EPS_Eloquence,
	EPS_Stamina,

	//Resources
	EPS_Money,
	EPS_Wood,
	EPS_Stone,
	EPS_Iron,
};


class IPlayerStatsListener
{
public:
	virtual void OnStatChanged(CPlayerStat* stat) = 0;
	virtual void OnStatAdded(int type){}
	virtual void OnStatDeleted(int type){}
};

class CPlayerStatsManager
{
public:
	//Описание:
	//Конструктор по умолчанию
	CPlayerStatsManager();

	//Описание:
	//Функция добавляет стат в массив статов для дальнейшего управления
	//Принимает:
	//Указатель на стат, который нужно добавить в массив
	void AddStat(CPlayerStat* stat);

	//Описание:
	//Функция удаляет стат из массива по его типу
	//Принимает:
	//Тип стата из перечисления EPlayerStats
	//Возвращает:
	//true - успешное удаление, иначе - false
	bool DeleteStat(int type);

	//Описание:
	//Функция возвращает указатель на стат(CPlayerStat) по его типу из перечисления EPlayerStats
	CPlayerStat* GetStat(int type);

	//Описание:
	//Функция реализует добавление новых слушателей, которые буду в дальнейшем следить за изменением статов
	//Принимает:
	//Класс, которые поддерживает интерфейс IPlayerStatsListener и который будет слушать данного менеджера
	void AddListener(IPlayerStatsListener *listener);

	//Описание:
	//Удаление слушателя
	bool DeleteListener(IPlayerStatsListener *listener);

	//Описание:
	//Данная функция вызывается самим статов, сообщая о том, что он изменился
	//Данная функция, затем, оповещает слушателей об изменениях
	void OnStatChanged(CPlayerStat* stat);

	//Функции на получение статуса рюкзака (вне радиуса/в поле радиуса)
	void SetBackpackStatus(bool isBackpackLost);
	bool GetBackpackStatus();
private:
	std::vector<CPlayerStat*> m_pPlayerStats;	
	std::vector<IPlayerStatsListener*> m_pListeners;
	bool m_bBackpackLost = false;// статус рюкзака(вне радиуса/в поле радиуса)
};

#endif