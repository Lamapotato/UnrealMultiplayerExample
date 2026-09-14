// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetData.h"
#include "EnhancedActionKeyMapping.h"
#include "AutoSettingsInputDataCache.generated.h"

/**
 * Cache for input mapping data used by AutoSettings that doesn't change at runtime
 * Eliminates repeated calls and searches for performance optimization
 */
UCLASS()
class UAutoSettingsInputDataCache : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Get cache instance from world context
	 * @param World World context to access subsystem cache
	 * @return Cache instance or nullptr if not available
	 */
	static UAutoSettingsInputDataCache* Get(const UWorld* World);

	/**
	 * Get cached Input Mapping Context assets
	 * @return Array of Input Mapping Context asset data, or empty array if not cached
	 */
	const TArray<FAssetData>& GetInputContextAssets() const;

	/**
	 * Store Input Mapping Context assets in cache
	 * @param Assets Array of asset data to cache
	 */
	void PutInputContextAssets(const TArray<FAssetData>& Assets);

	/**
	 * Get cached mapping result if it exists
	 * @param PlayerMappingName Name of the mapping to find
	 * @return Pointer to cached mapping, or nullptr if not cached
	 */
	const FEnhancedActionKeyMapping* GetMapping(FName PlayerMappingName) const;

	/**
	 * Store a mapping in the cache
	 * @param PlayerMappingName Name of the mapping
	 * @param Mapping The mapping data to cache
	 */
	void PutMapping(FName PlayerMappingName, const FEnhancedActionKeyMapping& Mapping);

private:
	/** Cache for Asset Registry scan results */
	UPROPERTY(Transient)
	TArray<FAssetData> CachedAssets;

	/** Cache for mapping name to mapping data lookups */
	UPROPERTY(Transient)
	TMap<FName, FEnhancedActionKeyMapping> CachedMappings;
};