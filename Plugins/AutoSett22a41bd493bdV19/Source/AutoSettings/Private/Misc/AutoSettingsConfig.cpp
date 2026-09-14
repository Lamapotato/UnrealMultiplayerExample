// Copyright Sam Bonifacio. All Rights Reserved.

#include "Misc/AutoSettingsConfig.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

#define LOCTEXT_NAMESPACE "FAutoSettingsModule"

UAutoSettingsConfig::UAutoSettingsConfig()
{
	CategoryName = "Plugins";
}

EAutoSettingsRegistryInitializationMode UAutoSettingsConfig::GetRegistryInitializationMode(const ESettingDomain SettingDomain)
{
	// Read the config once through the class default object so runtime callers do not need to keep
	// their own cached copy of the developer settings object.
	const UAutoSettingsConfig* Config = GetDefault<UAutoSettingsConfig>();
	return SettingDomain == ESettingDomain::Player
		? Config->PlayerRegistryInitializationMode
		: Config->GlobalRegistryInitializationMode;
}

void UAutoSettingsConfig::PostInitProperties()
{
	Super::PostInitProperties();
}

bool UAutoSettingsConfig::IsConfigNameValid(const FString& ConfigName)
{
	// Case-insensitive check for "user" in config name
	return ConfigName.ToLower().Contains(TEXT("user"));
}

#if WITH_EDITOR
FText UAutoSettingsConfig::GetSectionText() const
{
	return LOCTEXT("AutoSettingsName", "Auto Settings");
}

FText UAutoSettingsConfig::GetSectionDescription() const
{
	return LOCTEXT("AutoSettingsDescription", "Configure the Auto Settings plugin.");
}
#endif

#undef LOCTEXT_NAMESPACE
