// Copyright Sam Bonifacio. All Rights Reserved.

#include "ButtonGroupSettingWidget.h"

#include "AutoSettingsError.h"
#include "AutoSettingsLogs.h"
#include "Groups/CommonButtonGroupBase.h"
#include "Logging/StructuredLog.h"

void UButtonGroupSettingWidget::NativePreConstruct()
{
	// Create the ButtonGroup before calling Super, which will populate the options
	ButtonGroup = NewObject<UCommonButtonGroupBase>(this);
	ButtonGroup->NativeOnSelectedButtonBaseChanged.AddWeakLambda(this, [this](UCommonButtonBase* Button, int32 Index)
		{
			ApplySettingValue(ButtonValueMap[Button]);

			if (HasFocusedDescendants())
			{
				Button->SetFocus();
			}
		});

	Super::NativePreConstruct();
}

void UButtonGroupSettingWidget::UpdateOptions_Implementation(const TArray<FSettingOption>& InOptions)
{
	Super::UpdateOptions_Implementation(InOptions);

	RemoveAllButtons();

	if (!IsValid(ButtonGroup))
	{
		return;
	}

	if (!ensure(ButtonClass))
	{
		SB::AutoSettings::Errors::LogInvalidClass(GetClass(), "ButtonClass");
		return;
	}

	for (auto& Option : InOptions)
	{
		UCommonButtonBase* NewButton = CreateWidget<UCommonButtonBase>(this, ButtonClass);

		if (!ensure(NewButton))
		{
			SB::AutoSettings::Errors::LogWidgetCreationFailed(GetClass(), "RadioButton");
			continue;
		}

		UE_LOGFMT(LogAutoSettings, VeryVerbose, "{0}: Adding button {1}", GetDebugName(), Option.Value);
		ButtonValueMap.Add(NewButton, Option.Value);
		ButtonGroup->AddWidget(NewButton);

		HandleButtonCreation(NewButton, Option);
	}
}

void UButtonGroupSettingWidget::HandleButtonCreation_Implementation(UCommonButtonBase* NewButton, const FSettingOption& Option)
{
}

void UButtonGroupSettingWidget::HandleButtonRemoval_Implementation(UCommonButtonBase* NewButton)
{
}

void UButtonGroupSettingWidget::UpdateSelection_Implementation(const FString& Value)
{
	Super::UpdateSelection_Implementation(Value);

	TArray<FString> ValueArray;
	ButtonValueMap.GenerateValueArray(ValueArray);

	int32 Index = ValueArray.Find(Value);

	if (Index == INDEX_NONE)
	{
		UE_LOGFMT(LogAutoSettings, Warning, "{0}: Could not find option for value '{1}', ignoring", GetDebugName(), Value);
		return;
	}

	ButtonGroup->SelectButtonAtIndex(Index);
}

void UButtonGroupSettingWidget::NativeDestruct()
{
	UE_LOGFMT(LogAutoSettings, VeryVerbose, "{0}: NativeDestruct", GetDebugName());

	RemoveAllButtons();

	Super::NativeDestruct();
}

void UButtonGroupSettingWidget::RemoveAllButtons()
{
	UE_LOGFMT(LogAutoSettings, VeryVerbose, "{0}: Removing all buttons", GetDebugName());

	if (ButtonGroup)
	{
		ButtonGroup->RemoveAll();
	}

	for (auto& Pair : ButtonValueMap)
	{
		Pair.Key->RemoveFromParent();
	}

	ButtonValueMap.Reset();
}