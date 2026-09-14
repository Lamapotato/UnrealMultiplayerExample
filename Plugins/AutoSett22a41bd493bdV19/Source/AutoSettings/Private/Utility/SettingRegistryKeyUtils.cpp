// Copyright Sam Bonifacio. All Rights Reserved.

#include "Utility/SettingRegistryKeyUtils.h"

namespace SB::AutoSettings::SettingRegistries
{
	FPartialSettingKey GetBaseKey(ESettingDomain Domain)
	{
		FPartialSettingKey BaseKey;

		switch (Domain)
		{
			case ESettingDomain::Player:
				BaseKey += FSettingKeyFragment(TEXT("Player"));
				break;

			case ESettingDomain::Global:
			default:
				BaseKey += FSettingKeyFragment(TEXT("Global"));
				break;
		}

		return BaseKey;
	}

	FPartialSettingKey GetBaseKey(ESettingDomain Domain, FName PlayerId)
	{
		FPartialSettingKey BaseKey = GetBaseKey(Domain);
		if (Domain == ESettingDomain::Player && !PlayerId.IsNone())
		{
			BaseKey += FSettingKeyFragment(PlayerId);
		}

		return BaseKey;
	}

	FPartialSettingKey GetPreviewBaseKey(ESettingDomain Domain)
	{
		return Domain == ESettingDomain::Player ? GetBaseKey(Domain, TEXT("<PlayerId>")) : GetBaseKey(Domain);
	}
} // namespace SB::AutoSettings::SettingRegistries
