// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/Texture.h"
#include "InputCoreTypes.h"
#include "AutoSettingsInputConfig.generated.h"

class UInputMappingContext;
class UKeyIconSource;

// Set of key icons
USTRUCT()
struct FKeyIconSet
{
	GENERATED_BODY()

	/**
	 * Tags to identify this icon set
	 * May include platform such as "XBox" or "PlayStation" and/or variations such as "Small" or "Large"
	 */
	UPROPERTY(config, EditAnywhere, Category = "Key Icon Set")
	FGameplayTagContainer Tags;

	/** Icons defined for different keys */
	UPROPERTY(config, EditAnywhere, Category = "Key Icon Set")
	TMap<FKey, TSoftObjectPtr<UTexture>> IconMap;

	/** Return the icon texture for the given key */
	UTexture* GetIcon(const FKey& Key) const;
};

/** Represents pairing of a button key and a scale, used to associate buttons with an axis */
USTRUCT()
struct FKeyScale
{
	GENERATED_BODY()

	/** Button key */
	UPROPERTY(config, EditAnywhere, Category = "Key Scale")
	FKey Key;

	/** Scale of the axis that the button key should be associated with */
	UPROPERTY(config, EditAnywhere, Category = "Key Scale")
	float Scale;

	FKeyScale()
		: Scale(1.f)
	{
	}

	FKeyScale(FKey InKey)
		: Key(InKey)
		, Scale(1.f)
	{
	}

	FKeyScale(FKey InKey, float InScale)
		: Key(InKey)
		, Scale(InScale)
	{
	}
};

/**
 * Determines association between axis and buttons
 * Used to tell the system that for example "Gamepad Left Stick Y" axis with a scale of -1 should be associated with "Gamepad Left Stick Down"
 */
USTRUCT()
struct FAxisAssociation
{
	GENERATED_BODY()

	FAxisAssociation()
	{
	}

	FAxisAssociation(FKey InAxisKey, TArray<FKeyScale> InButtonKeys)
		: AxisKey(InAxisKey)
		, ButtonKeys(InButtonKeys)
	{
	}

	/** Axis key for this association */
	UPROPERTY(config, EditAnywhere, Category = "Axis Association")
	FKey AxisKey;

	/** Button keys to associate axis with */
	UPROPERTY(config, EditAnywhere, Category = "Axis Association", meta = (TitleProperty = "Key"))
	TArray<FKeyScale> ButtonKeys;
};

/** Defines friendly user-facing name text for a key */
USTRUCT()
struct FKeyFriendlyName
{
	GENERATED_BODY()

	/** Specified key */
	UPROPERTY(config, EditAnywhere, Category = "Key Friendly Name")
	FKey Key;

	/** User-facing friendly text name */
	UPROPERTY(config, EditAnywhere, Category = "Key Friendly Name")
	FText FriendlyName;
};

/**
 * Configuration object for Auto Settings Input
 */
UCLASS(config = Game, defaultconfig)
class AUTOSETTINGSINPUT_API UAutoSettingsInputConfig : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UAutoSettingsInputConfig();

	/**
	 * By default, Unreal will invert the right thumbstick Y axis so that up is -1 and down is 1.
	 * If true, Auto Settings will run an input processor to flip it again to un-invert the value back to the original,
	 * for consistency with other thumbsticks.
	 * This is off by default, but it's recommended to turn it on.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "General")
	bool bUninvertGamepadRightStickYAxis = false;

	/**
	 * By default, we only detect mapping collisions within the same context.
	 * Add a Mapping Context here in order to detect mapping collisions with all other contexts.
	 */
	UPROPERTY(config, EditAnywhere, Category = "General")
	TArray<TSoftObjectPtr<UInputMappingContext>> GlobalMappingContexts;

	/**
	 * Class to use to provide icons for keys
	 */
	UPROPERTY(config, EditAnywhere, Category = "Display")
	TSoftClassPtr<UKeyIconSource> KeyIconSource;

	/**
	 * Friendly key names to use if an icon is not available
	 * If a friendly name is not specified for a key, the default FKey GetDisplayName will be used
	 */
	UPROPERTY(config, EditAnywhere, Category = "Display", meta = (TitleProperty = "Key"))
	TArray<FKeyFriendlyName> KeyFriendlyNames;


	/**
	 * Determines association between axes and buttons
	 * This is used during axis rebinding to determine which axis should be used when a key press is detected
	 * e.g. If the system detects the player has pressed "Gamepad Left Stick Down" key when binding "Move Backward",
	 * these associations could specify that the "Gamepad Left Stick Y" axis should be bound with a scale of -1
	 * otherwise the mapping will only have 2 key states (pressed and not pressed) rather than the full range of the axis
	 */
	UPROPERTY(config, EditAnywhere, Category = "Input Remapping", meta = (TitleProperty = "AxisKey"))
	TArray<FAxisAssociation> AxisAssociations;

	/** KeyIconSets is deprecated in v2, but we're keeping it around to support the "AutoSettings.MigrateKeyIcons" command */
	UPROPERTY(config, meta = (DeprecatedProperty))
	TArray<FKeyIconSet> KeyIconSets_DEPRECATED;

	const UKeyIconSource* GetKeyIconSource() const;

	/** Returns the icon texture for the given key and key icon tags */
	UTexture* GetIconForKey(FKey InKey, FGameplayTagContainer Tags, float AxisScale = 0.f) const;

	/** Returns the Friendly Name override for the key if available (specified in AutoSettings config) or falls back to the FKey DisplayName */
	FText GetKeyFriendlyName(FKey Key) const;

	/** Returns the deprecated config value used to seed widget-owned escape keys */
	const TArray<FKey>& GetDeprecatedBindingEscapeKeys() const;

	/** Returns the deprecated config value used to seed widget-owned mouse-axis capture distance */
	float GetDeprecatedMouseMoveCaptureDistance() const;

	/** Returns the deprecated config value used to seed widget-owned allowed keys */
	const TArray<FKey>& GetDeprecatedAllowedKeys() const;

	/** Returns the deprecated config value used to seed widget-owned disallowed keys */
	const TArray<FKey>& GetDeprecatedDisallowedKeys() const;

	/** Returns axis key for the given button key */
	FKeyScale GetAxisKey(FKey InButtonKey) const;

	/**
	 * Gets an axis button in any stored Axis Association
	 * e.g. turn AxisKey= "Gamepad Left Stick Y" AxisScale= -1 into "Gamepad Left Stick Down"
	 */
	FKey GetAxisButton(FKey AxisKey, float AxisScale) const;

	/** True if the given key is whitelisted (if applicable) and not blacklisted */
	bool IsKeyAllowed(FKey Key) const;

	virtual bool IsAxisKey(FKey Key) const;

private:
	/** Deprecated config default for input mapping widgets that have not authored their own escape keys */
	UPROPERTY(config, meta = (DeprecatedProperty))
	TArray<FKey> BindingEscapeKeys_DEPRECATED;

	/** Deprecated config default for widget key allow-list */
	UPROPERTY(config, meta = (DeprecatedProperty))
	TArray<FKey> AllowedKeys_DEPRECATED;

	/** Deprecated config default for widget key deny-list */
	UPROPERTY(config, meta = (DeprecatedProperty))
	TArray<FKey> DisallowedKeys_DEPRECATED;

	/** Deprecated config default for input mapping widgets that have not authored their own mouse-axis capture distance */
	UPROPERTY(config, meta = (DeprecatedProperty))
	float MouseMoveCaptureDistance_DEPRECATED = 80.f;

	/** Automatically generates axis associations based on key naming patterns */
	static TArray<FAxisAssociation> GenerateAxisAssociations();

	/** Returns the friendly name for the given key */
	UFUNCTION(BlueprintPure, Category = "Auto Settings Config", DisplayName = "Get Key Friendly Name")
	static FText GetKeyFriendlyNameStatic(FKey Key);

#if WITH_EDITOR
	virtual bool SupportsAutoRegistration() const override
	{
		return true;
	}
	virtual FText GetSectionText() const override;
	virtual FText GetSectionDescription() const override;
#endif
};
