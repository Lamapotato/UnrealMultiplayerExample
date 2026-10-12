// Fill out your copyright notice in the Description page of Project Settings.


#include "BFLHelper.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Containers/List.h"
#include "Misc/DateTime.h"
#include "Logging/LogMacros.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

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

TArray<FString> UBFLHelper::GetAllSaveSlots()
{
    TArray<FString> Result;

    // Build the path to the SaveGames directory
    const FString SaveDir = FPaths::ProjectSavedDir() / TEXT("SaveGames");

    // Find all .sav files in that directory
    TArray<FString> FoundFiles;
    IFileManager::Get().FindFiles(FoundFiles, *SaveDir, TEXT(".sav"));

    // Strip the extension from each file name
    for (const FString& File : FoundFiles)
    {
        Result.Add(File.LeftChop(4)); // Remove ".sav"
    }

    return Result;
}

TArray<FContainerBenchmarkResult> UBFLHelper::Run(int32 N)
{
    TArray<FContainerBenchmarkResult> Results;
    Results.Reserve(9);

    BenchmarkTArray(N, Results);
    BenchmarkTMap(N, Results);
    BenchmarkTList(N, Results);

    return Results;
}

void UBFLHelper::PrintResults(const TArray<FContainerBenchmarkResult>& Results)
{
    UE_LOG(LogTemp, Warning, TEXT("===================== BENCHMARK RESULTS ====================="));

    for (const FContainerBenchmarkResult& R : Results)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("%-8s | %-15s | Time: %10.6f sec | Memory: %10lld bytes"),
            *R.ContainerName,
            *R.OperationName,
            R.TimeSeconds,
            R.MemoryBytes);
    }

    UE_LOG(LogTemp, Warning, TEXT("============================================================="));
}

// ============================================================
//  TArray
// ============================================================

void UBFLHelper::BenchmarkTArray(int32 N, TArray<FContainerBenchmarkResult>& OutResults)
{
    const FString Name = TEXT("TArray");
    TArray<int32> TestArray;
    TestArray.Reserve(N);

    // --- Add ---
    {
        const double Start = FPlatformTime::Seconds();
        for (int32 i = 0; i < N; ++i)
        {
            TestArray.Add(i);
        }
        const double Elapsed = FPlatformTime::Seconds() - Start;
        OutResults.Emplace(Name, TEXT("Add"), Elapsed, TestArray.GetAllocatedSize());
    }

    // --- Find ---
    {
        volatile int32 Sink = 0;
        const double Start = FPlatformTime::Seconds();
        for (int32 i = 0; i < N; ++i)
        {
            if (TestArray.Find(i) != INDEX_NONE)
            {
                ++Sink;
            }
        }
        const double Elapsed = FPlatformTime::Seconds() - Start;
        OutResults.Emplace(Name, TEXT("Find"), Elapsed, 0);
    }

    // --- RemoveAt ---
    {
        const double Start = FPlatformTime::Seconds();
        for (int32 i = N - 1; i >= 0; --i)
        {
            TestArray.RemoveAt(i);
        }
        const double Elapsed = FPlatformTime::Seconds() - Start;
        OutResults.Emplace(Name, TEXT("RemoveAt"), Elapsed, TestArray.GetAllocatedSize());
    }
}

// ============================================================
//  TMap
// ============================================================

void UBFLHelper::BenchmarkTMap(int32 N, TArray<FContainerBenchmarkResult>& OutResults)
{
    const FString Name = TEXT("TMap");
    TMap<int32, FString> TestMap;
    TestMap.Reserve(N);

    // --- Add ---
    {
        const double Start = FPlatformTime::Seconds();
        for (int32 i = 0; i < N; ++i)
        {
            TestMap.Add(i, FString::FromInt(i));
        }
        const double Elapsed = FPlatformTime::Seconds() - Start;
        OutResults.Emplace(Name, TEXT("Add"), Elapsed, TestMap.GetAllocatedSize());
    }

    // --- Find ---
    {
        volatile int32 Sink = 0;
        const double Start = FPlatformTime::Seconds();
        for (int32 i = 0; i < N; ++i)
        {
            if (TestMap.Find(i) != nullptr)
            {
                ++Sink;
            }
        }
        const double Elapsed = FPlatformTime::Seconds() - Start;
        OutResults.Emplace(Name, TEXT("Find"), Elapsed, 0);
    }

    // --- Remove ---
    {
        const double Start = FPlatformTime::Seconds();
        for (int32 i = 0; i < N; ++i)
        {
            TestMap.Remove(i);
        }
        const double Elapsed = FPlatformTime::Seconds() - Start;
        OutResults.Emplace(Name, TEXT("Remove"), Elapsed, TestMap.GetAllocatedSize());
    }
}

// ============================================================
//  TList (односвязный список, управляется вручную)
// ============================================================

void UBFLHelper::BenchmarkTList(int32 N, TArray<FContainerBenchmarkResult>& OutResults)
{
    const FString Name = TEXT("TList");

    TList<int32>* Head = nullptr;
    TList<int32>* Tail = nullptr;

    // --- Build (аналог Add) ---
    {
        const double Start = FPlatformTime::Seconds();
        for (int32 i = 0; i < N; ++i)
        {
            TList<int32>* NewNode = new TList<int32>(i, nullptr);

            if (Tail)
            {
                Tail->Next = NewNode;
            }
            else
            {
                Head = NewNode;
            }
            Tail = NewNode;
        }
        const double Elapsed = FPlatformTime::Seconds() - Start;

        const SIZE_T Memory = static_cast<SIZE_T>(N) * sizeof(TList<int32>);
        OutResults.Emplace(Name, TEXT("Build"), Elapsed, Memory);
    }

    // --- Find (линейный поиск) ---
    {
        volatile int32 Sink = 0;
        const double Start = FPlatformTime::Seconds();
        for (int32 i = 0; i < N; ++i)
        {
            TList<int32>* Current = Head;
            while (Current)
            {
                if (Current->Element == i)
                {
                    ++Sink;
                    break;
                }
                Current = Current->Next;
            }
        }
        const double Elapsed = FPlatformTime::Seconds() - Start;
        OutResults.Emplace(Name, TEXT("Find (linear)"), Elapsed, 0);
    }

    // --- Delete all (аналог Remove) ---
    {
        const double Start = FPlatformTime::Seconds();
        TList<int32>* Current = Head;
        while (Current)
        {
            TList<int32>* Next = Current->Next;
            delete Current;
            Current = Next;
        }
        Head = nullptr;
        Tail = nullptr;
        const double Elapsed = FPlatformTime::Seconds() - Start;
        OutResults.Emplace(Name, TEXT("Delete all"), Elapsed, 0);
    }
}
