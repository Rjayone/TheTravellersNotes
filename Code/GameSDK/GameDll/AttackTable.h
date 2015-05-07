/****************************************************************************************************
* Attack Table
* Description: Класс определяет тип атаки по некоторым параметрам: угол тип оружия и тд
* Created by: Andrew Medvedev
* Date: 29.09.2014
* Diko Source File
****************************************************************************************************/
#ifndef __ATTACK_TABLE__
#define __ATTACK_TABLE__

#include "Sword.h"
#include "PlayerAnimation.h"

enum EAttackTable
{
	EAT_Default,
	EAT_HitRightHorizontal,
	EAT_HitLeftHorizontal,
	EAT_HitRightDiagonal,
	EAT_HitLeftDiagonal,
	EAT_HitUpVertical,
	EAT_HitDownVertical,
	EAT_HitFrontPricking
};

enum EAttackPosition
{
	e_AttackPositionHorizontal,
	e_AttackPositionVertical,
	e_AttackPositionPrick
};


class CMeleeAttackTable
{
public:

	//Функция возвращает тип аттаки в зависимости от угла
	int GetAttackType(int weapon, float angle)
	{
		switch (weapon)
		{
		case 1/*Sword*/:
		{
			if (angle == 90)
			{
			  CryLogAlways("Default attack");
			  return EAT_Default;
			}
			if (angle >= 0 && angle <= 45)
			{
			  return EAT_HitRightHorizontal;
			}
			if (angle > 45 && angle <= 135)
			{
			  return EAT_HitUpVertical;
			}
			if (angle > 135 && angle <= 180)
			{
			  return EAT_HitLeftHorizontal;
			}
			if (angle > 180 && angle <= 225)
			{
			  return EAT_HitLeftDiagonal;
			}
			if (angle > 225 && angle <= 315)
			{
			  return EAT_HitDownVertical;
			}
			if (angle > 315 && angle <= 360)
			{
			  return  EAT_HitRightDiagonal;
			}

		}
		case 2/*Axe*/:
		{
			break;
		}
		};
		return EAT_Default;
	}


	//Функция возвращает следующее действие в комбо ударе
	int GetCurrentComboAction(int weaponType, int selectedCombo, int perfomedMoveCount)
	{
		//switch (selectedCombo)
		//{
		//case e_Left_Right_Horizontal_Top:{
		//									 if (perfomedMoveCount == 0) return EAT_HitLeftHorizontal;
		//									 if (perfomedMoveCount == 1) return EAT_HitRightHorizontal;
		//									 if (perfomedMoveCount == 2) return EAT_HitDownVertical;
		//									 break;
		//}
		//case e_Left_Right_Horizontal_Bottom:{
		//										if (perfomedMoveCount == 0) return EAT_HitLeftHorizontal;
		//										if (perfomedMoveCount == 1) return EAT_HitRightHorizontal;
		//										if (perfomedMoveCount == 2) return EAT_HitUpVertical;
		//										break;
		//}
		//case e_Front_Left_Horizontal:{
		//								 if (perfomedMoveCount == 0) return EAT_HitFrontPricking;
		//								 if (perfomedMoveCount == 1) return EAT_HitLeftHorizontal;
		//								 break;
		//}
		//}
		return 0;
	}

	//Функция возвращает рандомную анимацию в заданной стойке
	//Доделать реализацию зависимости стойки и оружия
	int GetStanceRandomAttackAction(int weaponType = 0, int stance = 0)
	{
		return Random(1, 6);
	}

	//Переделать на установку тегов
	FragmentIDAutoInit GetAttackFragmentID(CItem* item, int attackType)
	{
		switch (attackType)
		{
		case EAT_HitDownVertical:
			return item->GetFragmentIds().hit;
		case EAT_HitFrontPricking:
			return item->GetFragmentIds().hit;
		case EAT_HitLeftDiagonal:
			return item->GetFragmentIds().hit;
		case EAT_HitLeftHorizontal:
			return item->GetFragmentIds().hit;
		case EAT_HitRightDiagonal:
			return item->GetFragmentIds().hit;
		case EAT_HitRightHorizontal:
			return item->GetFragmentIds().hit;
		case EAT_HitUpVertical:
			return item->GetFragmentIds().hit;
		};
	}

	//Описание:
	//Функция возвращает рандомный тэг в зависимости от текущей стойки
	uint16 GetRandomTagDirection(CItem *pItem)
	{
		IActionController *pActionController = pItem ? pItem->GetActionController() : NULL;
		if (!pActionController)
			return 0;

		SAnimationContext &animContext = pActionController->GetContext();
		if (animContext.state.IsSet(PlayerMannequin.tagIDs.horizontal))//Если выбрана горизонтальная стойка
		{
			int rand = Random(0, 2);
			if (rand == 0)
				return PlayerMannequin.tagIDs.leftSide;
			if (rand == 1)
				return PlayerMannequin.tagIDs.rightSide;
		}
		if (animContext.state.IsSet(PlayerMannequin.tagIDs.vertical))//Если выбрана вертикальная стойка
		{
			int rand = Random(0, 2);
			if (rand == 0)
				return PlayerMannequin.tagIDs.up;
			if (rand == 1)
				return PlayerMannequin.tagIDs.down;
		}
		return PlayerMannequin.tagIDs.pricking;
	}


};



#endif