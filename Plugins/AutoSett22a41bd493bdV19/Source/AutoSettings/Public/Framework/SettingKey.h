// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/TypeHash.h"
#include "Templates/SubclassOf.h"
#include "SettingKey.generated.h"

class USettingType;

/**
 * Element that a Setting Key is composed of
 */
USTRUCT(BlueprintType)
struct AUTOSETTINGS_API FSettingKeyFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Settings")
	FName Name;

	FSettingKeyFragment() = default;

	FSettingKeyFragment(const FName InName)
		: Name(InName) {}

	friend bool operator==(const FSettingKeyFragment& Lhs, const FSettingKeyFragment& RHS)
	{
		return Lhs.Name == RHS.Name;
	}

	friend bool operator!=(const FSettingKeyFragment& Lhs, const FSettingKeyFragment& RHS)
	{
		return !(Lhs == RHS);
	}

	friend uint32 GetTypeHash(const FSettingKeyFragment& Arg)
	{
		return GetTypeHash(Arg.Name);
	}

	bool IsNone() const { return Name.IsNone(); }
};

/**
 * A setting key is a hierarchical sequence of fragments used to identify or partially identify a setting.
 *
 * Keys are built by combining fragments from multiple sources:
 * - Registry base key (e.g., "Global" or "Player:1")
 * - Setting type hierarchy (e.g., "Graphics:ShadowQuality")
 * - Optional extensions (e.g., Slot "Jump:0" for input mappings)
 *
 * Example full key: ["Global", "Graphics", "ShadowQuality"]
 * This creates a colon-separated string: "Global:Graphics:ShadowQuality"
 */
USTRUCT(BlueprintType)
struct AUTOSETTINGS_API FSettingKey
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Settings")
	TArray<FSettingKeyFragment> Fragments;

	FSettingKey();
	FSettingKey(const FString& FullName);

	FString ToString() const;
	FName ToFullName() const;

	bool IsChildOf(const FSettingKey& Parent) const;

	bool EndsWith(const FSettingKey& Other) const;

	FSettingKeyFragment GetLastFragment(int32 IndexFromEnd = 0) const
	{
		return Fragments.Last(IndexFromEnd);
	}

	bool IsValid() const;

	bool operator==(const FSettingKey& Other) const
	{
		return Fragments == Other.Fragments;
	}

	bool operator!=(const FSettingKey& Other) const
	{
		return !(*this == Other);
	}

	void operator+=(const FSettingKey& Other)
	{
		Fragments.Append(Other.Fragments);
	}

	void operator+=(const FSettingKeyFragment& Fragment)
	{
		Fragments.Add(Fragment);
	}

	friend uint32 GetTypeHash(const FSettingKey& Key)
	{
		uint32 Hash = 0;
		for (FSettingKeyFragment Name : Key.Fragments)
		{
			Hash = HashCombine(Hash, GetTypeHash(Name));
		}
		return Hash;
	}

	void MigrateDeprecatedProperties()
	{
		if (!KeyNames.IsEmpty())
		{
			for (FName Name : KeyNames)
			{
				Fragments.Add(Name);
			}
			KeyNames.Empty();
		}
	}

private:
	UPROPERTY()
	TArray<FName> KeyNames;
};

/**
 * A complete setting key including the registry's base fragment, and all ancestors.
 * This key can be used to uniquely identify a setting value.
 *
 * Example: FFullSettingKey with fragments ["Global", "Graphics", "ShadowQuality"]
 * uniquely identifies the shadow quality setting in the global registry.
 */
USTRUCT(BlueprintType)
struct AUTOSETTINGS_API FFullSettingKey : public FSettingKey
{
	GENERATED_BODY()

	FFullSettingKey() = default;

	// Preserve the semantic distinction between full and partial keys while reusing the base parser.
	explicit FFullSettingKey(const FString& FullName)
		: FSettingKey(FullName)
	{
	}

	// Allow callers to promote a generic key to a full key without manually copying fragments at each call site.
	explicit FFullSettingKey(const FSettingKey& Other)
	{
		Fragments = Other.Fragments;
	}
};

/**
 * An incomplete setting key that requires additional fragments to uniquely identify a setting.
 *
 * Used for:
 * - Widget key extensions (combined with setting type to create full key)
 * - Setting type partial keys (combined with registry base key)
 *
 * Example: FPartialSettingKey with fragments ["Graphics", "ShadowQuality"]
 * needs a registry base key like "Global" to become a complete identifier.
 */
USTRUCT(BlueprintType)
struct AUTOSETTINGS_API FPartialSettingKey : public FSettingKey
{
	GENERATED_BODY()

	FPartialSettingKey() = default;

	// Preserve the semantic distinction between full and partial keys while reusing the base parser.
	explicit FPartialSettingKey(const FString& FullName)
		: FSettingKey(FullName)
	{
	}

	// Allow callers to treat a generic key as partial without repeating fragment-copy boilerplate.
	explicit FPartialSettingKey(const FSettingKey& Other)
	{
		Fragments = Other.Fragments;
	}
};

/**
 * Reusable reference to a setting key, deriving it from SettingType + SubKey when a type is set,
 * or using SubKey directly otherwise.
 */
USTRUCT(BlueprintType)
struct AUTOSETTINGS_API FSettingSpecifier
{
	GENERATED_BODY()

	/** Setting type used to derive the key when set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Settings", meta = (AllowAbstract = false))
	TSubclassOf<USettingType> SettingType;

	/**
	 * Key to use directly when SettingType is not set, or the extension appended when SettingType is set.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Settings")
	FPartialSettingKey SubKey;

	bool UsesSettingType() const;
	bool IsSet() const;
	FPartialSettingKey ResolveKey() const;
	FString ToString() const;
};

namespace SB::AutoSettings::SettingKeys
{
	/**
	 * Create a partial setting key from a setting type and optional sub-key (C++ version)
	 *
	 * @param SettingType - Class of the setting type
	 * @param SubKey - Optional partial key extension to identify the specific setting
	 * @return Partial setting key that can be used with registry methods
	 */
	AUTOSETTINGS_API FPartialSettingKey MakeSettingKey(const TSubclassOf<::USettingType>& SettingType, const FPartialSettingKey& SubKey = FPartialSettingKey());
} // namespace SB::AutoSettings::SettingKeys
