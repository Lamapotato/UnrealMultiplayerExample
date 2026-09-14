// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Framework/SettingKey.h"
#include "SettingTypes.generated.h"

/**
 * Container for setting value data with validation and serialization support.
 *
 * This struct wraps the string value that represents a setting's data,
 * providing import/export functionality for persistence and validation.
 */
USTRUCT(BlueprintType)
struct FSettingData
{
	GENERATED_BODY()

	// The actual value of the setting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FString Value;

	bool IsValid() const
	{
		return !Value.IsEmpty();
	}

	FString ExportString() const;

	static FSettingData ImportString(FString String);
};

/**
 * Context information that identifies which registry a setting belongs to.
 *
 * This allows settings to operate across multiple registries (e.g., global vs player-specific).
 */
USTRUCT(BlueprintType)
struct FSettingContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	class USettingRegistry* Registry = nullptr;

	FSettingContext() {}

	FSettingContext(USettingRegistry* Registry)
		: Registry(Registry) {}
};

/**
 * Complete reference to a specific setting, combining its unique key with registry context.
 *
 * This struct provides everything needed to locate and operate on a setting:
 * - The full hierarchical key that uniquely identifies the setting
 * - The registry context that contains the setting
 */
USTRUCT(BlueprintType)
struct FSettingReference
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	FFullSettingKey Key;

	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	FSettingContext Context;
};

/**
 * Event data structure used when applying or saving settings.
 *
 * This struct contains all the information needed for setting operations:
 * - Complete setting reference (key + registry context)
 * - The value data being set
 * - Associated gameplay tags for filtering and categorization
 */
USTRUCT(BlueprintType)
struct FSetSettingEvent
{
	GENERATED_BODY()

	// Reference to the exact setting being set
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FSettingReference Setting;

	// Data structure of the setting to be set
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FSettingData Data;

	// Tags of the setting being saved
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Transient, Category = "Settings")
	FGameplayTagContainer Tags;

	FSetSettingEvent() {}

	FSetSettingEvent(const FSettingReference& Setting, FSettingData Data, FGameplayTagContainer Tags)
		: Setting(Setting)
		, Data(Data)
		, Tags(Tags) {}

	USettingRegistry* GetRegistry() const;
};
