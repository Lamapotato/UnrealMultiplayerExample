// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "Framework/SettingType.h"
#include "AutoSettingsConfig.generated.h"

/** Controls whether a registry initializes itself automatically or waits for an explicit request. */
UENUM(BlueprintType)
enum class EAutoSettingsRegistryInitializationMode : uint8
{
	/** Initialize as soon as the subsystem prerequisites are ready. */
	Automatic,
	/** Wait until game code or Blueprint explicitly requests initialization. */
	Manual
};

// Plugin configuration page in Project Settings
UCLASS(config = Game, defaultconfig)
class AUTOSETTINGS_API UAutoSettingsConfig : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UAutoSettingsConfig();

	/** Returns the configured initialization mode for the requested registry domain. */
	static EAutoSettingsRegistryInitializationMode GetRegistryInitializationMode(ESettingDomain SettingDomain);

	// Config ini file (without extension) that settings are saved to.
	// Requires engine restart for changes to take effect.
	// Note: Must contain "user" (case-insensitive) for Unreal to allow saving, unless bCanSaveAllSections=true
	UPROPERTY(Config, EditAnywhere, Category = Settings)
	FString SettingsIniName = "UserSettings";

	// Config section that settings are saved to within the config ini file
	UPROPERTY(Config, EditAnywhere, Category = Settings)
	FString SettingsSectionName = "Settings";

	/** Controls when the global registry performs its first registration and apply pass. */
	UPROPERTY(Config, EditAnywhere, Category = Initialization)
	EAutoSettingsRegistryInitializationMode GlobalRegistryInitializationMode = EAutoSettingsRegistryInitializationMode::Automatic;

	/** Controls when each player registry performs its first registration and apply pass. */
	UPROPERTY(Config, EditAnywhere, Category = Initialization)
	EAutoSettingsRegistryInitializationMode PlayerRegistryInitializationMode = EAutoSettingsRegistryInitializationMode::Automatic;

	/**
	 * Check if the config name is valid for saving.
	 * Config files must contain "user" (case-insensitive) in the name OR
	 * have bCanSaveAllSections=true in the config hierarchy.
	 * @param ConfigName The config file name to validate
	 * @return True if the config name contains "user"
	 */
	static bool IsConfigNameValid(const FString& ConfigName);

protected:
	virtual void PostInitProperties() override;

#if WITH_EDITOR
	virtual FText GetSectionText() const override;
	virtual FText GetSectionDescription() const override;
#endif
};
