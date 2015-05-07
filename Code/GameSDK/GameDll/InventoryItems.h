/****************************************

* File: InventoryItems.h

* Description: ���� �������� ���������� ��������� � ������������ ����������:
  1) �������� ����� ��������� ��� ���������(EInventoryItemType)
  2) �������� �������� ������ ��� ����������� �� �����
  3) ��������� ������ ���������� ��� ���� ��������� � ������� Basic******* � �������� ������������� ���������� ����� ����� � ����
  � �.�.

* Created by: Andrew Medvedev

* Date: 08.02.2014

* Diko Source File

*****************************************/

#ifndef _INVENTORY_ITEMS_
#define _INVENTORY_ITEMS_

#include "Game.h"
#include <StdAfx.h>

#ifndef _RECIPE_MAX_REG_COUNT_
//�������� �� ������������ ���������� ��������� ������� �������
//�� ���� ������ ����� ����� �� 1 �� 6 ����� ��������� ��� ������
#define _RECIPE_MAX_REG_COUNT_ 6
#endif

enum EInventoryItemType
{
	EIT_NULL = 0,  //��������� ��������
	EIT_Weapon,    //������, ���� �� �� ������������ �������. BasicWeapon
	EIT_Armor,     //�����, �� ������������. BasicArmor
	EIT_Food,      //BasicFood
	EIT_Shield,    //BasicShield
	EIT_Flask,     //BasicFlask
	EIT_Ammo,      //BasicAmmo
	EIT_Recipe,    //BasicRecipe
	EIT_Other,     //BasicObject(Loot)
	EIT_Special,   //��������� ��� ��������� ������(?)
	EIT_Artefact,  //�������� ��� ������ ����������������(?)
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
	string name; // ��� ��������
	string description;	// �������� ��������
	EntityId itemId; //�� �����. ����� ��� �����

	int size; // ������ ��������.
	int cost; // ����
	int type; // ���
	int wearout; // ������������
	static int globalCount;	 //����� ���������� ������

	//Entity params
	string objModel; // ���� � ������
	string entityClassType; // ��� ������ ��������

	bool hiden; // ���������� �������� � ���� � ����������� �������

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
	eWeapon = 0,     //��������� ������
	eArmor,			 //������ �����
	eCloth,			 //������ ������
	eAmmo,           //������ ���������
	eAccesuar,       //������ �����������
	eJuviler         //��������� ������
};

struct SCraftList
{
	string name; // ��� �������
	string description; // �������� �������

	string needs[_RECIPE_MAX_REG_COUNT_]; // ����� ������ ������� ����� � ������
	string condition[_RECIPE_MAX_REG_COUNT_]; // ������� ������, �������� "Fire", "Water", "Smith"

	int type; // ��. ECrfatType
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