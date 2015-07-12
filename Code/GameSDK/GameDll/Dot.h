#pragma once

#include "IFlashUI.h"

class CUIDot
{
public:
	//Описание:
	//Устанавливает текст события для элемента Dot(точка в центре экрана)
	void ShowDotWithTitle(const char* text)
	{
		IUIElement* pDot = gEnv->pFlashUI->GetUIElement("Dot");
		if (pDot != NULL)
		{
			pDot->SetVisible(true);
			SUIArguments args;
			string sText = string(text);
			args.AddArgument(sText);
			pDot->CallFunction("PlayMessage", args);
		}
	}

	//Описание:
	//
	void Reset()
	{
	
	}
};