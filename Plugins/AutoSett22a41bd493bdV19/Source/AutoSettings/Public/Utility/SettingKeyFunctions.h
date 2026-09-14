// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Kismet/BlueprintFunctionLibrary.h"
#include "Framework/SettingKey.h"
#include "SettingKeyFunctions.generated.h"

UCLASS()
class AUTOSETTINGS_API USettingKeyFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, meta = (Category = "Setting Key"))
	static FSettingKey ParseSettingKey(const FString& String);

	UFUNCTION(BlueprintPure, meta = (Category = "Setting Key"))
	static FPartialSettingKey ParsePartialSettingKey(const FString& String);

	UFUNCTION(BlueprintPure, meta = (Category = "Setting Key"))
	static FFullSettingKey ParseFullSettingKey(const FString& String);

	UFUNCTION(BlueprintPure, meta = (Category = "Setting Key"))
	static FFullSettingKey ConvertToFullSettingKey(const FSettingKey& SettingKey);

	UFUNCTION(BlueprintPure, meta = (Category = "Setting Key"))
	static FPartialSettingKey ConvertToPartialSettingKey(const FSettingKey& SettingKey);

	/**
	 * Create a partial setting key from a setting type and optional sub-key
	 * This key can then be used with the registry's SetSetting method
	 *
	 * @param SettingType - Class of the setting type
	 * @param SubKey - Optional partial key extension to identify the specific setting
	 * @return Partial setting key that can be used with registry methods
	 */
	UFUNCTION(BlueprintPure, meta = (Category = "Setting Key", AutoCreateRefTerm = "SettingType,SubKey"))
	static FPartialSettingKey MakeSettingKey(UPARAM(meta = (AllowAbstract = false))
												 const TSubclassOf<USettingType>& SettingType,
		const FPartialSettingKey& SubKey = FPartialSettingKey());
};