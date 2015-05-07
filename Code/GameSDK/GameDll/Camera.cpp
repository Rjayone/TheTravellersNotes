/*********************************************************************************************************
- File: Camera.cpp
- Description: ���������� ����� ��� ������ �������
- Created by: ����� ����
- Date: 25.03.2014
- Diko Source File
- ToDo:�������� ���������� ���-�� ����� � ��������� ��������������� �������� �����
********************************************************************************************************/

#include "StdAfx.h"
#include "Camera.h"
#include "IGameFramework.h"
#include "IEntitySystem.h"
#include "Globals.h"

//������������ ���������� ����� ��� ��������� ������������
//����������� �������� ��� �� ������
//����� ������������ ���� ����� ���� � ����� 100,
//�� � ������ ����� �������� ���� 100
#define MAX_CAM_ITERATIONS 100

CBuildCamera::CBuildCamera() :
m_pBuildCameraView(0),
m_BuildCameraViewID(0),
m_BuildCameraExist(false)
{
}

CBuildCamera::~CBuildCamera()
{
}

void CBuildCamera::CreateBuildCamera()
{
#ifdef __TDEBUG__
	CryLogAlways("[BuildSystem]: Start searching for buildings cameras");
#endif
	IViewSystem *pViewSystem = gEnv->pGame->GetIGameFramework()->GetIViewSystem();
	IEntitySystem *pEntitySystem = gEnv->pSystem->GetIEntitySystem();
	m_pGameCameraEntity.clear();
	//�������� ����� �����
	for (int i = 0; i < MAX_CAM_ITERATIONS; i++)
	{	
		char* index = new char;
		itoa(i, index, 10);
		string number = index;
		IEntity* pCam = pEntitySystem->FindEntityByName("BuildCamera" + number);
		if (pCam != NULL && pViewSystem != NULL)
		{
#ifdef __TDEBUG__
			CryLogAlways("[BuildSystem]: BuildCamera%d was added", i);
#endif
			//��������� ��������� ������ � ������
			m_pGameCameraEntity.push_back(pCam);
			pCam->Activate(true);

			//Create a new View and assign it to a pointer named m_pIView
			m_pBuildCameraView = pViewSystem->CreateView();

			//Get the View ID of our new camera and assign it to a variable named RTSCameraViewID
			m_BuildCameraViewID = pViewSystem->GetViewId(m_pBuildCameraView);

			//Remove the currently active View
			pViewSystem->RemoveView(pViewSystem->GetActiveViewId());

			//Set our new view as the active one
			pViewSystem->SetActiveView(m_pBuildCameraView);

			//Give Initial values to our camera's view params
			EntityId id = pCam->GetId();
			CreateViewParams(m_BuildCameraViewID, id, pCam);
			m_pBuildCameraView->SetCurrentParams(m_CurViewParams);

			//Link Our new view to the game camera entity in our world
			m_pBuildCameraView->LinkTo(pCam);
			m_BuildCameraExist = true;
		}
	}
}


void CBuildCamera::CreateViewParams(unsigned int CameraId, EntityId GameCameraId, IEntity *GameCamera)
{
	m_CurViewParams.position = GameCamera->GetWorldPos();
	m_CurViewParams.rotation = GameCamera->GetWorldRotation();
	m_CurViewParams.nearplane = 0.0f;
	m_CurViewParams.viewID = CameraId;
	m_CurViewParams.groundOnly = false;
	m_CurViewParams.shakingRatio = 0.0f;
	m_CurViewParams.currentShakeQuat = IDENTITY;
	m_CurViewParams.currentShakeShift = ZERO;
	m_CurViewParams.idTarget = GameCameraId;
	m_CurViewParams.targetPos = GameCamera->GetWorldPos();
	m_CurViewParams.frameTime = 60.0f;
	m_CurViewParams.angleVel = 0.0f;
	m_CurViewParams.vel = 0.0f;
	m_CurViewParams.dist = 0.0f;
	m_CurViewParams.blend = true;
}