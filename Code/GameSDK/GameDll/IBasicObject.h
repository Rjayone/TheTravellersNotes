/****************************************************************************************************************************
* File: BasicObject.h

* Description: ������� ������� ����������� ������. ������ � ����� �������� ���� ���� ����������� ������������ ��� � �������� ������

* Created by: Andrew Medvedev

* Date: 22.08.2013

* -Update: 29.03.14

* Diko Source File

*****************************************************************************************************************************/

#ifndef _IBASIC_OBJECT_
#define _IBASIC_OBJECT_

#include "IGameObject.h"

//Pick-Drop-Use protocols
#include "Pickable.h"
#include "Dropable.h"
#include "Usable.h"

//SInventoryItem declaration
struct SInventoryItem;

class IBasicObject : public CGameObjectExtensionHelper< IBasicObject, IGameObjectExtension >,
	public IDropable,
	public IPickable,
	public IUsable
{
public:
	//��������:
	//����� ����������� ��� ������ � ������������ ���������� ��������
	virtual void Reset() = 0;

	//��������:
	//����� ���������� ������ SmartScriptTable, ������� � ���������� ����������� ��� ��������� ���������� �� ������� ��������,
	//��� ������� ���� ����������.
	virtual SmartScriptTable GetSmartScriptTable() = 0;

	//�������:
	//����� ���������� ��� ��������. ��� �������� ����������.
	//��� �� ������������ � ���������
	virtual string GetObjectName() = 0;

	//��������:
	//����� ���������� �������� ��������, ������� ������������ � ���������
	virtual string GetObjetDescription() = 0;

	//��������:
	//����� ���������� ���� � ������(�������) ��������
	virtual string GetModelPath() = 0;

	//��������:
	//����� ���������� ��� ��������.
	//� ��������� ��� ��� ���������� ��������� ��������� ��������� ��������
	virtual string GetEntityType() = 0;

	//��������:
	//����� ���������� ��������� �� ��������� SInventoryItem ����� ��������� XML-����� ������ ��������.
	//� XML-����� ��������� ������� ��������� ��������, ������� � ������������� ��� �������.
	virtual SInventoryItem* GetItemParamsXML() = 0;

protected:
	string m_ObjectName;
	string m_ObjectDescr;
	string m_ModelPath;
	string m_EntityType;

	int m_nSize;
	int m_nCost;

	EntityId m_ItemId;
};

#endif