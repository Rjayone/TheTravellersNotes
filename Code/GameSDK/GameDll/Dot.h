#pragma once

#include "IFlashUI.h"

class CUIDot
{
public:
	//��������:
	//������������� ����� ������� ��� �������� Dot(����� � ������ ������)
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

	//��������:
	//
	void Reset()
	{
	
	}
};