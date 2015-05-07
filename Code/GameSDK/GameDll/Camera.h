/*********************************************************************************************************
- File: Camera.cpp
- Description: Реализация камер для режима стройки
- Created  by: Гомза Яков
- Modified by:
- Date:		   25.03.2014
- Update:      11.10.14
- Edda Studio
- Description:
  Данный класс отвечает за определение на сцене камер, предназначенных для строительства.
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


	//Описание:
	//Данный метод проходится по всему списку сущностей и ищет совпадения BuildCamera#
	//Если такое совпадение есть, то инициализируется камера и заносится в список.
	void CreateBuildCamera();

	//Описание:
	//Задание параметров камеры и вида. Таких как угол обзора, ид, позиция, вращение.
	void CreateViewParams(unsigned int CameraId, EntityId GameCameraId, IEntity *GameCamera);
	
	SViewParams m_CurViewParams;
	IView *m_pBuildCameraView;
	uint8 m_BuildCameraViewID;
	bool m_BuildCameraExist = false;

	//Список созданных камер под постройку
	std::vector<IEntity*> m_pGameCameraEntity;
};

#endif 