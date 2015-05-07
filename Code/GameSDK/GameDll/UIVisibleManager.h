//Отвечает за отображение флеш роликов

#pragma once
#include "Game.h"
#include "IFlashUI.h"

class CUIVisibleManager
{
	std::vector<IUIElement*> m_pVisibleUI;
public:
	void HideAllUIElements()
	{
		int count = gEnv->pFlashUI->GetUIElementCount();
		if (count <= 0) return;

		m_pVisibleUI.clear();
		for (int i = 0; i < count; i++)
		{
			IUIElement *pElement = gEnv->pFlashUI->GetUIElement(i);
			if (pElement)
			{
				if (pElement->IsVisible())
				{
					m_pVisibleUI.push_back(pElement);
					pElement->SetVisible(false);

					IRenderer* pRenderer = gEnv->pRenderer;
					CRY_ASSERT(pRenderer);
					pRenderer->EF_SetPostEffectParam("Dof_User_Active", 1.f, true);
					pRenderer->EF_SetPostEffectParam("Dof_User_FocusDistance", 10000, true);
					pRenderer->EF_SetPostEffectParam("Dof_User_FocusRange", 0, true);
					pRenderer->EF_SetPostEffectParam("Dof_User_BlurAmount", 10, true);
					pRenderer->EF_SetPostEffectParam("Dof_User_ScaleCoc", 1, true);

					if (ICVar* myVar = gEnv->pConsole->GetCVar("t_scale"))
						myVar->ForceSet("1");
				}
			}
		}
	}

	void ShowHidenUIElements(bool bHUDOnly = true)
	{
		for (int i = 0; i < m_pVisibleUI.size(); i++)
		{
			m_pVisibleUI[i]->SetVisible(true);
			const char* name = m_pVisibleUI[i]->GetName();

			if (bHUDOnly)
			{
				if (!strcmp(name, "CraftMenu") || !strcmp(name, "DialogMenu") || !strcmp(name, "BuildMenu") || !strcmp(name, "Inventory"))
				{
					m_pVisibleUI[i]->SetVisible(false);
				}
			}
		}
		IUIElement* dot = gEnv->pFlashUI->GetUIElement("Dot");
		if (dot) dot->SetVisible(true);
	}
};