// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "SettingOption.h"
#include "SettingOptionSource.generated.h"

/**
 * Base class for an object providing a set of options for a setting
 */
UCLASS(abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class AUTOSETTINGS_API USettingOptionSource : public UObject
{
	GENERATED_BODY()

public:
	// Override to construct options dynamically
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Setting Option Source")
	TArray<FSettingOption> GetOptions() const;
};