// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BFLHelper.generated.h"

/**
 * 
 */
UCLASS()
class UME_API UBFLHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static TArray<AActor*> SortActorsByDistance(const TArray<AActor*>& Actors, FVector Location);
	
};
