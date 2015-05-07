/*********************************************************************************************************
- File: Camera.cpp
- Description: ���������� ����� ��� ������ �������
- Created  by: ����� ����
- Modified by:
- Date:		   25.03.2014
- Update:      11.10.14
- Edda Studio
- Description:
  ������ ����� �������� �� ����������� �� ����� �����, ��������������� ��� �������������.
********************************************************************************************************/

#ifndef _BuildCamera_
#define _BuildCamera_

#include "StdAfx.h"
#include "IGameFramework.h"
#include "IEntitySystem.h"
#include "IViewSystem.h"

class CBuildCamera
{
public:
	CBuildCamera();
	~CBuildCamera();


	//��������:
	//������ ����� ���������� �� ����� ������ ��������� � ���� ���������� BuildCamera#
	//���� ����� ���������� ����, �� ���������������� ������ � ��������� � ������.
	void CreateBuildCamera();

	//��������:
	//������� ���������� ������ � ����. ����� ��� ���� ������, ��, �������, ��������.
	void CreateViewParams(unsigned int CameraId, EntityId GameCameraId, IEntity *GameCamera);
	
	SViewParams m_CurViewParams;
	IView *m_pBuildCameraView;
	uint8 m_BuildCameraViewID;
	bool m_BuildCameraExist = false;

	//������ ��������� ����� ��� ���������
	std::vector<IEntity*> m_pGameCameraEntity;
};

#endif 