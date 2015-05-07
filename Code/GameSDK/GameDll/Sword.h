/****************************************************************************************************
* Weapon: Sword
* Description: Класс для определения поведения меча
  Для реализации возможности задержать атаку(удерживание) был использован класс CHolding,
  Который представляет собой механизм взмах-спуск-удар
  Во время атаки урон рассчитывается покадрово: в функции Update() вызывается PerformIntersection()
  Для определения попадания используется цилиндр, для его отображения использовать команду td_draw 1
  После успешного пересечения идет спаун партикла и задается имульс.

* Created by: Andrew Medvedev
* Date: 29.09.2014
* Edda Studio
****************************************************************************************************/

#pragma once
#include "Weapon.h"
#include "MouseDirection.h"
#include "Melee.h"
#include "Holding.h"
#include "CombatStance.h"
#include "WeaponProperties.h"


class CSword : public CWeapon
{
public:
	CSword();
	void Release();

	bool Init(IGameObject * pGameObject);
	void Update(SEntityUpdateContext& ctx, int slot);

	//Описание:
	//Данная функция вызывается при выборе и убирании оружия
	//Если достается оружие до - true, иначе false
	void Select(bool select);

	//Описание:
	//Функция, которая прячет оружие если гг бездействует
	void AutoDeselect();
	
	//Описание:
	//Данная функция вызывается в момент старта огня или удара
	//В данной реализации данный метод используется для определения того момента, когда нужно сделать взмаха мечем
	//После, если кнопка удерживается, то меч входит в режим удержания атаки
	void StartFire();

	//Описание:
	//Данная функция вызывается одной из 2 других функций, которые определяют это удар на левую или правую кнопку мыши
	//В зависимости от кнопки функция получает позицию, с которой будет вестись атака(вертикальная/горизонтальная)
	void StartFire(int position);

	//Описание:
	//Функция вызывается в момент отпускания кнопки мыши
	//В данной реализации используется как начало атаки мечем
	void StopFire();


	//Описание:
	//Данный метод обрабатывает евенты сущности - меча
	//Используется для определения евента таймера
	void ProcessEvent(SEntityEvent& event);

	//Описание:
	//Данный метод является главным в определении атаки
	//В процессе атаки меча создается примитив по которому идет определение пересечения объектов(столкновение полигонов 2 моделей)
	//После идет рассчет урона и импульса. Проигрывается эффект удара. 
	//Затем данные о точки контакта, направлении и сущностях передаются в Hit()
	void PerfomIntersection();


	//Описание:
	//Данная функция составляет объект, на основе аргументов, для передачи движку информации о произведенной атаке
	//pt  - точка пересечения
	//dir - направление удара
	//normal - нормаль удара
	//pCollider - физический интерфейс сущности, которой назначается урон/импульс
	//collidedEntityId - id сущности выше
	//pratId - (?) возможно используется для определния кости, по которой прошел урон
	//ipart (?)
	//surfaceIdx - индекс поверхности по которой идет урон(землая, трава, дерево и тд)
	void Hit(const Vec3 &pt, const Vec3 &dir, const Vec3 &normal, IPhysicalEntity *pCollider, EntityId collidedEntityId, int partId, int ipart, int surfaceIdx);
	
	//Описание:
	//Функция определяет это дружественный удар или нет
	//Возвращает: true - если цель союзник, иначе false
	bool IsFriendlyHit(IEntity* pShooter, IEntity* pTarget);

	//Описание
	//Функция проигрывает эффект удара в точке удара
	//Принимает: position - точка удара, noramal - нормаль, surfaceIdx - индекс поверхности
	void PlayHitMaterialEffect(const Vec3 &position, const Vec3 &normal, int surfaceIdx);

	bool IsBusy() const { return m_bBusy; }
	void SetBusy(bool busy){ m_bBusy = busy; }

	//Вырезано
	DEPRICATED int GetComboMoveCount(int combo);

	//Описание:
	//Функция возвращает указатель на механизм, дающий возможность удерживать меч
	CHolding* GetHoldingStatus(){ return m_Holding; }

	//Описание:
	//Функция возвращает указатель на класс, в котором идет описание стойки игрока
	CCombatStance* GetCombatStance(){ return m_CombatStance; }
	int GetWeaponType(){ return e_WeaponTypeSword; }

	//Описание:
	//Функция задает параметры отображения хелпера
	void DebugDraw();

private:
	//Описание:
	//Функция отчищает теги, которые задаюится в маникен, для фильтрации анимаций
	void ClearTag(uint tag);
	void ClearAllTags();

	//Пока не используется
	DEPRICATED CMouseDirection m_MouseDir;

	CHolding* m_Holding;
	CCombatStance* m_CombatStance;

	//Данный примитив используется для определения пересечения сущностей
	primitives::cylinder* cyl;

	Vec3 m_prevPt; // Для рассчета направления движения меча
	
	bool m_bNowAttack;
	bool m_bBusy;			//Weapon busy
	bool m_bHitSoundPlayed; //Отвечает, проигран ли звук 

	int m_selectedCombo;	//One of combo move set. ECombo
	int m_comboMoveCount;   //Count of actions in current combo
	int m_performedMoveCount;	//performed move count of current combo
};