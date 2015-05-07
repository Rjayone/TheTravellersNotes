// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#pragma once

#include "platform.h"
#include "CryString.h"
#include "IAudioConnection.h"
#include "ACBTypes.h"

namespace AudioControls
{
	class IAudioSystemControl
	{
	public:
		IAudioSystemControl::IAudioSystemControl()
			: m_name("")
			, m_id(ACB_INVALID_ID)
			, m_type(AUDIO_IMPL_INVALID_TYPE)
			, m_bPlaceholder(false)
			, m_bLocalised(false)
		{
		}

		IAudioSystemControl::IAudioSystemControl(const string& name, CID id, TImplControlType type)
			: m_name(name)
			, m_id(id)
			, m_type(type)
			, m_bPlaceholder(false)
			, m_bLocalised(false)
		{
		}

		virtual ~IAudioSystemControl() {}

		// unique id for this control
		CID GetId() const { return m_id; }
		void SetId(CID id) { m_id = id; }

		TImplControlType GetType() const { return m_type; }
		void SetType(TImplControlType type) { m_type = type; }

		// Virtual paths are used for grouping
		// control them within a file without/
		// having to mimic the structure in disk
		string GetVirtualPath() const { return m_path; }
		void SetVirtualPath(const string& path) 
		{
			if (path != m_path)
			{
				m_path = path;
			}
		}

		string GetName() const { return m_name; }
		void SetName(const string& name)
		{
			if (name != m_name)
			{
				m_name = name;
			}
		}

		bool IsPlaceholder() const { return m_bPlaceholder; }
		void SetPlaceholder(bool bIsPlaceholder) { m_bPlaceholder = bIsPlaceholder; }

		bool IsLocalised() const { return m_bLocalised; }
		void SetLocalised(bool bIsLocalised) { m_bLocalised = bIsLocalised; }

	private:
		CID m_id;
		TImplControlType m_type;
		string m_name;
		string m_path;
		bool m_bPlaceholder;
		bool m_bLocalised;

	public:
		// used for saving/loading
		std::string m_controlTag;
	};
}