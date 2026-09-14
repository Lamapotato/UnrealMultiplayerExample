// Copyright Sam Bonifacio. All Rights Reserved.

#include "Widgets/SettingControls/SelectSettingWidget.h"

void USelectSettingWidget::RegenerateOptions()
{
	DispatchUpdateOptions(GetCombinedOptions());
}

void USelectSettingWidget::NativePreConstruct()
{
	// Update options before calling super, which updates the selection
	// The options need to exist before the selection will work
	DispatchUpdateOptions(GetCombinedOptions());
	Super::NativePreConstruct();
}

void USelectSettingWidget::UpdateOptions_Implementation(const TArray<FSettingOption>& InOptions) {}

bool USelectSettingWidget::HasOptionWithValue(FString Value) const
{
	return GetCombinedOptions().ContainsByPredicate([Value](const FSettingOption& Option)
		{
			return Option.Value == Value;
		});
}

bool USelectSettingWidget::HasOptionWithLabel(FText Label) const
{
	return GetCombinedOptions().ContainsByPredicate([Label](const FSettingOption& Option)
		{
			return Option.Label.EqualTo(Label);
		});
}

FSettingOption USelectSettingWidget::GetOptionByValue(FString Value) const
{
	for (FSettingOption Option : GetCombinedOptions())
	{
		if (Option.Value == Value)
		{
			return Option;
		}
	}

	return FSettingOption();
}

FSettingOption USelectSettingWidget::GetOptionByLabel(FText Label) const
{
	for (FSettingOption Option : GetCombinedOptions())
	{
		if (Option.Label.EqualTo(Label))
		{
			return Option;
		}
	}

	return FSettingOption();
}

void USelectSettingWidget::PostLoad()
{
	Super::PostLoad();

	if (OptionFactory_DEPRECATED != nullptr)
	{
		OptionSource = NewObject<USettingOptionSource>(this, OptionFactory_DEPRECATED);
		OptionFactory_DEPRECATED = {};
	}
}

void USelectSettingWidget::PostInitProperties()
{
	Super::PostInitProperties();

	if (OptionFactory_DEPRECATED != nullptr)
	{
		OptionSource = NewObject<USettingOptionSource>(this, OptionFactory_DEPRECATED);
		OptionFactory_DEPRECATED = {};
	}
}

TArray<FSettingOption> USelectSettingWidget::GetCombinedOptions() const
{
	TArray<FSettingOption> ResultOptions = Options;

	// Replace empty options with index
	for (int i = 0; i < ResultOptions.Num(); i++)
	{
		FSettingOption& Option = ResultOptions[i];
		if (Option.Label.IsEmpty())
		{
			Option.Label = FText::FromString(FString::FromInt(i));
		}
		if (Option.Value.IsEmpty())
		{
			Option.Value = FString::FromInt(i);
		}
	}

	if (OptionSource)
	{
		ResultOptions.Append(OptionSource->GetOptions());
	}

	return ResultOptions;
}

void USelectSettingWidget::DispatchUpdateOptions(const TArray<FSettingOption>& InOptions)
{
	bUpdatingSettingOptions = true;
	UpdateOptions(InOptions);
	bUpdatingSettingOptions = false;
}