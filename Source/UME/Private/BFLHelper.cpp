// Fill out your copyright notice in the Description page of Project Settings.


#include "BFLHelper.h"

TArray<AActor*> UBFLHelper::SortActorsByDistance(const TArray<AActor*>& Actors, FVector Location)
{
    TArray<AActor*> SortedActors = Actors;

    SortedActors.RemoveAllSwap([](AActor* A) { return !IsValid(A); });

    SortedActors.Sort([Location](const AActor& A, const AActor& B) {
        const float distanceA = FVector::Distance(A.GetActorLocation(), Location);
        const float distanceB = FVector::Distance(B.GetActorLocation(), Location);
        return distanceA < distanceB;
        });

    return SortedActors;
}

int UBFLHelper::GetWeightedRandomIndex(const TArray<float>& Weights)
{
    // Check if the array is empty
    if (Weights.Num() == 0)
    {
        return -1;
    }

    // Calculate total sum of all weights
    float TotalWeight = 0.0f;
    for (const float Weight : Weights)
    {
        // Ensure weights are non-negative
        if (Weight < 0.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Negative weight found, clamping to 0"));
            continue;
        }
        TotalWeight += Weight;
    }

    // Check if total weight is 0 (all weights are zero)
    if (TotalWeight <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Total weight is zero or negative, returning random index"));
        return FMath::RandRange(0, Weights.Num() - 1);
    }

    // Generate a random number between 0 and TotalWeight
    const float RandomValue = FMath::FRandRange(0.0f, TotalWeight);

    // Find the index where the random value falls
    float CumulativeWeight = 0.0f;
    for (int32 Index = 0; Index < Weights.Num(); ++Index)
    {
        if (Weights[Index] < 0.0f)
        {
            continue; // Skip negative weights
        }

        CumulativeWeight += Weights[Index];
        if (RandomValue <= CumulativeWeight)
        {
            return Index;
        }
    }

    // Fallback (should never reach here if weights are valid)
    return Weights.Num() - 1;
}
