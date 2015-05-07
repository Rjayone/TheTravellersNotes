// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#pragma once

#include <QWidget>
#include "ACBTypes.h"

namespace AudioControls
{
	class IAudioConnection;

	class IAudioConnectionInspectorPanel : public QWidget
	{
	public:
		virtual ~IAudioConnectionInspectorPanel() {}
		virtual void UpdatePanel(IAudioConnection* pConnection, EACBControlType eATLControlType = eACBT_NUM_TYPES) = 0;
	};
}