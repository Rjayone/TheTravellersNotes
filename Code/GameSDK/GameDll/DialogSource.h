#ifndef _DIALOG_SOURCE_
#define _DIALOG_SOURCE_

#include "IGameFramework.h"

class CDialogAbility : public IGameFrameworkListener, public IActionListener
{
public:
	CDialogAbility();

	void OnPostUpdate(float fDeltaTime);
	void OnSaveGame(ISaveGame* pSaveGame){}
	void OnLoadGame(ILoadGame* pLoadGame){}
	void OnLevelEnd(const char* nextLevel){}
	void OnActionEvent(const SActionEvent& event){}

	virtual void OnAction(const ActionId& action, int activationMode, float value);

	IEntity* GetEntity();
private:
	bool bStartDialog;
};

#endif