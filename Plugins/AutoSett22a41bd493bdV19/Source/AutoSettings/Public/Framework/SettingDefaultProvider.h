// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/SettingKey.h"
#include "UObject/Object.h"
#include "SettingDefaultProvider.generated.h"

/**
 * Base class for providing dynamic default values for settings.
 *
 * Default providers are executed during AutoSettings initialization to determine appropriate
 * default values based on hardware, user preferences, or custom logic.
 *
 * Providers can be implemented in both C++ and Blueprint, and are configured on individual
 * SettingType assets. Multiple providers can exist per SettingType and are executed in the
 * order they appear in the DefaultProviders array.
 *
 * Providers return key fragments that are appended to their parent SettingType's base key.
 * If a generated setting doesn't exist, it will be automatically registered using the parent SettingType.
 */
UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, meta = (ShowTreeView))
class AUTOSETTINGS_API USettingDefaultProvider : public UObject
{
	GENERATED_BODY()

public:
	USettingDefaultProvider();

	/**
	 * Whether this provider is enabled and should be executed.
	 * Can be configured per-provider in Project Settings.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
	bool bEnabled = true;

	/**
	 * Override this function to provide default values for settings.
	 * Called during AutoSettings initialization if the provider is enabled.
	 *
	 * This method should return key fragments relative to the parent SettingType's base key.
	 * For example, if your SettingType has base key "Global:Video" and you return a fragment
	 * "sg.ShadowQuality", the final setting key will be "Global:Video:sg.ShadowQuality".
	 *
	 * If a setting with the generated full key doesn't exist, it will be automatically
	 * registered using the parent SettingType.
	 *
	 * @return Map of setting key fragments to their recommended default values as strings
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Auto Settings")
	TMap<FPartialSettingKey, FString> GetDefaultValues();
};