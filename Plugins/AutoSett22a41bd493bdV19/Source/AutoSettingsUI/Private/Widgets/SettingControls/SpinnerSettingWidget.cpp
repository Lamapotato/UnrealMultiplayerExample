// Copyright Sam Bonifacio. All Rights Reserved.

#include "Widgets/SettingControls/SpinnerSettingWidget.h"

void USpinnerSettingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(Spinner))
	{
		Spinner->SelectionChangedEvent.AddUniqueDynamic(this, &USpinnerSettingWidget::SpinnerSelectionChanged);
	}
}

void USpinnerSettingWidget::UpdateSelection_Implementation(const FString& Value)
{
	if (IsValid(Spinner))
	{
		Spinner->SelectValue(Value);
	}
}

void USpinnerSettingWidget::UpdateOptions_Implementation(const TArray<FSettingOption>& InOptions)
{
	if (IsValid(Spinner))
	{
		Spinner->Options = InOptions;
	}
}

void USpinnerSettingWidget::SpinnerSelectionChanged(FString Value)
{
	ApplySettingValue(Value);
}