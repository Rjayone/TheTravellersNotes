// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#pragma once

#include "CryString.h"
#include "ACBTypes.h"

namespace AudioControls
{
	class IAudioSystemControl;
	class IAudioConnection
	{
	public:
		IAudioConnection()
		: m_pControl(nullptr) { }
		IAudioConnection(IAudioSystemControl* pControl)
			: m_pControl(pControl)
		{ }

		virtual ~IAudioConnection() {}

		IAudioSystemControl* GetControl() const {return m_pControl;}
		void SetControl(IAudioSystemControl* pControl) {m_pControl =  pControl;}

		const string& GetGroup() const {return m_sGroup;}
		void SetGroup(const string& group) {m_sGroup =  group;}

	private:
		IAudioSystemControl* m_pControl;
		string m_sGroup;
	};
}