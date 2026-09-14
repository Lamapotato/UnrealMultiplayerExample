// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "SettingRegistry.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AutoSettingsSubsystem.generated.h"

class USettingRegistry;

/**
 * Game Instance Subsystem that manages the AutoSettings plugin's core functionality.
 *
 * This subsystem is responsible for:
 * - Initializing and managing the game's setting registry
 * - Acting as the main entry point for accessing AutoSettings functionality
 * - Coordinating with other AutoSettings subsystems
 *
 * The subsystem always exposes the global setting registry, and it can initialize that registry
 * automatically or wait for an explicit Blueprint/C++ request depending on project settings.
 */
UCLASS()
class AUTOSETTINGS_API UAutoSettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UAutoSettingsSubsystem();

	/** Returns the global registry object, which may be uninitialized when startup mode requires an explicit initialization call. */
	UFUNCTION(BlueprintPure, Category = "Settings")
	USettingRegistry* GetGameSettingRegistry() const
	{
		return GameSettingRegistry;
	}

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

private:
	/** Attempts automatic startup once the global registry has the assets it needs. */
	void TryInitializeSettingRegistry();

	/** The global setting registry that manages all game-wide settings */
	UPROPERTY()
	USettingRegistry* GameSettingRegistry;
};
