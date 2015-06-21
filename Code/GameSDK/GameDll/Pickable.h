/****************************************************************************************************************************
* File: Pickable.h

* Description: ���������(��������) ������������ ��������� �������� ������� �������������

* Created by: Andrew Medvedev

* Date: 21.06.15

* Edda Source File

*****************************************************************************************************************************/

#pragma once

typedef unsigned int EntityId;

class IPickable {
public:
	virtual void OnPickUp(EntityId id) = 0;
};