// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Misc/SettingValueMask.h"
#include "GameplayTagContainer.h"
#include "Framework/SettingType.h"
#include "AutoSettingWidget.generated.h"

/**
 * Widget that exposes a setting as a UI control
 */
UCLASS(abstract)
class AUTOSETTINGSUI_API UAutoSettingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UAutoSettingWidget(const FObjectInitializer& ObjectInitializer);

	// Should this setting automatically save to config when changed?
	// Saving also applies the setting if it hasn't been already
	// If false, "Save" function can be called for manual control
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting")
	bool bAutoSave;

	// Should this setting automatically apply when changed?
	// Will also apply automatically if AutoSave is true
	// If false, "Apply" function can be called for manual control
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting", meta = (EditCondition = "!bAutoSave"))
	bool bAutoApply;

#if WITH_EDITOR
	static inline TDelegate<USettingRegistry*(ESettingDomain)> GetDesignTimeRegistry;
#endif

	/** Find the AutoSettingWidget contained within a Named Slot widget */
	UFUNCTION(BlueprintPure, Category = "Setting")
	static UAutoSettingWidget* FindInNamedSlot(const UNamedSlot* NamedSlot);

	const USettingType* GetSettingType() const;

	/** Configure this widget to use the specified setting specifier. SettingType is required. */
	UFUNCTION(BlueprintCallable, Category = "Setting")
	void SetSetting(const FSettingSpecifier& InSetting);

	// True if the setting has changed since the last time it was applied
	UFUNCTION(BlueprintPure, Category = "Setting")
	bool HasUnappliedChange() const
	{
		return bHasUnappliedChange;
	}

	// True if the setting has changed since the last time it was saved
	UFUNCTION(BlueprintPure, Category = "Setting")
	bool HasUnsavedChange() const
	{
		return bHasUnsavedChange;
	}

	// Apply the setting by pushing the current widget value into the registry
	UFUNCTION(BlueprintCallable, Category = "Setting")
	void Apply();

	UFUNCTION(BlueprintPure, Category = "Setting")
	FString GetDefaultValue() const;

	UFUNCTION(BlueprintPure, Category = "Setting")
	bool CanResetToDefault() const;

	UFUNCTION(BlueprintCallable, Category = "Setting")
	void ResetToDefault();

	// Save the current value to config
	// This will also Apply the current value if it hasn't been already
	UFUNCTION(BlueprintCallable, Category = "Setting")
	void Save();

	// Cancel unsaved changes, reverting to the value saved in config
	UFUNCTION(BlueprintCallable, Category = "Setting")
	void Cancel();

	FString GetDebugName() const;

protected:
	/** The setting targeted by this widget. SettingType is required for widgets. */
	UPROPERTY(EditAnywhere, Category = "Setting")
	FSettingSpecifier Setting;

	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Set value on control widget, used for setting default and current value
	UFUNCTION(BlueprintNativeEvent, Category = "Setting")
	void UpdateSelection(const FString& Value);

	// Update the setting's selected value
	// @param bSaveIfPossible Save the setting if AutoSave is enabled
	UFUNCTION(BlueprintCallable, Category = "Setting")
	void ApplySettingValue(FString Value, bool bSaveIfPossible = true);

	bool IsUpdatingSettingSelection() const
	{
		return bUpdatingSettingSelection;
	}

	/**
	 * Get the full setting key
	 */
	FFullSettingKey GetFullSettingKey() const;

	USettingRegistry* GetRegistry(bool bChecked = true) const;

	virtual void PostLoad() override;
	virtual void PostInitProperties() override;

private:
	// Value that is currently set on the widget - it may not be applied or saved
	UPROPERTY()
	FString CurrentValue;

	UPROPERTY()
	bool bHasUnappliedChange;

	UPROPERTY()
	bool bHasUnsavedChange;

	// True if UpdateSelection is being run
	UPROPERTY()
	bool bUpdatingSettingSelection;

	UPROPERTY()
	FName SettingKey_DEPRECATED;

	UPROPERTY()
	FSettingKey SettingKeyExtension_DEPRECATED;

	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Use Setting instead."))
	TSubclassOf<USettingType> SettingType_DEPRECATED;

	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Use Setting instead."))
	FPartialSettingKey SubKey_DEPRECATED;

	// Sets the widget to the value of the setting from the registry or config
	void ResetToSettingValue(bool bPreferSavedValue = false);

	void DispatchUpdateSelection(const FString& Value);

	UFUNCTION()
	void OnAppliedValueChanged(const FSettingReference& SettingRef, const FString& Value);

	void LogMissingSetting() const;

	FSettingReference GetSettingReference() const;

	FSettingContext GetSettingContext() const;

	void MigrateDeprecatedProperties();
};
