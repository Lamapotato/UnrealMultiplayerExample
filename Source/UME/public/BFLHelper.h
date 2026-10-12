// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BFLHelper.generated.h"

/**
 * Результат одного замера бенчмарка.
 * Помечен как USTRUCT, чтобы Blueprint мог читать поля.
 */
USTRUCT(BlueprintType)
struct FContainerBenchmarkResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    FString ContainerName;

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    FString OperationName;

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    float TimeSeconds = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Benchmark")
    int64 MemoryBytes = 0;

    FContainerBenchmarkResult() {}

    FContainerBenchmarkResult(const FString& InContainer, const FString& InOperation, double InTime, SIZE_T InMemory)
        : ContainerName(InContainer)
        , OperationName(InOperation)
        , TimeSeconds(static_cast<float>(InTime))
        , MemoryBytes(static_cast<int64>(InMemory))
    {
    }
};


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

public:
    /**
     * Запустить полный бенчмарк для N элементов.
     * @param N - количество элементов
     * @return массив результатов по всем операциям
     */
    UFUNCTION(BlueprintCallable, Category = "Benchmark", meta = (DisplayName = "Run Container Benchmark"))
    static TArray<FContainerBenchmarkResult> Run(int32 N);

    /**
     * Вывести результаты бенчмарка в лог UE.
     * @param Results - результаты, полученные из Run()
     */
    UFUNCTION(BlueprintCallable, Category = "Benchmark", meta = (DisplayName = "Print Benchmark Results"))
    static void PrintResults(const TArray<FContainerBenchmarkResult>& Results);

private:
    static void BenchmarkTArray(int32 N, TArray<FContainerBenchmarkResult>& OutResults);
    static void BenchmarkTMap(int32 N, TArray<FContainerBenchmarkResult>& OutResults);
    static void BenchmarkTList(int32 N, TArray<FContainerBenchmarkResult>& OutResults);
};
