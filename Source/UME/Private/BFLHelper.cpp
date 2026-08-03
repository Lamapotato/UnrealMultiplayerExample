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