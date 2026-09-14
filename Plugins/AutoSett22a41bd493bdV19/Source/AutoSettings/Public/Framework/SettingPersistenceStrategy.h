// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SettingTypes.h"

#include "SettingPersistenceStrategy.generated.h"

/**
 * Base class for implementing how settings are saved and loaded from persistent storage.
 *
 * This class uses the Strategy pattern to allow different storage methods for different
 * types of settings (config files, save games, external databases, etc.).
 *
 * Inherit from this class and implement OnSave and GetSavedValue to create custom persistence strategies.
 */
UCLASS(Abstract, EditInlineNew, Blueprintable)
class AUTOSETTINGS_API USettingPersistenceStrategy : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Saves a setting value to persistent storage.
	 * Logs the change, skips saving in design time, and broadcasts save events.
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Settings")
	void Save(const FSetSettingEvent& Event);

	/**
	 * Retrieves a saved setting value from persistent storage.
	 * Default implementation returns invalid data - subclasses must override.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Auto Settings")
	FSettingData GetSavedValue(const FSettingReference& Setting) const;

	UFUNCTION(BlueprintCallable, Category = "Auto Settings")
	bool HasSavedValue(const FSettingReference& Setting) const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Auto Settings")
	TArray<FFullSettingKey> FindKeys() const;

	UFUNCTION(BlueprintCallable, Category = "Auto Settings")
	TArray<FFullSettingKey> FindChildKeys(const FFullSettingKey& ParentKey);

protected:
	UFUNCTION(BlueprintNativeEvent)
	void OnSave(const FSetSettingEvent& Event);
};