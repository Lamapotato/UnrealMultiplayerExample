// Copyright Sam Bonifacio. All Rights Reserved.

#include "Utility/SettingUtils.h"

#include "Framework/SettingType.h"
#include "Utility/AutoSettingsAssetUtils.h"
#include "AutoSettingsLogs.h"
#include "HAL/IConsoleManager.h"
#include "Logging/StructuredLog.h"

static FAutoConsoleCommandWithWorld LogSettingsCommand(
	TEXT("AutoSettings.LogSettings"),
	TEXT("Logs all settings found"),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			TArray<USettingType*> Assets = FSettingUtils::GetAllSettingTypes();
			UE_LOGFMT(LogAutoSettings, Display, "Found {0} settings", Assets.Num());
			for (const USettingType* Asset : Assets)
			{
				UE_LOGFMT(LogAutoSettings, Display, "    {0}", Asset->GetName());
			}
		}),
	ECVF_Default);

TArray<USettingType*> FSettingUtils::GetAllSettingTypes()
{
	// Combined discovery exposes both content-defined setting classes and reflected C++ setting classes to registries.
	TArray<USettingType*> Settings = FAutoSettingsAssetUtils::LoadAndGetAllClassDefaults<USettingType>(
		USettingType::SettingPrimaryAssetTypeName);

	// Downstream registries should only see settings that are enabled and safe to use.
	Settings.RemoveAll([](const USettingType* Setting)
		{
			return !Setting || !Setting->bEnabled;
		});
	return Settings;
}

TArray<USettingType*> FSettingUtils::GetAllSettingTypes(const ESettingDomain SettingDomain)
{
	// Reuse the shared asset-loading path so both subsystem types resolve settings through the same
	// enabled-asset filter before narrowing to their requested domain.
	return GetAllSettingTypes().FilterByPredicate([SettingDomain](const USettingType* Setting)
		{
			return Setting && Setting->SettingDomain == SettingDomain;
		});
}
