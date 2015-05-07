// ����������� ������ ��� ������ �����������
// ������ ��� ��������


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

	float m_fScalePerSecond; // ������ �����
	float m_fMinDamage; //����������� ���� �� �������� ����� ���������� ��� ���������
	float m_fMaxDamage; //������������ ���� ��� ����������� 
	float m_fMaxHoldTime; //����� ����� ������� ����� ���������� ��������� � ������ �� ����.	
public:
	friend class CSingle;
};

#endif // _NOCK_H_
