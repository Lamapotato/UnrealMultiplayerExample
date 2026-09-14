// Copyright Sam Bonifacio. All Rights Reserved.

#include "SettingKeyDisplayUtils.h"

#include "Utility/SettingRegistryKeyUtils.h"

namespace SB::AutoSettings::Editor::SettingKeyDisplay
{
	FPartialSettingKey GetCombinedKey(const USettingType* SettingType, const FPartialSettingKey& SubKey)
	{
		FPartialSettingKey CombinedKey;

		if (SettingType)
		{
			CombinedKey = SettingType->GetPartialSettingKey();
		}

		CombinedKey += SubKey;
		return CombinedKey;
	}

	FText GetPreviewText(const USettingType* SettingType, const FPartialSettingKey& SubKey)
	{
		const FPartialSettingKey CombinedKey = GetCombinedKey(SettingType, SubKey);
		if (!CombinedKey.IsValid())
		{
			return FText::FromString(TEXT("None"));
		}

		if (!SettingType)
		{
			return FText::FromString(CombinedKey.ToString());
		}

		FPartialSettingKey RegistryPreview = SB::AutoSettings::SettingRegistries::GetPreviewBaseKey(SettingType->SettingDomain);
		RegistryPreview += GetCombinedKey(SettingType, SubKey);
		return RegistryPreview.IsValid() ? FText::FromString(RegistryPreview.ToString()) : FText::FromString(CombinedKey.ToString());
	}
} // namespace SB::AutoSettings::Editor::SettingKeyDisplay
