// Copyright Sam Bonifacio. All Rights Reserved.

#include "Utility/InputMappingUtils.h"

#include "Core/AutoSettingsEnhancedInputUserSettings.h"
#include "AutoSettingsError.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Processing/InputModifierAxisDirection.h"
#include "PlayerMappableKeySettings.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "GameFramework/PlayerController.h"
#include "Logging/StructuredLog.h"
#include "AutoSettingsInputLogs.h"
#include "Utility/EnhancedInputCompatibility.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "Utility/AutoSettingsInputDataCache.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

namespace SB::AutoSettings::Input
{
	// Helper function for Asset Registry searching

	// Helper function for asset cache access
	const TArray<FAssetData>& GetInputContextAssets(const UWorld* World)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(GetInputContextAssets);
		UAutoSettingsInputDataCache* Cache = UAutoSettingsInputDataCache::Get(World);

		// Check cache if available
		if (Cache)
		{
			const TArray<FAssetData>& CachedAssets = Cache->GetInputContextAssets();
			if (!CachedAssets.IsEmpty())
			{
				return CachedAssets;
			}
		}

		// Cache miss or no cache - perform computation
		static TArray<FAssetData> ComputedAssets;
		ComputedAssets.Empty();
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(SearchInputContextAssets);
			IAssetRegistry::Get()->GetAssetsByClass(
				UInputMappingContext::StaticClass()->GetClassPathName(),
				ComputedAssets);
		}

		// Store in cache if available
		if (Cache)
		{
			Cache->PutInputContextAssets(ComputedAssets);
			return Cache->GetInputContextAssets();
		}

		return ComputedAssets;
	}

} // namespace SB::AutoSettings::Input

SB::AutoSettings::Input::FEnhancedInputSubsystemData SB::AutoSettings::Input::GetEnhancedInputSubsystemData(const ULocalPlayer* LocalPlayer)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(GetEnhancedInputSubsystemData);

	FEnhancedInputSubsystemData Data;

	if (!LocalPlayer)
	{
		return Data;
	}

	{
		TRACE_CPUPROFILER_EVENT_SCOPE(GetSubsystem);
		Data.Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	}

	if (!Data.Subsystem)
	{
		return Data;
	}

	auto UserSettings = Data.Subsystem->GetUserSettings();

	// User Settings can be null if Enhanced Input hasn't initialized it yet
	if (!UserSettings)
	{
		return Data;
	}

	Data.UserSettings = Cast<UAutoSettingsEnhancedInputUserSettings>(UserSettings);
	Data.bIsValid = Data.UserSettings != nullptr;

	return Data;
}

/**
 * Discovers input mappings across all Input Mapping Context assets using cached Asset Registry results.
 * Works with inactive/unloaded contexts to support comprehensive input remapping.
 */
FEnhancedActionKeyMapping SB::AutoSettings::Input::PlayerMapping::GetMapping(FName PlayerMappingName, const UWorld* World)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(PlayerMapping_GetMapping);

	// Try to use cache when world context is available
	UAutoSettingsInputDataCache* Cache = UAutoSettingsInputDataCache::Get(World);
	if (Cache)
	{
		// Check cache for hit
		if (const FEnhancedActionKeyMapping* CachedMapping = Cache->GetMapping(PlayerMappingName))
		{
			return *CachedMapping;
		}
	}

	// Cache miss or no cache available - search directly
	const TArray<FAssetData>& InputContexts = GetInputContextAssets(World);
	FEnhancedActionKeyMapping FoundMapping;
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(SearchMappingInAssets);
		bool bFound = false;
		for (const FAssetData& ContextAsset : InputContexts)
		{
			const UInputMappingContext* LoadedContext = Cast<UInputMappingContext>(ContextAsset.GetAsset());

			for (const FEnhancedActionKeyMapping& Mapping : LoadedContext->GetMappings())
			{
				if (Mapping.GetMappingName() == PlayerMappingName)
				{
					FoundMapping = Mapping;
					bFound = true;
					break;
				}
			}
			if (bFound)
				break;
		}
	}

	// Store in cache if available
	if (Cache)
	{
		Cache->PutMapping(PlayerMappingName, FoundMapping);
	}

	return FoundMapping;
}

UInputMappingContext* SB::AutoSettings::Input::PlayerMapping::GetMappingContext(FName PlayerMappingName, const UWorld* World)
{
	const TArray<FAssetData>& InputContexts = GetInputContextAssets(World);

	for (const FAssetData& ContextAsset : InputContexts)
	{
		UInputMappingContext* LoadedContext = Cast<UInputMappingContext>(ContextAsset.GetAsset());
		for (const FEnhancedActionKeyMapping& Mapping : LoadedContext->GetMappings())
		{
			if (Mapping.GetMappingName().IsEqual(PlayerMappingName, ENameCase::IgnoreCase, false))
			{
				return LoadedContext;
			}
		}
	}
	return {};
}

bool SB::AutoSettings::Input::PlayerMapping::IsNegated(FName PlayerMappingName, const UWorld* World)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(PlayerMapping_IsNegated);

	FEnhancedActionKeyMapping MappingDefinition = GetMapping(PlayerMappingName, World);
	// Iterate by reference to avoid copying TObjectPtr wrappers while checking modifier types.
	for (const auto& Modifier : MappingDefinition.Modifiers)
	{
		if (Modifier->IsA<UInputModifierNegate>())
		{
			return true;
		}
	}
	return false;
}

float SB::AutoSettings::Input::PlayerMapping::GetEffectiveDirectionalScale(const ULocalPlayer* LocalPlayer, FName PlayerMappingName)
{
	const FEnhancedInputSubsystemData Data = GetEnhancedInputSubsystemData(LocalPlayer);

	// Check for stored directional intent first
	if (Data.bIsValid && Data.UserSettings->MappingDataMap.Contains(PlayerMappingName))
	{
		const FMappingMetadata& Metadata = Data.UserSettings->GetMappingMetadata(PlayerMappingName);
		return Metadata.bNegate ? DirectionalScale_Negated : DirectionalScale_Normal;
	}

	// Fall back to original mapping design
	const UWorld* World = LocalPlayer ? LocalPlayer->GetWorld() : nullptr;
	return IsNegated(PlayerMappingName, World) ? DirectionalScale_Negated : DirectionalScale_Normal;
}

bool SB::AutoSettings::Input::Player::IsValidForInput(const APlayerController* PlayerController, bool bLogError,
	const FString& ErrorContext)
{
	if (!IsValid(PlayerController))
	{
		if (bLogError)
		{
			SB::AutoSettings::Errors::LogError(FString::Printf(TEXT("%s: Invalid Player Controller object"), *ErrorContext));
		}
		return false;
	}

	if (!PlayerController->IsLocalPlayerController())
	{
		if (bLogError)
		{
			SB::AutoSettings::Errors::LogError(FString::Printf(TEXT("%s: Player Controller '%s' is not locally controlled"),
				*ErrorContext, *PlayerController->GetHumanReadableName()));
		}
		return false;
	}

	// There are cases where a LocalPlayerController might have it's actual LocalPlayer stolen away from it (For example, by CheatManager EnableDebugCamera, which used to cause a crash)
	// So we need to check it actually has a LocalPlayer too
	if (!IsValid(PlayerController->GetLocalPlayer()))
	{
		if (bLogError)
		{
			SB::AutoSettings::Errors::LogError(FString::Printf(TEXT("%s: Player Controller '%s' does not have a valid LocalPlayer"),
				*ErrorContext, *PlayerController->GetHumanReadableName()));
		}
		return false;
	}

	return true;
}

FKey SB::AutoSettings::Input::Player::GetCurrentKeyForMapping(const ULocalPlayer* LocalPlayer, FName PlayerMappingName,
	const EPlayerMappableKeySlot& KeySlot)
{
	if (!ensure(LocalPlayer))
	{
		return EKeys::Invalid;
	}

	const auto UserSettings = UAutoSettingsEnhancedInputUserSettings::Get(LocalPlayer);
	if (ensure(UserSettings))
	{
		// Firstly, check for player mapping overrides
		// This will include all player-specified keys
		FMapPlayerKeyArgs Args;
		Args.MappingName = PlayerMappingName;
		Args.Slot = KeySlot;

		if (const FPlayerKeyMapping* Mapping = SB::AutoSettings::Input::GetActiveKeyProfile(UserSettings)->FindKeyMapping(Args))
		{
			FKey CurrentKey = Mapping->GetCurrentKey();
			if (CurrentKey == Unbound)
			{
				// Special case:
				// Return Invalid if player mapped key is explicitly Unbound
				return EKeys::Invalid;
			}
			return CurrentKey;
		}
	}

	// No player-specified key
	// Fall back to the default if applicable
	return PlayerMapping::GetDefaultKey(LocalPlayer, PlayerMappingName, KeySlot);
}

FKey SB::AutoSettings::Input::PlayerMapping::GetDefaultKey(const ULocalPlayer* LocalPlayer, FName PlayerMappingName,
	const EPlayerMappableKeySlot& KeySlot)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(PlayerMapping_GetDefaultKey);

	// Enhanced Input supports per-profile default keys, allowing different defaults for:
	// - Device-specific profiles (e.g., "Keyboard" vs "Gamepad" with different default bindings)
	// - Accessibility profiles (e.g., one-handed control schemes)
	// - Regional profiles (e.g., AZERTY vs QWERTY keyboard defaults)
	//
	// When a LocalPlayer is available, we check for profile-specific defaults first to support
	// these advanced use cases. This maintains compatibility with Enhanced Input's full feature set.
	if (LocalPlayer)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(Profile Lookup);
		if (const auto UserSettings = UAutoSettingsEnhancedInputUserSettings::Get(LocalPlayer))
		{
			FMapPlayerKeyArgs Args;
			Args.MappingName = PlayerMappingName;
			Args.Slot = KeySlot;
			UEnhancedPlayerMappableKeyProfile* Profile = nullptr;
			{
				TRACE_CPUPROFILER_EVENT_SCOPE(GetCurrentKeyProfile);
				Profile = SB::AutoSettings::Input::GetActiveKeyProfile(UserSettings);
			}
			const FPlayerKeyMapping* Mapping = nullptr;
			{
				TRACE_CPUPROFILER_EVENT_SCOPE(FindKeyMapping);
				Mapping = Profile->FindKeyMapping(Args);
			}
			if (Mapping)
			{
				FKey DefaultKey = Mapping->GetDefaultKey();
				if (DefaultKey.IsValid())
				{
					return DefaultKey;
				}
			}
		}
	}

	// Fall back to the default defined in the Input Mapping Context asset
	// This path is used when:
	// - No LocalPlayer is available (e.g., design-time editor preview)
	// - No profile-specific default exists
	// - The profile system hasn't been initialized yet
	if (KeySlot == EPlayerMappableKeySlot::First || KeySlot == EPlayerMappableKeySlot::Unspecified)
	{
		// Only the first slot has a default in the IMC asset
		// Additional slots are for player customization only
		const UWorld* World = LocalPlayer ? LocalPlayer->GetWorld() : nullptr;
		FEnhancedActionKeyMapping MappingDefinition = GetMapping(PlayerMappingName, World);
		return MappingDefinition.Key;
	}

	return EKeys::Invalid;
}

FPartialSettingKey SB::AutoSettings::Input::PlayerMapping::GetMappingSettingSubKey(FName PlayerMappingName,
	const EPlayerMappableKeySlot& KeySlot)
{
	FPartialSettingKey SettingKey;
	SettingKey.Fragments = { PlayerMappingName, FName(FString::FromInt(static_cast<int32>(KeySlot))) };
	return SettingKey;
}

const UInputAction* SB::AutoSettings::Input::Action::Find(FName Name)
{
	TArray<FAssetData> InputActionAssets;
	IAssetRegistry::Get()->GetAssetsByClass(UInputAction::StaticClass()->GetClassPathName(), InputActionAssets);
	for (const auto& Asset : InputActionAssets)
	{
		if (Asset.AssetName == Name)
		{
			const UInputAction* LoadedAction = Cast<UInputAction>(Asset.GetAsset());
			if (!ensure(LoadedAction))
			{
				// Loaded object is not an action?
				continue;
			}
			return LoadedAction;
		}
	}
	return nullptr;
}

TArray<UInputMappingContext*> SB::AutoSettings::Input::Action::FindMappingContexts(const UInputAction* InputAction, const UWorld* World)
{
	TArray<UInputMappingContext*> Result;

	const TArray<FAssetData>& MappingContextAssets = GetInputContextAssets(World);

	for (const auto& Asset : MappingContextAssets)
	{
		UInputMappingContext* LoadedContext = Cast<UInputMappingContext>(Asset.GetAsset());
		if (!ensure(LoadedContext))
		{
			// Loaded object is not a context?
			continue;
		}
		for (const auto& Mapping : LoadedContext->GetMappings())
		{
			if (Mapping.Action == InputAction)
			{
				Result.Add(LoadedContext);
			}
		}
	}
	return Result;
}

TArray<FKey> SB::AutoSettings::Input::Player::GetCurrentKeysForAction(const ULocalPlayer* LocalPlayer,
	const UInputAction* InputAction)
{
	if (!ensure(LocalPlayer))
	{
		return {};
	}

	const auto Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

	TArray<FKey> Keys = Subsystem->QueryKeysMappedToAction(InputAction);

	return Keys;
}

TArray<FKey> SB::AutoSettings::Input::Action::GetDefaultKeys(const UInputAction* InputAction)
{
	const auto Contexts = FindMappingContexts(InputAction);

	if (!ensure(Contexts.Num() > 0))
	{
		// Didn't find any contexts with this action
		return {};
	}

	// Just use the first for now
	const auto Context = Contexts[0];

	// Find keys mapped to the action in the context
	TArray<FKey> Result;
	for (const auto& Mapping : Context->GetMappings())
	{
		if (Mapping.Action == InputAction)
		{
			Result.Add(Mapping.Key);
		}
	}

	return Result;
}

FText SB::AutoSettings::Input::Mapping::GetDisplayName(const FEnhancedActionKeyMapping& Mapping)
{
	// First check the settings
	const auto PlayerMappableKeySettings = Mapping.GetPlayerMappableKeySettings();
	if (IsValid(PlayerMappableKeySettings))
	{
		if (!PlayerMappableKeySettings->DisplayName.IsEmpty())
		{
			// If the display name is set, use that
			return PlayerMappableKeySettings->DisplayName;
		}
	}

	// Fall back to the internal name of the mapping so that we can at least show something
	FName MappingName = Mapping.GetMappingName();
	UE_LOGFMT(LogAutoSettingsInput, Warning, "Mapping {Name} has no Display Name, using the FName instead", MappingName);
	return FText::FromName(MappingName);
}

TArray<TTuple<FPlayerKeyMapping, UInputMappingContext*>> SB::AutoSettings::Input::Player::GetCurrentMappingsInContextsWithKey(
	const ULocalPlayer* LocalPlayer, TArray<UInputMappingContext*> Contexts, const FKey& Key)
{
	const auto Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	const auto UserSettings = Subsystem->GetUserSettings();

	if (!ensure(UserSettings))
	{
		return {};
	}

	TArray<TTuple<FPlayerKeyMapping, UInputMappingContext*>> Result;

	for (const auto& Context : Contexts)
	{
		if (!ensure(Context))
		{
			continue;
		}

		if (!UserSettings->IsMappingContextRegistered(Context))
		{
			UserSettings->RegisterInputMappingContext(Context);
		}
	}

	TArray<FName> MappingsForKey;
	SB::AutoSettings::Input::GetActiveKeyProfile(UserSettings)->GetMappingNamesForKey(Key, MappingsForKey);

	for (const auto& MappingName : MappingsForKey)
	{
		auto Context = PlayerMapping::GetMappingContext(MappingName);
		if (!Contexts.Contains(Context))
		{
			continue;
		}
		auto FoundMappings = UserSettings->FindMappingsInRow(MappingName);
		for (const auto& FoundMapping : FoundMappings)
		{
			if (FoundMapping.GetCurrentKey() == Key)
			{
				Result.Add({ FoundMapping, Context });
			}
		}
	}

	return Result;
}
