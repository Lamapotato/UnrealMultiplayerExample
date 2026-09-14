// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "SliderSettingWidget.h"
#include "Components/Slider.h"
#include "NativeSliderSettingWidget.generated.h"

/**
 * AutoSetting for a native Unreal Slider
 */
UCLASS(abstract)
class AUTOSETTINGSUI_API UNativeSliderSettingWidget : public USliderSettingWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Setting|Native Slider", meta = (BindWidget))
	USlider* Slider;

	virtual void NativePreConstruct() override;

	virtual void NativeConstruct() override;

	virtual void UpdateSliderValue_Implementation(float SliderValue) override;

	// Only save if handle is released
	virtual bool ShouldSaveCurrentValue_Implementation() const override
	{
		return !bMouseCaptureInProgress;
	}

private:
	UPROPERTY()
	bool bMouseCaptureInProgress;

	UFUNCTION()
	void SliderValueChanged(float NewSliderValue);

	UFUNCTION()
	void SliderMouseCaptureBegin();

	UFUNCTION()
	void SliderMouseCaptureEnd();
};