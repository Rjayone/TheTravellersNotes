/****************************************************************************************************
* Class: CDialogDispatcher
* Description: ����� ��� ����������� �������� � �������.
  ����� ����� ������������ �������: ����������� ������ ������
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

	//��������:
	//���������� ���������
	//����� ���������� �� DialogSystem ��� ����������� ����������� �������
	void Update();
private:

	//��������:
	//������� ����������, �������� �� ��������, �� ������� ������� ����� ��
	bool IsAI();

	//��������:
	//����� ���������� ��������� �� ��������, �� ������� ������� ����� ��� NULL
	IEntity* GetTarget();

	//IActionListener
	void OnAction(const ActionId& action, int activationMode, float value);
	//~

	bool m_bCanStartDialog;
	CUIDialogMenu* m_pDialogSystem;

};