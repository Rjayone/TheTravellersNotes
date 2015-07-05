/****************************************************************************************************
* Class: CDialogDispatcher
* Description: Класс для определения действий в диалоге.
  Класс может обрабатывать события: возможность начать диалог
* Created by: Andrew Medvedev
* Date: 23.06.15
* Edda Studio
****************************************************************************************************/

#pragma once
#include "GameActions.h"

class CUIDialogMenu;

class CDialogDispatcher : public IActionListener
{
public:
	CDialogDispatcher();
	~CDialogDispatcher();

	//Описание:
	//Покадровое обновлеие
	//Метод вызывается из DialogSystem для определения возможности диалога
	void Update();
private:

	//Описание:
	//Функция определяет, является ли сущность, на которую смотрит игрок АИ
	bool IsAI();

	//Описание:
	//Метод возвращает указатель на сущность, на которую смотрит игрок или NULL
	IEntity* GetTarget();

	//IActionListener
	void OnAction(const ActionId& action, int activationMode, float value);
	//~

	bool m_bCanStartDialog;
	CUIDialogMenu* m_pDialogSystem;

};