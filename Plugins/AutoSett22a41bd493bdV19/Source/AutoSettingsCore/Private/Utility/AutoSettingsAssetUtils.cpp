// Copyright Sam Bonifacio. All Rights Reserved.

#include "Utility/AutoSettingsAssetUtils.h"
#include "Engine/AssetManager.h"
#include "UObject/UObjectHash.h"

/**
 * Discovers and loads all assets of a specific type using Unreal's Asset Manager.
 * Handles bulk loading and supports converting UClass assets to their default objects.
 */
TArray<UObject*> FAutoSettingsAssetUtils::LoadAndGetAllAssets(const FPrimaryAssetType& AssetType, bool bGetClassDefaults)
{
	check(UAssetManager::IsInitialized());
	check(UAssetManager::Get().HasInitialScanCompleted());

	// Discover all Primary Asset IDs of the requested type
	// Asset Manager maintains an indexed registry of all primary assets in the project
	TArray<FPrimaryAssetId> AssetIds;
	UAssetManager::Get().GetPrimaryAssetIdList(AssetType, AssetIds);

	if (AssetIds.Num() == 0)
	{
		// Early exit: No assets of this type exist in the project
		return {};
	}

	// Bulk load all discovered assets (if not already in memory)
	// LoadPrimaryAssetsWithType returns null handle if assets are already loaded
	if (const auto Handle = UAssetManager::Get().LoadPrimaryAssetsWithType(AssetType))
	{
		// Synchronously wait for all assets to finish loading
		// This ensures all setting assets are available before proceeding
		Handle->WaitUntilComplete();
	}

	// Retrieve all loaded object instances
	// This includes both previously loaded and newly loaded assets
	TArray<UObject*> Assets;
	UAssetManager::Get().GetPrimaryAssetObjectList(AssetType, Assets);

	// Convert UClass objects to their Class Default Objects (CDO)
	// Setting classes need their default object instances for configuration data
	TArray<UObject*> FinalAssets;
	for (UObject* Asset : Assets)
	{
		if (const UClass* Class = Cast<UClass>(Asset); bGetClassDefaults && Class)
		{
			// Use CDO to access default property values defined in Blueprint/C++
			FinalAssets.Add(Class->GetDefaultObject());
		}
		else
		{
			// Asset instance can be used directly
			FinalAssets.Add(Asset);
		}
	}
	return FinalAssets;
}

TArray<UClass*> FAutoSettingsAssetUtils::GetLoadedNativeDerivedClasses(const UClass* BaseClass)
{
	if (!ensure(BaseClass))
	{
		return {};
	}

	// Unreal reflection can only enumerate native classes from Unreal modules that are loaded in this process.
	TArray<UClass*> DerivedClasses;
	GetDerivedClasses(BaseClass, DerivedClasses, true);

	// Keep only concrete reflected C++ classes so callers can safely read class defaults or instantiate them.
	DerivedClasses.RemoveAll([](const UClass* Class)
		{
			return !Class
				|| !Class->HasAnyClassFlags(CLASS_Native)
				|| Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists);
		});

	return DerivedClasses;
}

TArray<UClass*> FAutoSettingsAssetUtils::LoadAndGetAllClasses(const FPrimaryAssetType& AssetType, const UClass* BaseClass)
{
	if (!ensure(BaseClass))
	{
		return {};
	}

	// Primary asset discovery owns content-defined classes such as Blueprint-generated setting and console-variable classes.
	TArray<UClass*> Classes;
	for (UClass* AssetClass : LoadAndGetAllAssets<UClass>(AssetType, false))
	{
		Classes.AddUnique(AssetClass);
	}

	// Reflected native classes are not Asset Manager assets, so they are merged into the same class list here.
	for (UClass* NativeClass : GetLoadedNativeDerivedClasses(BaseClass))
	{
		Classes.AddUnique(NativeClass);
	}

	// Callers may instantiate these classes or read their CDOs, so only concrete descendants are exposed.
	Classes.RemoveAll([BaseClass](const UClass* Class)
		{
			return !Class
				|| !Class->IsChildOf(BaseClass)
				|| Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists);
		});

	return Classes;
}