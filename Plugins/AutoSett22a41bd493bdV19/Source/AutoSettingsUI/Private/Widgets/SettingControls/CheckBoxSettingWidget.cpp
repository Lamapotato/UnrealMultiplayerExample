// Copyright Sam Bonifacio. All Rights Reserved.

#include "Widgets/SettingControls/CheckBoxSettingWidget.h"

void UCheckBoxSettingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(CheckBox))
	{
		CheckBox->OnCheckStateChanged.AddUniqueDynamic(this, &UCheckBoxSettingWidget::CheckBoxStateChanged);
	}
}

void UCheckBoxSettingWidget::UpdateToggleState_Implementation(bool State)
{
	if (IsValid(CheckBox))
	{
		CheckBox->SetCheckedState(State ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
	}
}

void UCheckBoxSettingWidget::CheckBoxStateChanged(bool IsChecked)
{
	ToggleStateUpdated(IsChecked);
}