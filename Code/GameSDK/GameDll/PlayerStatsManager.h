/****************************************

* File: PlayerStatsManager.h

* Description: ����� ��� ������ � ���������� ����������� ������

* Created by: Andrew Medvedev

* Date: 15.04.2014

* Diko Source File

*****************************************/
#ifndef _PLAYER_STATS_MANAGER_
#define _PLAYER_STATS_MANAGER_

#include "StdAfx.h"
#include "PlayerStats.h"

enum EPlayerStats
{
	//Player
	EPS_NULL = 0,
	EPS_Health,
	EPS_Mana,
	EPS_Strength,
	EPS_Agila,
	EPS_Eloquence,
	EPS_Stamina,

	//Resources
	EPS_Money,
	EPS_Wood,
	EPS_Stone,
	EPS_Iron,
};


class IPlayerStatsListener
{
public:
	virtual void OnStatChanged(CPlayerStat* stat) = 0;
	virtual void OnStatAdded(int type){}
	virtual void OnStatDeleted(int type){}
};

class CPlayerStatsManager
{
public:
	//��������:
	//����������� �� ���������
	CPlayerStatsManager();

	//��������:
	//������� ��������� ���� � ������ ������ ��� ����������� ����������
	//���������:
	//��������� �� ����, ������� ����� �������� � ������
	void AddStat(CPlayerStat* stat);

	//��������:
	//������� ������� ���� �� ������� �� ��� ����
	//���������:
	//��� ����� �� ������������ EPlayerStats
	//����������:
	//true - �������� ��������, ����� - false
	bool DeleteStat(int type);

	//��������:
	//������� ���������� ��������� �� ����(CPlayerStat) �� ��� ���� �� ������������ EPlayerStats
	CPlayerStat* GetStat(int type);

	//��������:
	//������� ��������� ���������� ����� ����������, ������� ���� � ���������� ������� �� ���������� ������
	//���������:
	//�����, ������� ������������ ��������� IPlayerStatsListener � ������� ����� ������� ������� ���������
	void AddListener(IPlayerStatsListener *listener);

	//��������:
	//�������� ���������
	bool DeleteListener(IPlayerStatsListener *listener);

	//��������:
	//������ ������� ���������� ����� ������, ������� � ���, ��� �� ���������
	//������ �������, �����, ��������� ���������� �� ����������
	void OnStatChanged(CPlayerStat* stat);

	//������� �� ��������� ������� ������� (��� �������/� ���� �������)
	void SetBackpackStatus(bool isBackpackLost);
	bool GetBackpackStatus();
private:
	std::vector<CPlayerStat*> m_pPlayerStats;	
	std::vector<IPlayerStatsListener*> m_pListeners;
	bool m_bBackpackLost = false;// ������ �������(��� �������/� ���� �������)
};

#endif