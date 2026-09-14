// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "Widgets/AutoSettingWidget.h"
#include "SliderSettingWidget.generated.h"

/**
 * Base AutoSetting with generic slider functionality
 */
UCLASS(abstract)
class AUTOSETTINGSUI_API USliderSettingWidget : public UAutoSettingWidget
{
	GENERATED_BODY()

public:
	USliderSettingWidget(const FObjectInitializer& ObjectInitializer);

	/**
	 * If true, we will keep the slider values in normalized 0-1 space, and remap the values in the setting widget if necessary.
	 * If false, the slider will be given the exact value of the setting.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting|Slider")
	bool bNormalizeValues = true;

	// Value assigned with the left-most position of the slider
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Slider")
	float LeftValue;

	// Value assigned with the right-most position of the slider
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Slider")
	float RightValue;

	// Clamps the setting value between the Left Value and Right Value of the slider
	UFUNCTION(BlueprintPure, Category = "Setting|Slider")
	float ClampSettingValue(float SettingValue) const;

	UFUNCTION(BlueprintPure, Category = "Setting|Slider")
	float SettingToSliderValue(float SettingValue) const;

	UFUNCTION(BlueprintPure, Category = "Setting|Slider")
	float SliderToSettingValue(float SliderValue) const;

	UFUNCTION(BlueprintPure, Category = "Setting|Slider")
	float GetMinSettingValue() const;

	UFUNCTION(BlueprintPure, Category = "Setting|Slider")
	float GetMaxSettingValue() const;

protected:
	virtual void UpdateSelection_Implementation(const FString& Value) override;

	// Called to update the slider with a new setting value
	// Not called when the slider's value itself is changed
	// @param SliderValue Value in space of the slider
	UFUNCTION(BlueprintNativeEvent, Category = "Setting|Slider")
	void UpdateSliderValue(float SliderValue);

	// Notify the setting that the slider's value has updated
	UFUNCTION(BlueprintCallable, Category = "Setting|Slider")
	void SliderValueUpdated(float SliderValue);

	// Called when the slider's value changes
	// Useful for updating UI such as labels to reflect the internal value
	UFUNCTION(BlueprintImplementableEvent, Category = "Setting|Slider")
	void OnSliderValueUpdated(float SliderValue, float SettingValue);

	// Used to determine if the slider value can also be saved when it is applied
	// e.g. Apply the value as the slider value is updating from mouse drag, but for optimization only save when the handle is released
	UFUNCTION(BlueprintNativeEvent, Category = "Setting|Slider")
	bool ShouldSaveCurrentValue() const;
};
