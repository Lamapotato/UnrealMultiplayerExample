// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/SettingType.h"

class USettingType;

/**
 * Static utility functions for working with setting types.
 */
class AUTOSETTINGS_API FSettingUtils
{
public:
	/** Returns every enabled SettingType asset that AutoSettings can discover. */
	static TArray<USettingType*> GetAllSettingTypes();

	/** Returns every enabled SettingType asset for the requested setting domain. */
	static TArray<USettingType*> GetAllSettingTypes(ESettingDomain SettingDomain);
};
