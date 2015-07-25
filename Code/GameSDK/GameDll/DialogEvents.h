/****************************************************************************************************
* Interface: IDialgEvent
* Description: ��������� ����������� �� ������, ������� ����� ������������ � �������� ������� ������� ����������� �������
* Created by: Andrew Medvedev
* Date: 15.07.15
* Edda Studio
****************************************************************************************************/

#pragma once
#include "StdAfx.h"

//��� ����������� ��������� ������ ������� ��������� ��������� ��������
//0. ����������� ����� ������� ��������� IDialogEvents � ���������� ������ ����������� ������
//1. ������� ��������� �� CUIDialogMenu  g_pGame->GetDialogSystem(); 
//2. ��������� ��������  m_pDialogSystem->AddEventListener(this);
//3. � ����������� ���������� ��������� �������� ��������� m_pDialogSystem->RemoveEventListener(this);
class IDialogEvents 
{
public:
	//��������:
	//����� ����������� � ������ ������ �������
	//�������� - �� ���, � ������� ����� ������
	virtual void OnDialogStarted(EntityId targetNPCId) = 0;

	//��������:
	//����� ����������� � ������ ��������� �������
	//�������� - �� ���, � ������� �������� ������
	virtual void OnDialogFinished(EntityId targetNPCId) = 0;

	//��������:
	//����� ����������� ����� ����� �������� ���� �� ��������� �������
	//���������:
	//messageToAnswer - ����� ������ ������
	//selectedAnswerId - �� ���������� ������
	//nextNPCAnswer - �� ����� ���, ������� �� ������ � ����� �� ��������� ����� ������
	virtual void OnPlayerSelectAnswer(string messageToAnswer, int selectedAnswerId, int nextNPCAnswer){}

	//��������:
	//����� �����������, ����� ��� ��������� �����
	virtual void OnNPCAnswer(string message){}
};