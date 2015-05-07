#pragma once
#include "Game.h"

//В данном перечислении следует указывать возможные  удары для мили атак.
//После, в каждом оружии создается массив последовательностей атак, который состоит из индивидуальных атак
//Для создания последовательности атак необходимо использовать класс CMeleeAttackSequence
enum EMeleeAttackActions
{
	e_MeleeAttackSequence_Horizontal_Up	,
	e_MeleeAttackSequence_Horizontal_Down,
	e_MeleeAttackSequence_Vertical_Left	,
	e_MeleeAttackSequence_Vertical_Right,
};


//Делаем сокращение имени не более
typedef std::queue<int> TMeleeAttackSequence;


//Данный класс реализует возможность обрабатывать последовательности атак
//Каждая последовательность представляет собой очередь атак
//После каждой атаки происходит сдвиг очереди(т.е. самый первый элемент выгружается)
class CMeleeAttackSequence
{
public:
	CMeleeAttackSequence();

	//Описание:
	//Функция позволяет добавить в очередь новую атаку
	//Принимает: action - атака из перечисления EMeleeAttackActions
	void AddMeleeAttackAction(int action);

	//Описание:
	//Функция удаляет атаку из очереди по индексу
	//Принимает: index - индекс удаляемой атаки из очереди
	void RemoveMeleeAttackAction(int index);


	//Описание:
	//Функция возвращает текущую атаку из последовательности
	//Должна использоваться перед определением анимации удара 
	//Является определяющей для выбора анимации и рассчета показателей удара
	//Возврашает: тип атаки
	int  GetMeleeAttackAction();

	//Описание:
	//Данная функция производит обновление очереди после совершения атаки
	//Должна вызываться после успешной атаки
	void UpdateSequence();

	//Описание:
	//Функция возвращает true, если последовательность не закончена, иначе false
	bool IsSequenceEmpty();

private:
	TMeleeAttackSequence m_sequence; //очередь последовательностей
};


class CMeleeAttackSequenceManager
{
public:

};