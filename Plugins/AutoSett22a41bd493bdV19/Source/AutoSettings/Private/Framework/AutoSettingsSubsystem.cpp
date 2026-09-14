// Copyright Sam Bonifacio. All Rights Reserved.

#include "Framework/AutoSettingsSubsystem.h"

#include "AutoSettingsConsoleGameInstanceSubsystem.h"
#include "AutoSettingsError.h"
#include "AutoSettingsLogs.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Framework/SettingType.h"
#include "Framework/SettingRegistry.h"
#include "Logging/StructuredLog.h"
#include "Misc/AutoSettingsConfig.h"
#include "Misc/AutoSettingsGameUserSettings.h"
#include "Subsystems/SubsystemCollection.h"
#include "Utility/SettingRegistryKeyUtils.h"
#include "Utility/SettingUtils.h"

UAutoSettingsSubsystem::UAutoSettingsSubsystem()
{
	GameSettingRegistry = CreateDefaultSubobject<USettingRegistry>(TEXT("GameSettingRegistry"), true);
	GameSettingRegistry->BaseKey = SB::AutoSettings::SettingRegistries::GetBaseKey(ESettingDomain::Global);
}

void UAutoSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UAutoSettingsConsoleGameInstanceSubsystem* ConsoleHandlerSubsystem =
		Collection.InitializeDependency<UAutoSettingsConsoleGameInstanceSubsystem>();
	check(ConsoleHandlerSubsystem);

	// Activate handlers when the global registry reports that its initial target capture is complete.
	GameSettingRegistry->Register_OnTargetsCaptured(FSimpleDelegate::CreateUObject(
		ConsoleHandlerSubsystem, &UAutoSettingsConsoleGameInstanceSubsystem::ActivateHandlers));

	// Configure setting assets only after startup composition is complete because this callback may execute immediately.
	UAssetManager::CallOrRegister_OnCompletedInitialScan(FSimpleMulticastDelegate::FDelegate::CreateLambda([this]()
		{
			// Cache the owned global settings as part of asset discovery so initialization can run without
			// repeating the asset scan.
			GameSettingRegistry->SetSettingAssets(FSettingUtils::GetAllSettingTypes(ESettingDomain::Global));
			TryInitializeSettingRegistry();
		}));

	SB::AutoSettings::Errors::ValidateSettingsProperty(
		"Engine - General Settings",
		GET_MEMBER_NAME_STRING_CHECKED(UEngine, GameUserSettingsClassName),
		GEngine->GameUserSettings->GetClass(),
		UAutoSettingsGameUserSettings::StaticClass());
}

void UAutoSettingsSubsystem::TryInitializeSettingRegistry()
{
	if (UAutoSettingsConfig::GetRegistryInitializationMode(ESettingDomain::Global) != EAutoSettingsRegistryInitializationMode::Automatic)
	{
		// Manual mode leaves initialization timing entirely in the caller's hands.
		return;
	}

	if (GameSettingRegistry->IsInitialized() || !GameSettingRegistry->HasSettingAssets())
	{
		// Automatic startup can only run after asset discovery has configured which settings belong to
		// this registry.
		return;
	}

	UE_LOGFMT(LogAutoSettings, Log, "{0}: Initializing global registry", GetNameSafe(GetGameInstance()));
	GameSettingRegistry->Initialize();
}

void UAutoSettingsSubsystem::Deinitialize()
{
	if (GameSettingRegistry->IsInitialized())
	{
		UE_LOGFMT(LogAutoSettings, Log, "{0}: Deinitializing global registry", GetNameSafe(GetGameInstance()));
		GameSettingRegistry->Deinitialize();
	}

	Super::Deinitialize();
}
