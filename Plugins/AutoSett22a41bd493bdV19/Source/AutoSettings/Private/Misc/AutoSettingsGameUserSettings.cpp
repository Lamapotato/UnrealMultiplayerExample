// Copyright Sam Bonifacio. All Rights Reserved.

#include "Misc/AutoSettingsGameUserSettings.h"

void UAutoSettingsGameUserSettings::ApplyNonResolutionSettings()
{
	/**
	 * Intentionally do nothing here.
	 * The base class would normally apply settings here, which would reset settings to incorrect values when F11 is used to change fullscreen mode.
	 * We leave this empty to prevent this issue.
	 */
}