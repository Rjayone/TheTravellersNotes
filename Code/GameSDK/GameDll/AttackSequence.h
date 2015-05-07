#pragma once
#include "Game.h"

//� ������ ������������ ������� ��������� ���������  ����� ��� ���� ����.
//�����, � ������ ������ ��������� ������ ������������������� ����, ������� ������� �� �������������� ����
//��� �������� ������������������ ���� ���������� ������������ ����� CMeleeAttackSequence
enum EMeleeAttackActions
{
	e_MeleeAttackSequence_Horizontal_Up	,
	e_MeleeAttackSequence_Horizontal_Down,
	e_MeleeAttackSequence_Vertical_Left	,
	e_MeleeAttackSequence_Vertical_Right,
};


//������ ���������� ����� �� �����
typedef std::queue<int> TMeleeAttackSequence;


//������ ����� ��������� ����������� ������������ ������������������ ����
//������ ������������������ ������������ ����� ������� ����
//����� ������ ����� ���������� ����� �������(�.�. ����� ������ ������� �����������)
class CMeleeAttackSequence
{
public:
	CMeleeAttackSequence();

	//��������:
	//������� ��������� �������� � ������� ����� �����
	//���������: action - ����� �� ������������ EMeleeAttackActions
	void AddMeleeAttackAction(int action);

	//��������:
	//������� ������� ����� �� ������� �� �������
	//���������: index - ������ ��������� ����� �� �������
	void RemoveMeleeAttackAction(int index);


	//��������:
	//������� ���������� ������� ����� �� ������������������
	//������ �������������� ����� ������������ �������� ����� 
	//�������� ������������ ��� ������ �������� � �������� ����������� �����
	//����������: ��� �����
	int  GetMeleeAttackAction();

	//��������:
	//������ ������� ���������� ���������� ������� ����� ���������� �����
	//������ ���������� ����� �������� �����
	void UpdateSequence();

	//��������:
	//������� ���������� true, ���� ������������������ �� ���������, ����� false
	bool IsSequenceEmpty();

private:
	TMeleeAttackSequence m_sequence; //������� �������������������
};


class CMeleeAttackSequenceManager
{
public:

};