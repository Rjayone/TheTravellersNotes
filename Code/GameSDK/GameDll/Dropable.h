/****************************************************************************************************************************
* File: Dropable.h

* Description: ���������(��������) ������������ ��������� �������� ������� �����

* Created by: Andrew Medvedev

* Date: 21.06.15

* Edda Source File

*****************************************************************************************************************************/

#pragma once

typedef unsigned int EntityId;
struct SInventoryItem;

class IDropable {
public:
	virtual void OnDrop(SInventoryItem* pItem, EntityId id) = 0;
};