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

	UFUNCTION(BlueprintCallable)
	static int GetWeightedRandomIndex(const TArray<float>& Weights);

    /**
     * Returns all save slot names found in the project's SaveGames directory.
     * File extensions (.sav) are stripped from the returned names.
     */
    UFUNCTION(BlueprintCallable, Category = "SaveSystem")
    static TArray<FString> GetAllSaveSlots();
};
