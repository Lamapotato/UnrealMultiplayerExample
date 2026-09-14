// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MappingIssueFunctions.generated.h"

/**
 *
 */
UCLASS()
class AUTOSETTINGSINPUT_API UMappingIssueFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Auto Settings|Input")
	static bool CanUnbindPreviousMappings(const FMappingAttemptInfo& Issues);
};