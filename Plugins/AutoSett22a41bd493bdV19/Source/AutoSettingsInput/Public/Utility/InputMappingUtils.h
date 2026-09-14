// Copyright Sam Bonifacio. All Rights Reserved.
#pragma once

#include "Engine/LocalPlayer.h"
#include "EnhancedActionKeyMapping.h"
#include "Framework/SettingKey.h"
#include "UserSettings/EnhancedInputUserSettings.h"

enum class EPlayerMappableKeySlot : uint8;
struct FPlayerMappableKeySlot;
class UInputMappingContext;

class UEnhancedInputLocalPlayerSubsystem;
class UAutoSettingsEnhancedInputUserSettings;

namespace SB::AutoSettings::Input
{
	/**
	 * A sentinel key value signifying that a mapping is unbound
	 * We can't use "Invalid" because that's what Enhanced Input returns as its own sentinel value when there is no mapping, which there is unfortunately no way to check for with any other API (e.g. something like HasPlayerMappedKey)
	 * However, "no mapping" is different from "unbound", because the former falls back to the default value while the latter obscures it
	 *	"Invalid" -> Player has never mapped the key -> Use the default key
	 *	"Unbound" -> Player has explicitly unbound the key -> Do not use any key at all
	 */
	inline FKey Unbound = FKey("Unbound");

	// Constants for directional scaling
	static constexpr float DirectionalScale_Normal = 1.0f;
	static constexpr float DirectionalScale_Negated = -1.0f;

	// Utility struct for consolidated Enhanced Input subsystem access
	struct FEnhancedInputSubsystemData
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = nullptr;
		UAutoSettingsEnhancedInputUserSettings* UserSettings = nullptr;
		bool bIsValid = false;
	};

	// Helper function to get Enhanced Input subsystem data from a LocalPlayer
	FEnhancedInputSubsystemData GetEnhancedInputSubsystemData(const ULocalPlayer* LocalPlayer);

	namespace Action
	{
		const UInputAction* Find(FName Name);

		TArray<UInputMappingContext*> FindMappingContexts(const UInputAction* InputAction, const UWorld* World = nullptr);

		TArray<FKey> GetDefaultKeys(const UInputAction* InputAction);
	} // namespace Action

	namespace Mapping
	{
		FText GetDisplayName(const FEnhancedActionKeyMapping& Mapping);
	}

	namespace PlayerMapping
	{
		FEnhancedActionKeyMapping GetMapping(FName PlayerMappingName, const UWorld* World = nullptr);

		UInputMappingContext* GetMappingContext(FName PlayerMappingName, const UWorld* World = nullptr);

		bool IsNegated(FName PlayerMappingName, const UWorld* World = nullptr);

		// Get the effective directional scale for a mapping, checking stored metadata first, then falling back to original mapping design
		// Returns 1.0f for positive direction (forward/right), -1.0f for negative direction (backward/left)
		float GetEffectiveDirectionalScale(const ULocalPlayer* LocalPlayer, FName PlayerMappingName);

		FKey GetDefaultKey(const ULocalPlayer* LocalPlayer, FName PlayerMappingName, const EPlayerMappableKeySlot& KeySlot = EPlayerMappableKeySlot::Unspecified);

		FPartialSettingKey GetMappingSettingSubKey(FName PlayerMappingName, const EPlayerMappableKeySlot& KeySlot);
	} // namespace PlayerMapping

	namespace Player
	{
		// Checks if the given Player Controller is valid for input mapping purposes (is a local player)
		bool IsValidForInput(const APlayerController* PlayerController, bool bLogError = false, const FString& ErrorContext = FString());

		// Returns the player mapped key, or otherwise the default key for a mapping
		FKey GetCurrentKeyForMapping(const ULocalPlayer* Player, FName PlayerMappingName, const EPlayerMappableKeySlot& KeySlot = EPlayerMappableKeySlot::Unspecified);

		TArray<FKey> GetCurrentKeysForAction(const ULocalPlayer* Player, const UInputAction* InputAction);

		TArray<TTuple<FPlayerKeyMapping, UInputMappingContext*>> GetCurrentMappingsInContextsWithKey(const ULocalPlayer* Player, TArray<UInputMappingContext*> Contexts, const FKey& Key);
	} // namespace Player

	// Utility functions for asset and mapping operations
	const TArray<FAssetData>& GetInputContextAssets(const UWorld* World);
} // namespace SB::AutoSettings::Input