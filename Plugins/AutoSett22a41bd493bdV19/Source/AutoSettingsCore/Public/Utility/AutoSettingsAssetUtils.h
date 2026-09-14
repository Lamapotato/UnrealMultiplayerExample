// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Class.h"

struct FPrimaryAssetType;

/**
 * Utility functions for loading and managing AutoSettings assets.
 *
 * Provides helper functions for bulk asset loading operations using the Asset Manager system.
 */
class AUTOSETTINGSCORE_API FAutoSettingsAssetUtils
{
public:
	/** Load all assets of a specific type using the Asset Manager */
	static TArray<UObject*> LoadAndGetAllAssets(const FPrimaryAssetType& AssetType, bool bGetClassDefaults = true);

	/** Template version that automatically casts loaded assets to the specified type */
	template <typename T>
	static TArray<T*> LoadAndGetAllAssets(const FPrimaryAssetType& AssetType, bool bGetClassDefaults = true)
	{
		TArray<UObject*> Assets = LoadAndGetAllAssets(AssetType, bGetClassDefaults);

		TArray<T*> AssetObjects;
		for (UObject* Asset : Assets)
		{
			T* TypedAssetObject = Cast<T>(Asset);
			if (!ensure(TypedAssetObject))
			{
				continue;
			}
			AssetObjects.Add(TypedAssetObject);
		}

		return AssetObjects;
	}

	/** Returns concrete classes from primary assets and loaded reflected native subclasses. */
	static TArray<UClass*> LoadAndGetAllClasses(const FPrimaryAssetType& AssetType, const UClass* BaseClass);

	/** Template version that loads primary asset classes and reflected native subclasses for the requested type. */
	template <typename T>
	static TArray<UClass*> LoadAndGetAllClasses(const FPrimaryAssetType& AssetType)
	{
		return LoadAndGetAllClasses(AssetType, T::StaticClass());
	}

	/** Returns class default objects for primary asset classes and loaded reflected native subclasses. */
	template <typename T>
	static TArray<T*> LoadAndGetAllClassDefaults(const FPrimaryAssetType& AssetType)
	{
		TArray<T*> ClassDefaults;
		for (UClass* Class : LoadAndGetAllClasses<T>(AssetType))
		{
			T* ClassDefault = Class->GetDefaultObject<T>();
			if (!ensure(ClassDefault))
			{
				continue;
			}
			ClassDefaults.Add(ClassDefault);
		}
		return ClassDefaults;
	}

private:
	/** Returns concrete native subclasses that are already loaded in reflected Unreal modules. */
	static TArray<UClass*> GetLoadedNativeDerivedClasses(const UClass* BaseClass);
};
