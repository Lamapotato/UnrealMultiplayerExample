// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "Scalability.h"
#include "AutoSettingsScalabilitySubsystem.generated.h"

/**
 * Subsystem that manages hardware benchmarking and scalability quality level caching
 * Provides both C++ and Blueprint access to benchmark results for use by default providers
 */
UCLASS()
class AUTOSETTINGS_API UAutoSettingsScalabilitySubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Run hardware benchmark and cache the results
	 * Subsequent calls will update the cached results
	 *
	 * @param WorkScale Benchmark work scale - higher values take longer but are more accurate (default: 10)
	 * @param CPUMultiplier CPU benchmark multiplier for adjusting CPU-bound test sensitivity (default: 1.0)
	 * @param GPUMultiplier GPU benchmark multiplier for adjusting GPU-bound test sensitivity (default: 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Settings|Scalability")
	void RunBenchmark(int32 WorkScale = 10, float CPUMultiplier = 1.0f, float GPUMultiplier = 1.0f);

	/**
	 * Check if benchmark results are available
	 *
	 * @return True if benchmark has been run and results are cached
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Auto Settings|Scalability")
	bool IsBenchmarkDataAvailable() const;

	/**
	 * Clear cached benchmark results
	 * Next call to get results will return false until a new benchmark is run
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Settings|Scalability")
	void ClearCachedResults();

	/**
	 * Set all scalability settings using benchmarked quality levels
	 * Runs benchmark first if no cached results are available
	 *
	 * @param bForceRunBenchmark If true, runs a new benchmark even if cached results exist
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Settings|Scalability")
	void SetQualityFromBenchmark(bool bForceRunBenchmark = false);

	/**
	 * Set all scalability settings to a specific preset quality level
	 *
	 * @param QualityLevel The quality level to apply (0=Low, 1=Medium, 2=High, 3=Epic/Ultra)
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Settings|Scalability")
	void SetQualityFromPreset(int32 QualityLevel);

	/**
	 * Get cached benchmark results for C++ access
	 *
	 * @return Optional containing quality levels if available, empty if no benchmark has been run
	 */
	TOptional<Scalability::FQualityLevels> GetCachedResults() const;

private:
	/**
	 * Set quality levels to both the scalability system and GameUserSettings
	 *
	 * @param QualityLevels The quality levels to set
	 */
	void SetQualityLevels(const Scalability::FQualityLevels& QualityLevels);

	/** Cached benchmark results */
	TOptional<Scalability::FQualityLevels> CachedResults;
};