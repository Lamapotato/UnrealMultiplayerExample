// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "Misc/SettingOptionSource.h"
#include "ResolutionOptionSource.generated.h"

/**
 * Class for constructing a set of options for supported fullscreen resolutions
 */
UCLASS()
class AUTOSETTINGS_API UResolutionOptionSource : public USettingOptionSource
{
	GENERATED_BODY()

	// Construct options for supported resolutions
	virtual TArray<FSettingOption> GetOptions_Implementation() const override;
};