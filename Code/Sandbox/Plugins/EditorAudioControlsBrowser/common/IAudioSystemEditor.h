// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#pragma once
#include "ACBTypes.h"
#include <IXml.h>
#include <QIcon>

namespace AudioControls
{
	class IAudioSystemControl;
	class IAudioConnection;
	class IAudioConnectionInspectorPanel;

	typedef uint TImplControlTypeMask;

	class IAudioSystemEditor
	{
	public:
		IAudioSystemEditor() {}
		virtual ~IAudioSystemEditor() {}

		// Controls
		virtual IAudioSystemControl* CreateControl(const string& name, TImplControlType type) = 0;
		virtual int ControlCount() const  = 0;
		virtual IAudioSystemControl* GetControlByID(CID id) const  = 0;
		virtual IAudioSystemControl* GetControlByIndex(unsigned int index) const = 0;
		virtual EACBControlType ImplTypeToATLType(TImplControlType type) const = 0;
		virtual TImplControlTypeMask GetCompatibleTypes(EACBControlType eATLControlType) const = 0;

		// Connections
		virtual IAudioConnection* CreateConnectionToControl(IAudioSystemControl* pControl) = 0;
		virtual IAudioConnection* CreateConnectionFromXMLNode(XmlNodeRef pNode) = 0;
		virtual void DestroyConnection(IAudioConnection* pConnection) = 0;
		virtual void WriteConnectionToXMLNode(XmlNodeRef pNode, const IAudioConnection* pConnection, const EACBControlType eATLControlType) = 0;
		
		// UI
		virtual IAudioConnectionInspectorPanel* NewConnectionInspectorPanel() const = 0;
		virtual QIcon GetTypeIcon(TImplControlType type) const = 0;
	};
}