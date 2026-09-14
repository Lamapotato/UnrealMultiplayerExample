// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "AutoSettingsGameUserSettings.generated.h"

/**
 * Custom UGameUserSettings that prevents base class from resetting settings on fullscreen changes.
 */
UCLASS()
class AUTOSETTINGS_API UAutoSettingsGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

	virtual void ApplyNonResolutionSettings() override;
};