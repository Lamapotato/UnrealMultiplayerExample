// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "UserSettings/EnhancedInputUserSettings.h"
#include "Misc/EngineVersionComparison.h"

namespace SB::AutoSettings::Input
{
	// Version-compatible wrapper for Enhanced Input API changes
	inline UEnhancedPlayerMappableKeyProfile* GetActiveKeyProfile(UEnhancedInputUserSettings* UserSettings)
	{
#if UE_VERSION_OLDER_THAN(5, 6, 0)
		return UserSettings->GetCurrentKeyProfile();
#else
		return UserSettings->GetActiveKeyProfile();
#endif
	}
} // namespace SB::AutoSettings::Input