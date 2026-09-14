// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "AutoSettingsEnhancedInputUserSettings.generated.h"

/**
 * Extensible metadata structure for storing additional data per mapping
 */
USTRUCT(BlueprintType)
struct AUTOSETTINGSINPUT_API FMappingMetadata
{
	GENERATED_BODY()

	/** Whether this mapping should have its axis negated */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "AutoSettings|Input|MappingMetadata")
	bool bNegate = false;
};

/**
 * This User Settings class is required for Auto Settings to detect when Enhanced Input key mappings are updated
 */
UCLASS()
class AUTOSETTINGSINPUT_API UAutoSettingsEnhancedInputUserSettings : public UEnhancedInputUserSettings
{
	GENERATED_BODY()

public:
	/**
	 * Typed convenience getter
	 */
	static UAutoSettingsEnhancedInputUserSettings* Get(const ULocalPlayer* LocalPlayer);

	/*
	 * Delegate fired when a key mapping is changed
	 * This differs from the delegate on the base class - it provides a param for the changed mapping,
	 * and it is non-dynamic so that can have lambdas bound to it
	 */
	TMulticastDelegate<void(FPlayerKeyMapping)> OnKeyMappingUpdatedEvent;
	/**
	 * Sets the metadata for a mapping
	 */
	UFUNCTION(BlueprintCallable, Category = "AutoSettings|Input")
	void SetMappingMetadata(const FName& MappingName, const FMappingMetadata& Metadata);

	/**
	 * Gets the metadata for a mapping
	 */
	UFUNCTION(BlueprintCallable, Category = "AutoSettings|Input")
	FMappingMetadata GetMappingMetadata(const FName& MappingName) const;

	/**
	 * Gets the directional intent for a mapping (convenience function)
	 */
	UFUNCTION(BlueprintCallable, Category = "AutoSettings|Input")
	bool GetMappingNegateFlag(const FName& MappingName) const;

	/**
	 * Clears the stored metadata for a mapping (used when resetting to default)
	 */
	UFUNCTION(BlueprintCallable, Category = "AutoSettings|Input")
	void ClearMappingMetadata(const FName& MappingName);

	/**
	 * Maps mapping name to custom metadata for extensible per-mapping data storage
	 */
	UPROPERTY(SaveGame)
	TMap<FName, FMappingMetadata> MappingDataMap;

protected:
	/**
	 * Overridden to fire the delegate when mappings are updated
	 */
	virtual void OnKeyMappingUpdated(FPlayerKeyMapping* ChangedMapping, const FMapPlayerKeyArgs& InArgs, const bool bIsBeingUnmapped) override;
};