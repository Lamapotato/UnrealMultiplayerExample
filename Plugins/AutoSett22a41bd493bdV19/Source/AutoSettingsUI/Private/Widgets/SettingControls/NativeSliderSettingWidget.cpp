// Copyright Sam Bonifacio. All Rights Reserved.

#include "Widgets/SettingControls/NativeSliderSettingWidget.h"

void UNativeSliderSettingWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (IsValid(Slider))
	{
		Slider->SetMinValue(SettingToSliderValue(LeftValue));
		Slider->SetMaxValue(SettingToSliderValue(RightValue));
	}
}

void UNativeSliderSettingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(Slider))
	{
		Slider->OnValueChanged.AddUniqueDynamic(this, &UNativeSliderSettingWidget::SliderValueChanged);
		Slider->OnMouseCaptureBegin.AddUniqueDynamic(this, &UNativeSliderSettingWidget::SliderMouseCaptureBegin);
		Slider->OnMouseCaptureEnd.AddUniqueDynamic(this, &UNativeSliderSettingWidget::SliderMouseCaptureEnd);
	}
}

void UNativeSliderSettingWidget::UpdateSliderValue_Implementation(float SliderValue)
{
	if (IsValid(Slider))
	{
		Slider->SetValue(SliderValue);
	}
}

void UNativeSliderSettingWidget::SliderValueChanged(float NewSliderValue)
{
	SliderValueUpdated(NewSliderValue);
}

void UNativeSliderSettingWidget::SliderMouseCaptureBegin()
{
	bMouseCaptureInProgress = true;
}

void UNativeSliderSettingWidget::SliderMouseCaptureEnd()
{
	bMouseCaptureInProgress = false;

	// Call SliderValueUpdated after released to trigger a save if we need one
	SliderValueUpdated(Slider->GetValue());
}