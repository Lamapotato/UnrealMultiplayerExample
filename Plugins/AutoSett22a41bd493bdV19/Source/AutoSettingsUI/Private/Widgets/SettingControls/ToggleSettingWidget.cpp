// Copyright Sam Bonifacio. All Rights Reserved.

#include "Widgets/SettingControls/ToggleSettingWidget.h"
#include "Utility/AutoSettingsStringUtils.h"

void UToggleSettingWidget::UpdateSelection_Implementation(const FString& Value)
{
	Super::UpdateSelection_Implementation(Value);

	UpdateToggleState(FAutoSettingsStringUtils::IsTruthy(Value));
}

void UToggleSettingWidget::UpdateToggleState_Implementation(bool State)
{
}

void UToggleSettingWidget::ToggleStateUpdated(bool State)
{
	const int32 IntValue = State ? 1 : 0;

	ApplySettingValue(FString::FromInt(IntValue));
}