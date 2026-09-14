// Copyright Sam Bonifacio. All Rights Reserved.

#include "Widgets/SettingControls/SliderSettingWidget.h"

USliderSettingWidget::USliderSettingWidget(const FObjectInitializer& ObjectInitializer)
	: UAutoSettingWidget(ObjectInitializer)
	, RightValue(1.0f)
{
}

float USliderSettingWidget::ClampSettingValue(float SettingValue) const
{
	const bool bLeftMin = LeftValue < RightValue;
	return FMath::Clamp(SettingValue, bLeftMin ? LeftValue : RightValue, bLeftMin ? RightValue : LeftValue);
}

float USliderSettingWidget::SettingToSliderValue(float SettingValue) const
{
	if (bNormalizeValues)
	{
		return FMath::GetMappedRangeValueUnclamped(FVector2D(LeftValue, RightValue), FVector2D(0.0f, 1.0f), SettingValue);
	}
	return SettingValue;
}

float USliderSettingWidget::SliderToSettingValue(float SliderValue) const
{
	if (bNormalizeValues)
	{
		return FMath::GetMappedRangeValueUnclamped(FVector2D(0.0f, 1.0f), FVector2D(LeftValue, RightValue), SliderValue);
	}
	return SliderValue;
}

float USliderSettingWidget::GetMinSettingValue() const
{
	return FMath::Min(LeftValue, RightValue);
}

float USliderSettingWidget::GetMaxSettingValue() const
{
	return FMath::Max(LeftValue, RightValue);
}

void USliderSettingWidget::UpdateSelection_Implementation(const FString& Value)
{
	Super::UpdateSelection_Implementation(Value);

	const float SettingValue = FCString::Atof(*Value);
	const float SliderValue = SettingToSliderValue(SettingValue);

	UpdateSliderValue(SliderValue);

	OnSliderValueUpdated(SliderValue, SettingValue);
}

void USliderSettingWidget::UpdateSliderValue_Implementation(float SliderValue)
{
}

void USliderSettingWidget::SliderValueUpdated(float SliderValue)
{
	const float SettingValue = SliderToSettingValue(SliderValue);

	ApplySettingValue(FString::SanitizeFloat(SettingValue), ShouldSaveCurrentValue());

	OnSliderValueUpdated(SliderValue, SettingValue);
}

bool USliderSettingWidget::ShouldSaveCurrentValue_Implementation() const
{
	return true;
}