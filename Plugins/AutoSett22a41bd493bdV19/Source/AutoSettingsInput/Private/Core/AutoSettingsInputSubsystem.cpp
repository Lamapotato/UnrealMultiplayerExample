// Copyright Sam Bonifacio. All Rights Reserved.

#include "Core/AutoSettingsInputSubsystem.h"

#include "Core/AutoSettingsEnhancedInputUserSettings.h"
#include "AutoSettingsError.h"
#include "EnhancedInputDeveloperSettings.h"
#include "EnhancedInputSubsystems.h"
#include "Subsystems/SubsystemCollection.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "Utility/AutoSettingsInputDataCache.h"

void UAutoSettingsInputSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Create internal mapping cache for performance optimization
	MappingCache = NewObject<UAutoSettingsInputDataCache>(this);

	const UEnhancedInputDeveloperSettings* EnhancedInputSettings = GetDefault<UEnhancedInputDeveloperSettings>();
	const FString SettingsCategoryName = SB::AutoSettings::Errors::GetDisplayName(UEnhancedInputDeveloperSettings::StaticClass());

	// Validate Enhanced Input user settings are enabled
	// This ensures player input remapping functionality is available
	SB::AutoSettings::Errors::ValidateSettingsProperty<bool>(
		SettingsCategoryName,
		GET_MEMBER_NAME_STRING_CHECKED(UEnhancedInputDeveloperSettings, bEnableUserSettings),
		static_cast<bool>(EnhancedInputSettings->bEnableUserSettings),
		true);

	// Validate that Enhanced Input is configured to use AutoSettings' user settings class
	// This ensures AutoSettings can detect input mapping changes and store directional intent metadata
	SB::AutoSettings::Errors::ValidateSettingsProperty(
		SettingsCategoryName,
		GET_MEMBER_NAME_STRING_CHECKED(UEnhancedInputDeveloperSettings, UserSettingsClass),
		EnhancedInputSettings->UserSettingsClass.LoadSynchronous(),
		UAutoSettingsEnhancedInputUserSettings::StaticClass());
}

void UAutoSettingsInputSubsystem::Deinitialize()
{
	// Cleanup mapping cache
	MappingCache = nullptr;

	Super::Deinitialize();
}