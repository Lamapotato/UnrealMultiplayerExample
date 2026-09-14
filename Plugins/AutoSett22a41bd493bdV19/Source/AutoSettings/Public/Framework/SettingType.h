// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/ConfigSettingPersistenceStrategy.h"
#include "GameplayTagContainer.h"
#include "SettingTypes.h"

#include "SettingType.generated.h"

class USettingDefaultProvider;

/**
 * Defines the scope at which a setting is applied
 */
UENUM(BlueprintType)
enum class ESettingDomain : uint8
{
	/**
	 * Settings are applied globally for the game session
	 * For example, video and audio settings
	 */
	Global,
	/**
	 * Settings are applied per-player
	 * For example, input settings
	 */
	Player
};

/**
 * A Setting Type describes the functionality of one or more settings
 *
 * The SettingType class defines the shared behavior for settings that work the same way.
 * Multiple settings can use the same SettingType if they have the same application and
 * persistence behavior, differing only in their keys and values.
 *
 * For example, all console variable settings (graphics quality, audio volume, etc.) can
 * share the same SettingType that applies values to CVars, while all input mappings can
 * share a SettingType that handles Enhanced Input bindings. Setting types use strategy
 * objects to define how values are applied and persisted.
 */
UCLASS(Abstract, Blueprintable)
class AUTOSETTINGS_API USettingType : public UObject
{
	GENERATED_BODY()

public:
	/** Primary asset type name used for storing and loading setting assets */
	inline const static FName SettingPrimaryAssetTypeName = TEXT("Setting");

	/**
	 * The domain which this setting type will be registered under
	 * Controls whether the setting is applied globally or per-player
	 */
	UPROPERTY(EditAnywhere, Category = "General")
	ESettingDomain SettingDomain = ESettingDomain::Global;

	/**
	 * The key fragment appended to setting keys of this type
	 * Used to build the full setting key path for identification
	 */
	UPROPERTY(EditAnywhere, Category = "Key")
	FSettingKeyFragment KeyFragment;

	/**
	 * If not empty, child settings will be restricted to key fragments defined here
	 * This can be useful for security reasons to prevent arbitrary key/values
	 * from being loaded if they have been tampered with
	 */
	UPROPERTY(EditAnywhere, Category = "Hierarchy")
	TArray<FSettingKeyFragment> AllowedChildKeyFragments;

	/**
	 * Whether this setting type is enabled
	 * If false, settings of this type will not be loaded, applied, or saved
	 * Note: Child types will continue to work if they are enabled
	 */
	UPROPERTY(EditAnywhere, Category = "General")
	bool bEnabled = true;

	/**
	 * Strategy to handle how settings of this type are applied
	 * Defines the behavior for applying a setting value to the game
	 * (e.g., to a console variable, config file, or input system).
	 * Optional: if left null, the registry still owns the applied value but no automatic external sync occurs.
	 */
	UPROPERTY(EditAnywhere, Instanced, Category = "Strategies")
	class USettingBindingStrategy* BindingStrategy = nullptr;

	/**
	 * Strategy to handle how settings of this type are saved and loaded
	 * Defines where and how setting values are stored between sessions
	 */
	UPROPERTY(EditAnywhere, Instanced, NoClear, Category = "Strategies")
	class USettingPersistenceStrategy* PersistenceStrategy = nullptr;

	/**
	 * If true, override the default value of the setting using the DefaultValue below
	 */
	UPROPERTY(EditAnywhere, Category = "Defaults", meta = (InlineEditConditionToggle))
	bool bOverrideDefaultValue = false;

	/**
	 * Default value for the setting if bOverrideDefaultValue is true
	 * This is the initial value for the setting when it hasn't been changed
	 */
	UPROPERTY(EditAnywhere, Category = "Defaults", meta = (EditCondition = bOverrideDefaultValue))
	FString DefaultValue;

	/**
	 * Partial setting keys to mark as controlled by this setting
	 * This affects the order that settings are applied in when loaded, and is
	 * important to set when settings control or influence other settings
	 *
	 * Setting keys ending with a partial key here will be marked as controlled by this setting
	 * e.g. A controlled setting of "B:C" will apply to a setting with a full key of "A:B:C"
	 */
	UPROPERTY(EditAnywhere, Category = "Relationships")
	TArray<FPartialSettingKey> ControlledSettingKeys;

	/**
	 * Arbitrary gameplay tags this setting has, for use by your own project
	 * Can be used for categorization, filtering, or other custom behaviors
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata")
	FGameplayTagContainer SettingTags;

	/**
	 * Default value providers for this setting type.
	 * Providers are executed in array order during registry initialization.
	 */
	UPROPERTY(EditAnywhere, Instanced, Category = "Defaults",
		meta = (ShowTreeView))
	TArray<TObjectPtr<USettingDefaultProvider>> DefaultProviders;

	/**
	 * Constructor for the setting type
	 * @param ObjectInitializer - Standard object initializer
	 */
	USettingType(const FObjectInitializer& ObjectInitializer);

	/**
	 * Gets the current value of the setting
	 *
	 * @param Setting - Reference to the setting to get the value for
	 * @param bPreferSavedValue - If true, returns the saved value, otherwise returns the applied value
	 * @return The setting value as a string
	 */
	FString GetValue(const FSettingReference& Setting, bool bPreferSavedValue) const;

	/**
	 * Override to provide primary asset ID for the settings system
	 */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/**
	 * Called after the object is loaded
	 * Handles data migration from deprecated formats
	 */
	virtual void PostLoad() override;

	/**
	 * Gets the partial setting key using this object's fragment as the leaf
	 * and non-empty ancestor class-default fragments as the inherited path.
	 *
	 * @return Partial setting key that represents this setting type's key path
	 */
	FPartialSettingKey GetPartialSettingKey() const;

	/**
	 * Get all default values for this setting type from all sources
	 * Combines static default values and dynamic provider-generated defaults
	 * Returns complete SettingType keys by combining this type's base key with provider fragments
	 * (Registry will still need to prepend its base key to make them full keys)
	 *
	 * @return Map of complete SettingType keys to their default values as strings
	 */
	TMap<FPartialSettingKey, FString> GetDefaultValues() const;

private:
	friend class USettingRegistry;

	/**
	 * Applies the setting value to the game
	 * Delegates to the BindingStrategy to perform the actual application
	 * Only the SettingRegistry should call this to ensure proper scoping
	 *
	 * @param Event - Event containing the setting and value to apply
	 */
	void Apply(const FSetSettingEvent& Event) const;

	/**
	 * Saves the setting value for persistence
	 * Delegates to the PersistenceStrategy to perform the actual saving
	 * Only the SettingRegistry should call this to ensure proper scoping
	 *
	 * @param Event - Event containing the setting and value to save
	 */
	void Save(const FSetSettingEvent& Event) const;

	/** Deprecated: Old format for setting key */
	UPROPERTY()
	FName SettingKey_DEPRECATED;

	/** Deprecated: Old format for allowed child keys */
	UPROPERTY()
	TArray<FName> AllowedChildKeys_DEPRECATED;

	/** Deprecated: Old name for controlled setting keys */
	UPROPERTY()
	TArray<FPartialSettingKey> Dependencies_DEPRECATED;
};
