/*********************************************************************************************************
- File: BuildSelectionHelper.cpp
- Description: �������� ���������� ����������� ����� �������
- Created by: ����� ����
- Date: 25.03.2014
- Diko Source File
- ToDo:�������� ���������� ��� ���������
********************************************************************************************************/
#include "StdAfx.h"
#include "Globals.h"
#include "BuildSelectionHelper.h"

#ifdef _BuildCamera_
 CBuildCamera g_BuildCamera;
#endif
bool OnBuildMode = false;

CSelectionBuildHelper g_Selecion;

IEntity *pSpawnObj;
int iCost;
float Gold=30000;
bool onPreBuild=false;
std::vector<IEntity*> m_pGameCameraEntity;

int CamCount;