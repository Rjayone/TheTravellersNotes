// ќпределение класса дл€ оружи€ ипользующие
// тетиву д€л выстрела


#ifndef _NOCK_H_
#define _NOCK_H_

#include "Game.h"
#include "MouseUtils.h"

class CNock
{
public:
	CNock();
	virtual ~CNock();

	void Nock();
	void StartFire();
	void StopFire();
private:
	//SFireModeParams	*m_pShared;
	CMouseUtils* m_pMouseUtils;

	float m_fScalePerSecond; // —кал€р урона
	float m_fMinDamage; //ћинимальный урон до которого может опуститьс€ при усталости
	float m_fMaxDamage; //ћаксимальный урон при нат€гивании 
	float m_fMaxHoldTime; //¬рем€ через которое уроне перестанет наростать и пойдет на спад.	
public:
	friend class CSingle;
};

#endif // _NOCK_H_
