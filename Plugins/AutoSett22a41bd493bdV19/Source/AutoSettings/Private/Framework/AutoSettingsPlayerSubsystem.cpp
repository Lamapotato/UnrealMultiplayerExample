// Copyright Sam Bonifacio. All Rights Reserved.

#include "Framework/AutoSettingsPlayerSubsystem.h"

#include "AutoSettingsLogs.h"
#include "Misc/AutoSettingsConfig.h"
#include "Misc/AutoSettingsPlayer.h"
#include "Engine/AssetManager.h"
#include "Framework/SettingType.h"
#include "Framework/SettingRegistry.h"
#include "Logging/StructuredLog.h"
#include "Utility/SettingRegistryKeyUtils.h"
#include "Utility/SettingUtils.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

ULocalPlayer* UAutoSettingsPlayerSubsystem::GetOwningLocalPlayerFromRegistry(const USettingRegistry* Registry)
{
	const UAutoSettingsPlayerSubsystem* Subsystem = Registry ? Registry->GetTypedOuter<UAutoSettingsPlayerSubsystem>() : nullptr;
	return Subsystem ? Subsystem->GetLocalPlayer() : nullptr;
}

APlayerController* UAutoSettingsPlayerSubsystem::GetOwningPlayerControllerFromRegistry(const USettingRegistry* Registry)
{
	const ULocalPlayer* LocalPlayer = GetOwningLocalPlayerFromRegistry(Registry);
	return LocalPlayer ? LocalPlayer->GetPlayerController(nullptr) : nullptr;
}

APlayerController* UAutoSettingsPlayerSubsystem::GetOwningPlayerControllerFromSetting(const FSettingReference& Setting)
{
	return GetOwningPlayerControllerFromRegistry(Setting.Context.Registry);
}

UAutoSettingsPlayerSubsystem::UAutoSettingsPlayerSubsystem()
{
	PlayerSettingRegistry = CreateDefaultSubobject<USettingRegistry>(TEXT("PlayerSettingRegistry"), true);
}

void UAutoSettingsPlayerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Keep the player namespace aligned with controller ownership so direct registry Initialize()
	// calls become valid as soon as the local player has a concrete identity.
	GetLocalPlayer()->OnPlayerControllerChanged().AddWeakLambda(this, [this](APlayerController* /*NewPlayerController*/)
		{
			UpdatePlayerRegistryBaseKey();
			TryInitializeSettingRegistry();
		});
	UpdatePlayerRegistryBaseKey();

	// Discover the owned player settings once, then let automatic startup or explicit callers choose
	// when the first initialization/apply pass should happen.
	UAssetManager::CallOrRegister_OnCompletedInitialScan(FSimpleMulticastDelegate::FDelegate::CreateLambda([this]()
		{
			// Asset discovery defines which setting assets belong to this registry. Controller ownership is a
			// separate concern handled by the base-key update path.
			UpdatePlayerRegistrySettingAssets();

			// Automatic player startup depends on both asset discovery and controller ownership.
			TryInitializeSettingRegistry();
		}));
}

void UAutoSettingsPlayerSubsystem::Deinitialize()
{
	if (PlayerSettingRegistry->IsInitialized())
	{
		PlayerSettingRegistry->Deinitialize();
	}

	Super::Deinitialize();
}

void UAutoSettingsPlayerSubsystem::UpdatePlayerRegistrySettingAssets()
{
	if (PlayerSettingRegistry->IsInitialized() || !UAssetManager::IsInitialized() || !UAssetManager::Get().HasInitialScanCompleted())
	{
		return;
	}

	// Configure the registry's owned setting set once discovery is complete so initialization no longer
	// needs to rediscover player-scoped settings.
	PlayerSettingRegistry->SetSettingAssets(FSettingUtils::GetAllSettingTypes(ESettingDomain::Player));
}

void UAutoSettingsPlayerSubsystem::UpdatePlayerRegistryBaseKey()
{
	if (PlayerSettingRegistry->IsInitialized())
	{
		return;
	}

	const APlayerController* PlayerController = GetLocalPlayer()->GetPlayerController(nullptr);
	if (!PlayerController)
	{
		return;
	}

	const FName PlayerId = IAutoSettingsPlayer::GetUniquePlayerIdentifier(PlayerController);
	if (!ensure(!PlayerId.IsNone()))
	{
		return;
	}

	// Derive the concrete player namespace before initialization so player-scoped keys resolve inside
	// the owning local player's namespace.
	PlayerSettingRegistry->BaseKey = SB::AutoSettings::SettingRegistries::GetBaseKey(ESettingDomain::Player, PlayerId);
}

void UAutoSettingsPlayerSubsystem::TryInitializeSettingRegistry()
{
	if (UAutoSettingsConfig::GetRegistryInitializationMode(ESettingDomain::Player) != EAutoSettingsRegistryInitializationMode::Automatic)
	{
		// Manual mode leaves initialization timing entirely in the caller's hands.
		return;
	}

	if (PlayerSettingRegistry->IsInitialized())
	{
		// The registry performs its registration and initial apply pass only once.
		return;
	}

	if (!PlayerSettingRegistry->HasSettingAssets())
	{
		// Automatic startup waits until asset discovery has configured which player-scoped settings this
		// registry owns.
		return;
	}

	const APlayerController* PlayerController = GetLocalPlayer()->GetPlayerController(nullptr);
	if (!PlayerController)
	{
		// Automatic startup waits until controller ownership can provide a concrete player namespace.
		return;
	}

	const FName PlayerId = IAutoSettingsPlayer::GetUniquePlayerIdentifier(PlayerController);
	if (PlayerId.IsNone())
	{
		// Automatic startup waits until the owning controller resolves to a stable player identifier.
		return;
	}

	UE_LOGFMT(LogAutoSettings, Log, "{0}: Initializing player registry", GetNameSafe(GetLocalPlayer()));
	PlayerSettingRegistry->Initialize();
}
