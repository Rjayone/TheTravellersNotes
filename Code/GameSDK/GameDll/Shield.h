/*********************************************
*
*
*			SHIELD.H
*								/	  /
*							   /	 /
*
*********************************************/
#ifndef _SHIELD_H_
#define _SHIELD_H_

#include "Item.h"


enum EPhysType
{
	Physicalize = 0,
	NonPhysicalize = 1
};


class CShield :public CItem
{
public:
	// simple phys enum


	CShield();
	virtual ~CShield();

	//Set physic
	virtual void SetPhysic(bool phys);
	virtual void Reset();

	//Read params from file
	//virtual void GetParams();

	//Chek if cur weapon is shield
	//virtual bool Selected();
	virtual void OnSelect(){}
	virtual void Select(bool select);

	//Attach to character
	//virtual void Attach();
	//virtual void Detach();

private:
	//Model Path. Get from file
	char *model;

	//Some funct;
	IEntity *pEntity;
	IInventory *pInventory;
	IGameObject *pGameObject;

	//Bool for ON/OFF phys collision
	bool CollisionON;
	bool ShieldSelected;
};

#endif //_SHIELD_H_