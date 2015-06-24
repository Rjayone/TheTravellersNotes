/****************************************************************************************************
* Firemode: Holding
* Description: Фаер мод для релизации возможности удерживания меча при замахе
* Created by: Andrew Medvedev
* Date: 29.09.2014
* Diko Source File
****************************************************************************************************/

#pragma once

#include "Game.h"
#include "MouseDirection.h"
#include "MouseUtils.h"
#include "FireMode.h"

enum EHoldingStatus
{
	e_HoldingTypeIdle,
	e_HoldingStatusSwing,
	e_HoldingStatusHold,
	e_HoldingStatusRelease,
	e_HoldingStatusCancel,
	e_HoldingStatusShortClick
};

//================================
class CHolding : public IActionListener
{
public:
	CHolding();
	~CHolding();

	//Описание:
	//Метод инициализирует итем
	bool Init(CItem *pItem);

	//Описание:
	//Покадровое обновление
	void Update();
	void ProcessEvent(SEntityEvent& event);


	//Описание:
	//Перегруженный метод из IActionListener для определения экшенов
	void OnAction(const ActionId& action, int activationMode, float value);
	IEntity* GetEntity();

	//Группа методов реализующая задержку перед ударом
	//
	//Описание:
	//Метод проигрывает анимацию взмаха оружием
	void PerfomSwing(CItem *pItem, int stance);	//Если во время взмаха была отпущена кнопка то проигрывается рандмо анима

	//Описание:
	//Метод запускает луп-анимацию удерживания меча с учетом стойки
	void OnHold(int stance);

	//Описание:
	//Метод проигрывает анимацию удара и определяет момент, когда идет расчет урона
	void PerfomRelease();

	//Описание:
	//Метод вызывается, когда игрок не хочет производить удар - нажимает кнопку отмены
	void CancelHolding();

	//~


	//Описание:
	//Метод инициализирует момент начала взмаха оружием
	void StartFire(CItem *pItem);
	void StopFire(CItem *pItem);

	//Описание:
	//Атака для шорт клика. Должна быть рандомной для текущей стойки
	void DefaultAttack(int stance = 0);

	//Описание:
	//Группа из сеттера и геттера установки/получения значения статуса удержания меча
	//см. EHoldingStatus
	void SetStatus(EHoldingStatus status);
	int GetStatus(){ return m_nStatus; }
	//~

	//Описание:
	//Возвращает время удержания оружия перед спуском
	float GetHoldingTime(){ return m_pMouseUtils ? m_pMouseUtils->CalculateClickTime() : 0.f; }

	//Описание:
	//Возвращает длительность фрагмента в милисеках
	float GetFragmentPlayDuration(IActionPtr pAction);
	
	//Описание:
	//Функция устанавливает теги в маникене
	void SetTag(uint32 id, bool set);

	//Описание:
	//Функция отчищает теги в дефолтное состояние
	void ClearAllHoldingTags();
private:
	int  m_nStatus;
	bool m_bShortAttack;
	bool m_bHolding;

	CMouseUtils* m_pMouseUtils;
	CItem *m_pItem;
};