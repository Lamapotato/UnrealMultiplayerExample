// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/SettingTypes.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "AutoSettingsPlayerSubsystem.generated.h"

class USettingRegistry;
class APlayerController;
class ULocalPlayer;
/**
 * A LocalPlayerSubsystem that manages player-specific settings.
 *
 * This subsystem creates and maintains a player-specific SettingRegistry
 * that handles settings which are unique to each player, such as input bindings,
 * player preferences, and other per-player configurations.
 *
 * Each local player will have their own instance of this subsystem, and project settings can
 * choose whether each player registry initializes automatically or waits for an explicit request.
 */
UCLASS()
class AUTOSETTINGS_API UAutoSettingsPlayerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	UAutoSettingsPlayerSubsystem();

	/**
	 * Initializes the subsystem, configures registry prerequisites, and runs automatic startup when enabled.
	 *
	 * @param Collection - The subsystem collection handling initialization
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Cleans up the subsystem when it's being destroyed.
	 * This releases active player registry bindings before the local player subsystem shuts down.
	 */
	virtual void Deinitialize() override;

	/**
	 * Returns the player-specific setting registry.
	 * This registry contains all settings that are specific to this player. Explicit-startup modes can
	 * leave it uninitialized until gameplay requests initialization.
	 *
	 * @return The player's setting registry instance
	 */
	UFUNCTION(BlueprintPure, Category = "Auto Settings")
	USettingRegistry* GetPlayerSettingRegistry() const
	{
		return PlayerSettingRegistry;
	}

	/**
	 * Resolve the owning local player from a setting registry if the registry is player-scoped.
	 * Returns null for global registries or if no player context is available yet.
	 */
	static ULocalPlayer* GetOwningLocalPlayerFromRegistry(const USettingRegistry* Registry);

	/**
	 * Resolve the owning player controller from a setting registry if the registry is player-scoped.
	 * Returns null for global registries or if no player/controller is available yet.
	 */
	UFUNCTION(BlueprintPure, Category = "Auto Settings")
	static APlayerController* GetOwningPlayerControllerFromRegistry(const USettingRegistry* Registry);

	/**
	 * Resolve the owning player controller from a setting reference if it is player-scoped.
	 * Returns null for global settings or if no player/controller is available yet.
	 */
	UFUNCTION(BlueprintPure, Category = "Auto Settings")
	static APlayerController* GetOwningPlayerControllerFromSetting(const FSettingReference& Setting);

private:
	/** Configure the player-owned setting assets after the asset manager finishes its initial scan. */
	void UpdatePlayerRegistrySettingAssets();

	/** Update the player-specific base key whenever the owning controller identity is available. */
	void UpdatePlayerRegistryBaseKey();

	/** The registry that manages all player-specific settings */
	UPROPERTY()
	USettingRegistry* PlayerSettingRegistry;

	/**
	 * Attempts to initialize the setting registry if prerequisites are met.
	 * Automatic startup waits for both player setting assets and a stable player namespace.
	 */
	void TryInitializeSettingRegistry();
};
