/****************************************
* File: NPCControl.h
* Description: ”правление нпс, возможность дать приказ
* Created by: Andrew Medvedev
* Date: 08.12.2013
* Diko Source File
*****************************************/

#ifndef _NPCCONTROL_
#define _NPCCONTROL_

#include "Game.h"

class CNPCControl : public IGameFrameworkListener, public IActionListener
{
public:
	CNPCControl();
	void OnPostUpdate(float fDeltaTime);
	void OnSaveGame(ISaveGame* pSaveGame){};
	void OnLevelEnd(const char* nextLevel){};
	void OnActionEvent(const SActionEvent& event){};
	void OnLoadGame(ILoadGame* pLoadGame){};

	void OnAction(const ActionId& action, int activationMode, float value);

	IEntity* GetEntity();
	Vec3 GetMovePoint();
	void Execute();
private:
	EntityId CurrentNPC;
};

#endif