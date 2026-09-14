// Copyright Sam Bonifacio. All Rights Reserved.

#include "Misc/ConfigSettingPersistenceStrategy.h"

#include "AutoSettingsLogs.h"
#include "Misc/AutoSettingsConfig.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "Framework/SettingKey.h"
#include "Framework/SettingTypes.h"
#include "Logging/StructuredLog.h"

namespace SB::AutoSettings::Config
{
	TMap<FString, FString> IniPaths = {};
}

UConfigSettingPersistenceStrategy::UConfigSettingPersistenceStrategy()
{
	auto Settings = GetDefault<UAutoSettingsConfig>();
	IniName = Settings->SettingsIniName;
	SectionName = Settings->SettingsSectionName;

	// Validate config name
	if (!UAutoSettingsConfig::IsConfigNameValid(IniName))
	{
		UE_LOGFMT(LogAutoSettings, Warning,
			"ConfigSettingPersistenceStrategy: Config file '{0}' may not be writable. "
			"Config names should contain 'user' (case-insensitive) OR the config must have bCanSaveAllSections=true. "
			"Settings may not persist.",
			IniName);
	}
}

void UConfigSettingPersistenceStrategy::OnSave_Implementation(const FSetSettingEvent& Event)
{
	SetConfigValue(Event.Setting.Key.ToFullName(), Event.Data);
}

FSettingData UConfigSettingPersistenceStrategy::GetSavedValue_Implementation(const FSettingReference& Setting) const
{
	return GetConfigValue(Setting.Key.ToFullName());
}

TArray<FFullSettingKey> UConfigSettingPersistenceStrategy::FindKeys_Implementation() const
{
	const FConfigSection* Section = GetSection();
	if (!Section)
	{
		// Section not found, likely the config file doesn't exist yet
		return {};
	}

	// Find all keys in the category
	TArray<FName> KeysInSection;
	Section->GetKeys(KeysInSection);

	TArray<FFullSettingKey> Result;
	for (FName& Key : KeysInSection)
	{
		FFullSettingKey ConfigKey = FFullSettingKey(Key.ToString());
		Result.Add(ConfigKey);
	}
	return Result;
}

FString UConfigSettingPersistenceStrategy::GetSectionName() const
{
	return SectionName;
}

FString UConfigSettingPersistenceStrategy::GetIniPath() const
{
	if (!ensure(!IniName.IsEmpty()))
	{
		// Invalid
		return {};
	}

	if (!SB::AutoSettings::Config::IniPaths.Contains(IniName))
	{
		// Load the ini
		FString LoadedIniPath;
		FConfigCacheIni::LoadGlobalIniFile(LoadedIniPath, *IniName);
		if (FPaths::FileExists(LoadedIniPath))
		{
			/* Existing generated user config files must be loaded directly so saved settings are
			 * available before writes flush new values.
			 */
			GConfig->LoadFile(LoadedIniPath);
		}
		SB::AutoSettings::Config::IniPaths.Add(IniName, LoadedIniPath);
	}

	return SB::AutoSettings::Config::IniPaths[IniName];
}

void UConfigSettingPersistenceStrategy::SetConfigValue(FName Key, const FSettingData& Value)
{
	if (!Key.IsNone() && !Value.Value.IsEmpty())
	{
		const FString ExportedValue = Value.ExportString();

		GConfig->SetString(*GetSectionName(), *Key.ToString(), *ExportedValue, GetIniPath());
		GConfig->Flush(false, GetIniPath());
	}
}

FSettingData UConfigSettingPersistenceStrategy::GetConfigValue(FName Key) const
{
	if (GetSection())
	{
		if (const FConfigValue* ConfigValue = GetSection()->Find(Key))
		{
			const FString FullValue = ConfigValue->GetValue();
			return FSettingData::ImportString(FullValue);
		}
	}

	return FSettingData();
}

const FConfigSection* UConfigSettingPersistenceStrategy::GetSection() const
{
	return GConfig->GetSection(*GetSectionName(), false, GetIniPath());
}
