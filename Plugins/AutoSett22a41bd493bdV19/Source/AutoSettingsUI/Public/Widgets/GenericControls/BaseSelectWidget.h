// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "Misc/SettingOption.h"
#include "BaseSelectWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSelectionChanged, FString, Value);

/**
 * Base widget class for selection-based UI controls that cycle through predefined options.
 */
UCLASS(Abstract)
class AUTOSETTINGSUI_API UBaseSelectWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Option definitions. Empty array will result in no selectable options. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Select")
	TArray<FSettingOption> Options;

	// Should the first and last options wrap around to each other?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Select")
	bool bAllowWrapping;

	// Fired when selection changes
	UPROPERTY(BlueprintAssignable, Category = "Select")
	FSelectionChanged SelectionChangedEvent;

	// Returns the available options
	UFUNCTION(BlueprintPure, Category = "Select")
	TArray<FSettingOption> GetOptions() const
	{
		return Options;
	}

	// Set the available options
	UFUNCTION(BlueprintCallable, Category = "Radio Select")
	virtual void SetOptions(TArray<FSettingOption> InOptions);

	// Select the option with the specified value
	UE_DEPRECATED(1.20, "Use SelectValue instead.")
	UFUNCTION(BlueprintCallable, Category = "Select", meta = (DeprecatedFunction, DeprecationMessage = "Use Select Value instead."))
	void Select(FString Value);

	// Select the option with the specified value
	UFUNCTION(BlueprintCallable, Category = "Select")
	void SelectValue(FString Value);

	// Select the option with the specified index
	UFUNCTION(BlueprintCallable, Category = "Select")
	void SelectIndex(int32 Index);

	// Select the next option
	UFUNCTION(BlueprintCallable, Category = "Select")
	void Next();

	// Select the previous option
	UFUNCTION(BlueprintCallable, Category = "Select")
	void Previous();

	// True if selected option is not the last, or if wrapping is enabled
	UFUNCTION(BlueprintPure, Category = "Select")
	bool HasValidNext() const;

	// True if selected option is not the first, or if wrapping is enabled
	UFUNCTION(BlueprintPure, Category = "Select")
	bool HasValidPrevious() const;

	// Return the currently selected option
	UFUNCTION(BlueprintPure, Category = "Select")
	FSettingOption GetCurrentOption() const;

	// Return the index of the currently selected option
	UFUNCTION(BlueprintPure, Category = "Select")
	int32 GetCurrentIndex() const
	{
		return CurrentIndex;
	}

	UFUNCTION(BlueprintNativeEvent, Category = "Select")
	void OnSelectionChanged(FSettingOption SelectedOption);

protected:
	virtual void OnWidgetRebuilt() override;

private:
	UPROPERTY()
	int32 CurrentIndex;
};