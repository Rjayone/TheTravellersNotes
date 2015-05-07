/****************************************

* File: InventoryItems.h

* Description: Файл содержит информацию связанную с инвентарными предметами:
  1) Описание типов предметов для инвентаря(EInventoryItemType)
  2) Описание размеров ячейки для отображения во флеше
  3) Структура данных включающая все поля собранных с классов Basic******* и является промежуточной стркутурой связи флеша и кода
  И т.д.

* Created by: Andrew Medvedev

* Date: 08.02.2014

* Diko Source File

*****************************************/

#ifndef _INVENTORY_ITEMS_
#define _INVENTORY_ITEMS_

#include "Game.h"
#include <StdAfx.h>

#ifndef _RECIPE_MAX_REG_COUNT_
//Отвечает за максимальное количество реагентов каждого рецепта
//То есть рецепт может иметь от 1 до 6 видов реагентов для крафта
#define _RECIPE_MAX_REG_COUNT_ 6
#endif

enum EInventoryItemType
{
	EIT_NULL = 0,  //Начальное значение
	EIT_Weapon,    //Оружие, надо бы на подмножества разбить. BasicWeapon
	EIT_Armor,     //Броня, на подмножество. BasicArmor
	EIT_Food,      //BasicFood
	EIT_Shield,    //BasicShield
	EIT_Flask,     //BasicFlask
	EIT_Ammo,      //BasicAmmo
	EIT_Recipe,    //BasicRecipe
	EIT_Other,     //BasicObject(Loot)
	EIT_Special,   //Компонент для улучшения оружия(?)
	EIT_Artefact,  //Артефакт или прочие супервозможности(?)
	EIT_Money,     //BasicMoney
	EIT_QuestItem, //BasicQuestItem
	EIT_Cloth      //BasicCloth
};

enum EInventoryItemSize
{
	EIIS_1x1 = 1,
	EIIS_2x2,
	EIIS_2x1,
	EIIS_1x2,
	EIIS_1x3,
	EIIS_3x1,
	EIIS_4x1
};

struct SInventoryItem
{
	string name; // Имя предмета
	string description;	// Описание предмета
	EntityId itemId; //ид итема. нужно при дропе

	int size; // Размер предмета.
	int cost; // Цена
	int type; // Тип
	int wearout; // Поношенность
	static int globalCount;	 //общее количество итемов

	//Entity params
	string objModel; // Путь к модели
	string entityClassType; // Имя класса предмета

	bool hiden; // Возможноть скрывать в инве и реализовать фильтры

	SInventoryItem() : cost(0), itemId(0), type(EIT_NULL), size(EIIS_1x1), hiden(false), wearout(0)
	{
		name = "";
		description = "";
		objModel = "";
		entityClassType = "";
	}
};


//Craft type defenition 
enum ECraftType
{
	eWeapon = 0,     //Оружейный рецепт
	eArmor,			 //Рецепт брони
	eCloth,			 //Рецепт одежды
	eAmmo,           //Рецепт аммуниции
	eAccesuar,       //Рецепт аксессуаров
	eJuviler         //Ювилирный рецепт
};

struct SCraftList
{
	string name; // Имя рецепта
	string description; // Описание рецепта

	string needs[_RECIPE_MAX_REG_COUNT_]; // Имена итемов которые нужны в крафте
	string condition[_RECIPE_MAX_REG_COUNT_]; // Условия крафта, например "Fire", "Water", "Smith"

	int type; // см. ECrfatType
	static int count;

	SCraftList() : name(""), description("")
	{
		for (int i = 0; i < _RECIPE_MAX_REG_COUNT_; i++)
		{
			needs[i] = "";
			condition[i] = "";
		}
	}

	int GetItemsCount()
	{
		int count = 0;
		for (int i = 0; i < _RECIPE_MAX_REG_COUNT_; i++)
		if (strcmp("", needs[i]))
			count++;
		return count;
	}
};

#endif