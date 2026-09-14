// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AutoSettingsInputSubsystem.generated.h"

class UAutoSettingsInputDataCache;

/**
 * GameInstance subsystem for handling Auto Settings input functionality
 *
 * This subsystem handles input-related configuration and validation at the GameInstance level,
 * ensuring it runs for every game session including Play In Editor.
 */
UCLASS()
class AUTOSETTINGSINPUT_API UAutoSettingsInputSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Initializes the input subsystem
	 * Validates input-related configuration and sets up required dependencies
	 *
	 * @param Collection - The subsystem collection handling initialization
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Deinitializes the input subsystem
	 * Performs cleanup for input-related resources
	 */
	virtual void Deinitialize() override;

	/**
	 * Get the internal mapping cache for performance optimization
	 * @return Cache instance or nullptr if not available
	 */
	UAutoSettingsInputDataCache* GetMappingCache() const { return MappingCache; }

private:
	/** Internal cache for Input Mapping Context assets */
	UPROPERTY(Transient)
	TObjectPtr<UAutoSettingsInputDataCache> MappingCache;
};