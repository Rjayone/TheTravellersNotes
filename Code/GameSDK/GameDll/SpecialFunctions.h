/*********************************************************************************************************
- File: SpecialFunctions.h
- Description: ����������� �������
- Created by: ����� ����
- Date: 18.08.2014
- Edda Source File
********************************************************************************************************/
#pragma once

#include "IGameFramework.h"

class CSpecialFunctions
{
	public:
	CSpecialFunctions();
	~CSpecialFunctions();

	static Vec3 GetBonePos(const char *bname, bool local, IEntity *pEntity);
};