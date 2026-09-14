// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/SettingKey.h"
#include "Framework/SettingType.h"

namespace SB::AutoSettings::SettingRegistries
{
	AUTOSETTINGS_API FPartialSettingKey GetBaseKey(ESettingDomain Domain);
	AUTOSETTINGS_API FPartialSettingKey GetBaseKey(ESettingDomain Domain, FName PlayerId);
	AUTOSETTINGS_API FPartialSettingKey GetPreviewBaseKey(ESettingDomain Domain);
} // namespace SB::AutoSettings::SettingRegistries
