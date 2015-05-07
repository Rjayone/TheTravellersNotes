// CryEngine Header File.
// Copyright (C), Crytek, 1999-2014.

#pragma once

namespace AudioControls
{
	enum EACBControlType
	{
		eACBT_TRIGGER = 1,
		eACBT_RTPC,
		eACBT_SWITCH,
		eACBT_ENVIRONMENTS,
		eACBT_PRELOADS,
		eACBT_NUM_TYPES
	};

	typedef unsigned int TImplControlType;
	static const TImplControlType AUDIO_IMPL_INVALID_TYPE = 0;

	typedef unsigned int CID;
	static const CID ACB_INVALID_ID = 0;
}