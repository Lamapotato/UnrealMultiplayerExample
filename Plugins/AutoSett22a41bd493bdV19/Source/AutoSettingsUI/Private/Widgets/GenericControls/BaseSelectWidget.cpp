// Copyright Sam Bonifacio. All Rights Reserved.

#include "Widgets/GenericControls/BaseSelectWidget.h"

void UBaseSelectWidget::SetOptions(TArray<FSettingOption> InOptions)
{
	Options = InOptions;
}

void UBaseSelectWidget::Select(FString Value)
{
	SelectValue(Value);
}

void UBaseSelectWidget::SelectValue(FString Value)
{
	const int32 Index = Options.IndexOfByPredicate([Value](FSettingOption Option)
		{
			return Option.Value == Value;
		});
	if (Index >= 0)
	{
		SelectIndex(Index);
	}
}

void UBaseSelectWidget::SelectIndex(int32 Index)
{
	if (!ensure(Options.IsValidIndex(Index)))
	{
		return;
	}

	CurrentIndex = Index;
	FSettingOption& SelectedOption = Options[CurrentIndex];
	OnSelectionChanged(SelectedOption);
	SelectionChangedEvent.Broadcast(SelectedOption.Value);
}

void UBaseSelectWidget::Next()
{
	if (HasValidNext())
	{
		if (CurrentIndex < Options.Num() - 1)
		{
			SelectIndex(CurrentIndex + 1);
		}
		else
		{
			SelectIndex(0);
		}
	}
}

void UBaseSelectWidget::Previous()
{
	if (HasValidPrevious())
	{
		if (CurrentIndex > 0)
		{
			SelectIndex(CurrentIndex - 1);
		}
		else
		{
			SelectIndex(Options.Num() - 1);
		}
	}
}

bool UBaseSelectWidget::HasValidNext() const
{
	return bAllowWrapping || CurrentIndex < Options.Num() - 1;
}

bool UBaseSelectWidget::HasValidPrevious() const
{
	return bAllowWrapping || CurrentIndex > 0;
}

FSettingOption UBaseSelectWidget::GetCurrentOption() const
{
	if (Options.IsValidIndex(CurrentIndex))
	{
		return Options[CurrentIndex];
	}
	return FSettingOption();
}

void UBaseSelectWidget::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();

	SetOptions(Options);
}

void UBaseSelectWidget::OnSelectionChanged_Implementation(FSettingOption SelectedOption)
{
}