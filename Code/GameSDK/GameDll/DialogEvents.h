/****************************************************************************************************
* Interface: IDialgEvent
* Description: Интерфейс определящий те методы, которые будут использовать в процессе отсылки евентов подписанным классам
* Created by: Andrew Medvedev
* Date: 15.07.15
* Edda Studio
****************************************************************************************************/

#pragma once
#include "StdAfx.h"

//Для регистрации получения данных евентов выполните следующие действия
//0. Унаследуйте своим классом интерфейс IDialogEvents и реализуйте чистые виртуальные методы
//1. Получиь указатель на CUIDialogMenu  g_pGame->GetDialogSystem(); 
//2. Проведите подписку  m_pDialogSystem->AddEventListener(this);
//3. В деструкторе обзательно проведите удаление слушателя m_pDialogSystem->RemoveEventListener(this);
class IDialogEvents 
{
public:
	//Описание:
	//Евент срабатывает в момент старта диалога
	//Аргумент - ид нпс, с которым ведем диалог
	virtual void OnDialogStarted(EntityId targetNPCId) = 0;

	//Описание:
	//Евент срабатывает в момент окончания диалога
	//Аргумент - ид нпс, с которым завершен диалог
	virtual void OnDialogFinished(EntityId targetNPCId) = 0;

	//Описание:
	//Евент срабатывает когда игрок выбирает один из вариантов овтетов
	//Аргументы:
	//messageToAnswer - Текст ответа игрока
	//selectedAnswerId - ид выбранного ответа
	//nextNPCAnswer - ид фразы нпс, которую он скажет в ответ на выбранный ответ игрока
	virtual void OnPlayerSelectAnswer(string messageToAnswer, int selectedAnswerId, int nextNPCAnswer){}

	//Описание:
	//Евент срабатывает, когда нпс произноит фразу
	virtual void OnNPCAnswer(string message){}
};