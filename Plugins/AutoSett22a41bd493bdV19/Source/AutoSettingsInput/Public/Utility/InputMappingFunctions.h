// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InputMappingFunctions.generated.h"

struct FEnhancedActionKeyMapping;

/**
 *
 */
UCLASS()
class AUTOSETTINGSINPUT_API UInputMappingFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Returns the internal FName associated with the key
	 * This is the ID of the key, not the display name
	 */
	UFUNCTION(BlueprintPure, Category = "Auto Settings|Input")
	static FName GetKeyName(const FKey& Key);

	UFUNCTION(BlueprintPure, Category = "Auto Settings|Input")
	static FText GetMappingDisplayName(const FEnhancedActionKeyMapping& Mapping);
};
