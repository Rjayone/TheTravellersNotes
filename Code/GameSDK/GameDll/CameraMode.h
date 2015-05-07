/****************************************
* File: CameraMode.h
* Description: Описание различных положений камеры
* Created by: Andrew Medvedev
* Date: 31.12.2013
* Diko Source File
*****************************************/

#ifndef _CAMERA_MODE_
#define _CAMERA_MODE_

#include "IActionMapManager.h"

enum ECameraRollType
{
	ERollNULL,
	ERollIn,
	ERollOut
};

class CCameraMode : public IActionListener, public IGameFrameworkListener
{
public:
	CCameraMode();
	~CCameraMode();

	void OnAction(const ActionId& action, int activationMode, float value);

	void OnPostUpdate(float fDeltaTime);
	void OnSaveGame(ISaveGame* pSaveGame);
	void OnLoadGame(ILoadGame* pLoadGame);
	void OnLevelEnd(const char* nextLevel){}
	void OnActionEvent(const SActionEvent& event){}

	//Implement camera zoom
	void SetCameraZoom(float value, ECameraRollType type, bool add = true);

	//Camera X/Y offset
	void SetCameraXOffset(float value, bool add = true);
	void SetCameraYOffset(float value, bool add = true);

private:
	float m_CurrentDist;
	float m_CurrentXOffset;
	float m_CurrentYOffset;

	//Остаток зумирования
	float m_DistToZoom;
	int m_nRollType;
};

#endif