// Copyright Sam Bonifacio. All Rights Reserved.

#include "Utility/AutoSettingsInputDataCache.h"

#include "InputMappingContext.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "Core/AutoSettingsInputSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Utility/InputMappingUtils.h"

UAutoSettingsInputDataCache* UAutoSettingsInputDataCache::Get(const UWorld* World)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UAutoSettingsInputDataCache::Get);

	if (!World)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}

	UAutoSettingsInputSubsystem* Subsystem = GameInstance->GetSubsystem<UAutoSettingsInputSubsystem>();
	if (!Subsystem)
	{
		return nullptr;
	}

	return Subsystem->GetMappingCache();
}

const TArray<FAssetData>& UAutoSettingsInputDataCache::GetInputContextAssets() const
{
	// Simple cache retrieval - no computation
	return CachedAssets;
}

void UAutoSettingsInputDataCache::PutInputContextAssets(const TArray<FAssetData>& Assets)
{
	// Simple cache store
	CachedAssets = Assets;
}

const FEnhancedActionKeyMapping* UAutoSettingsInputDataCache::GetMapping(FName PlayerMappingName) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(InputMappingCache_GetMapping);

	// Simple cache lookup - return pointer if found, nullptr if not
	return CachedMappings.Find(PlayerMappingName);
}

void UAutoSettingsInputDataCache::PutMapping(FName PlayerMappingName, const FEnhancedActionKeyMapping& Mapping)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(InputMappingCache_PutMapping);

	// Simple cache store
	CachedMappings.Add(PlayerMappingName, Mapping);
}
