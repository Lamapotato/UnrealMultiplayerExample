// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/SettingKey.h"
#include "Framework/SettingType.h"

namespace SB::AutoSettings::Editor::SettingKeyDisplay
{
	FPartialSettingKey GetCombinedKey(const USettingType* SettingType, const FPartialSettingKey& SubKey);
	FText GetPreviewText(const USettingType* SettingType, const FPartialSettingKey& SubKey);
} // namespace SB::AutoSettings::Editor::SettingKeyDisplay
