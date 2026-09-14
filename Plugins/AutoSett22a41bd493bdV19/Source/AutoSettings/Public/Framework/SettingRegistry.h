// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/SettingBinding.h"
#include "SettingType.h"
#include "SettingTypes.h"
#include "Tickable.h"
#include "UObject/Object.h"
#include "Templates/SubclassOf.h"
#include "SettingRegistry.generated.h"

class USettingControlRegistry;

/**
 * Specifies the initialization state of a setting registry
 */
UENUM()
enum class ESettingRegistryState : uint8
{
	/** Registry has not been initialized yet */
	Uninitialized,
	/** Registry is executing its atomic initialization transaction */
	Initializing,
	/** Registry is initialized and ready for use */
	Initialized,
	/** Registry has been deinitialized and should not accept new operations */
	Deinitialized
};

/**
 * Specifies how a setting value should be handled when setting it
 */
UENUM(BlueprintType)
enum class ESettingAction : uint8
{
	/** Apply the setting temporarily (lost on restart) */
	ApplyOnly,
	/** Apply and save the setting and any controlled settings (persisted between sessions) */
	ApplyAndSave
};

/**
 * Specifies which value to prefer when reading a setting
 */
UENUM(BlueprintType)
enum class ESettingValuePreference : uint8
{
	/** Use the currently applied value */
	Applied,
	/** Use the saved/persistent value */
	Saved
};

/** Describes why a registry applied value changed. */
UENUM(BlueprintType)
enum class ESettingAppliedValueChangeSource : uint8
{
	/** The value changed while the registry was establishing an initial value for a setting. */
	Initialization,
	/** The value changed because SetSetting was called directly on the registry. */
	SetSetting,
	/** The value changed because an external target system reported a new value. */
	TargetSync
};

UENUM()
enum class ETargetBindingState : uint8
{
	None,
	Pending,
	Unsupported,
	Bound
};

/** Event payload for registry-owned applied value changes. */
USTRUCT(BlueprintType)
struct FSettingAppliedValueChangedEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	FSettingReference Setting;

	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	FString OldValue;

	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	FString NewValue;

	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	ESettingAppliedValueChangeSource Source = ESettingAppliedValueChangeSource::SetSetting;
};

DECLARE_DYNAMIC_DELEGATE_TwoParams(FSettingAppliedValueCallback, const FSettingReference&, Setting, const FString&, Value);

/**
 * Represents data for a setting that is registered in the settings system
 *
 * This struct contains all information needed to identify, access, and
 * manage a specific setting instance within the registry, including its
 * key, type definition, default values, and dependency relationships.
 */
USTRUCT()
struct FRegisteredSetting
{
	GENERATED_BODY()

	/**
	 * The unique key for the setting
	 * This is a fully qualified path that uniquely identifies this setting
	 * in the format of hierarchical fragments (e.g., "Global:Video:Resolution")
	 */
	UPROPERTY()
	FFullSettingKey Key;

	/**
	 * The setting type that describes this setting
	 * References the asset that defines the behavior and properties of this setting
	 */
	UPROPERTY()
	const USettingType* SettingType = nullptr;

	/**
	 * The value that was initially applied
	 * Used as the default value and also to restore when the PIE session ends
	 * This is typically loaded from the setting's initial configuration
	 */
	UPROPERTY()
	FString InitialDefaultValue;

	/**
	 * The authoritative in-session applied value stored by the registry.
	 * Unset means the key is currently registered structurally but has no applied value.
	 */
	TOptional<FString> AppliedValue;

	/** Tracks whether this setting has no binding, is waiting to bind, is unsupported, or is actively bound. */
	ETargetBindingState TargetBindingState = ETargetBindingState::None;

	/** Active runtime binding instance for this setting while it is bound. */
	UPROPERTY(Transient)
	TObjectPtr<USettingBinding> Binding;

	/**
	 * If set by a controller setting, the controller-set default value for this setting
	 * This allows controller settings to influence their controlled settings' default values dynamically
	 * For example, overall quality presets can set default values for individual quality settings
	 */
	UPROPERTY()
	FString ControllerSetDefaultValue;

	/**
	 * Settings that are controlled by this setting
	 * i.e. When this setting changes, these settings are automatically updated
	 *
	 * Controlled settings affect the order of application - controlled settings
	 * must be applied after the settings that control them
	 */
	UPROPERTY()
	TArray<FFullSettingKey> ControlledSettings;
};

/**
 * Registry that maintains a collection of settings for a specific context
 *
 * The SettingRegistry is a central component that manages a set of settings,
 * handling their registration, initialization, application, and persistence.
 *
 * Multiple registries may exist in a single game instance, for example:
 * - A global registry for game-wide settings (graphics, audio, etc.)
 * - Individual player registries for player-specific settings (controls, preferences)
 *
 * The registry manages control relationships between settings and ensures proper
 * ordering when applying settings.
 */
UCLASS(BlueprintType)
class AUTOSETTINGS_API USettingRegistry : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	/**
	 * Result class used when a registry is determining control relationships between settings
	 * Allows control checking logic to indicate whether one setting controls another
	 */
	class FSettingControlResult
	{
	public:
		/** Mark this as a controlled setting */
		void SetControlled() { bIsControlled = true; }

		/** Check if this setting is marked as controlled */
		bool IsControlled() const { return bIsControlled; }

	private:
		/** Internal control flag */
		bool bIsControlled = false;
	};

	/**
	 * Global delegate that fires when checking if one setting controls another
	 * Used to allow external code to define custom control relationships
	 */
	static inline TMulticastDelegate<void(const FRegisteredSetting&, const FRegisteredSetting&, FSettingControlResult& Result)> OnCheckControlRelationship;

	/**
	 * Base key fragment for all settings in this registry
	 * All setting keys in this registry will start with this fragment
	 */
	UPROPERTY()
	FPartialSettingKey BaseKey;

	/**
	 * Whether this registry is being used at design time (editor)
	 * Design-time registries have different behavior for certain operations
	 */
	UPROPERTY()
	bool bDesignTime = false;

	/** Delegate that fires when a setting is applied */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSettingApplied, FSetSettingEvent, Event);

	/** Event broadcast when any setting in this registry is applied */
	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnSettingApplied OnSettingApplied;

	/** Delegate that fires when a setting is saved */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSettingSaved, FSetSettingEvent, Event);

	/** Event broadcast when any setting in this registry is saved */
	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnSettingSaved OnSettingSaved;

	/** Delegate that fires when the registry-owned applied value of a setting changes. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAppliedValueChanged, FSettingAppliedValueChangedEvent, Event);

	/** Event broadcast when any setting's registry-owned applied value changes. */
	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnAppliedValueChanged OnAppliedValueChanged;

	/**
	 * Configure which setting assets this registry owns.
	 *
	 * This separates asset discovery from the later Initialize call so callers can discover or cache the
	 * correct setting set ahead of time, then decide separately when the first registration and apply pass
	 * should actually happen.
	 *
	 * @param InSettingAssets - Array of setting type assets owned by this registry
	 * @return True if the registry accepted the configured setting assets
	 */
	bool SetSettingAssets(TArray<USettingType*> InSettingAssets);

	/** Registers a native listener for the one-time synchronous signal emitted after initial targets are captured. */
	void Register_OnTargetsCaptured(const FSimpleDelegate& Delegate);

	/** Whether this registry has already been configured with its owned setting assets. */
	bool HasSettingAssets() const
	{
		return bHasConfiguredSettingAssets;
	}

	/**
	 * Initialize the registry using the configured setting assets.
	 *
	 * This performs synchronous registration, saved-value hydration, dependency sorting,
	 * and the initial apply pipeline when initialization is explicitly requested.
	 *
	 * @return True if initialization completed successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	bool Initialize();

	/**
	 * Configure the owned setting assets and initialize the registry in one call.
	 *
	 * @param InSettingAssets - Array of setting type assets to register
	 * @return True if initialization completed successfully
	 */
	bool Initialize(TArray<USettingType*> InSettingAssets);

	/** Clean up the registry when it's being destroyed */
	virtual void BeginDestroy() override;

	/**
	 * Deinitialize the registry
	 * Cleans up registered settings and ensures values are properly saved
	 */
	void Deinitialize();

	/**
	 * Check if the registry has been initialized
	 *
	 * @return True if the registry is initialized and ready to use
	 */
	bool IsInitialized() const
	{
		return RegistryState == ESettingRegistryState::Initialized;
	}

	bool IsSettingRegistered(const FSettingKey& Key) const;

	/**
	 * Register a delegate to fire when the registry is initialized
	 * If the registry is already initialized, the delegate will fire immediately
	 *
	 * @param Delegate - The delegate to call when initialized
	 */
	void CallOrRegister_OnInitialized(const FSimpleDelegate& Delegate);

	/**
	 * Get the initial default value for a setting
	 *
	 * @param Key - The full key of the setting
	 * @return The initial default value as a string
	 */
	FString GetInitialDefaultValue(const FFullSettingKey& Key) const;

	/**
	 * Check if a setting has a controller-set default value
	 *
	 * @param Key - The full key of the setting
	 * @return True if the setting has a controller-set default value
	 */
	bool HasControllerSetDefaultValue(const FFullSettingKey& Key) const;

	/**
	 * Get the controller-set default value for a setting
	 *
	 * @param Key - The full key of the setting
	 * @return The controller-set default value as a string
	 */
	FString GetControllerSetDefaultValue(const FFullSettingKey& Key) const;

	/**
	 * Get the current effective default value for a setting
	 * Returns the controller-set value if one exists, otherwise the initial default
	 *
	 * @param Key - The full key of the setting
	 * @return The current default value as a string
	 */
	FString GetCurrentDefaultValue(const FFullSettingKey& Key) const;

	/**
	 * Get all registered settings in this registry
	 *
	 * @return Map of setting keys to registered setting data
	 */
	const TMap<FFullSettingKey, FRegisteredSetting>& GetRegisteredSettings() const { return RegisteredSettings; }

	/**
	 * Check if a setting is registered in this registry
	 *
	 * @param Key - The full key of the setting to check
	 * @return True if the setting is registered
	 */
	bool IsSettingRegistered(const FFullSettingKey& Key) const;

	/**
	 * Register and hydrate a batch of settings synchronously
	 *
	 * Registration and saved-value hydration happen in one pipeline so callers
	 * don't need to coordinate separate steps.
	 *
	 * @param Settings - Map of full setting keys to setting assets
	 * @return True if all requested settings were processed successfully
	 */
	bool RegisterSettings(const TMap<FFullSettingKey, USettingType*>& Settings);

	/**
	 * Get the full setting key for a setting type asset
	 *
	 * @param SettingAsset - The setting type asset
	 * @return The full key for the setting
	 */
	FFullSettingKey GetSettingKey(const USettingType* SettingAsset) const;

	// ========================================
	// DEPRECATED API (Backward Compatibility)
	// ========================================

	/**
	 * Apply a setting value by specifying the setting type and sub-key
	 *
	 * @param SettingType - Class of the setting type
	 * @param SubKey - Partial key extension to identify the specific setting
	 * @param Value - The value to apply
	 * @param bSave - Whether to also save the setting after applying
	 * @deprecated Use SetSetting with USettingKeyFunctions::MakeSettingKey and ESettingAction::ApplyOnly instead
	 */
	UE_DEPRECATED(2.0, "Use SetSetting with USettingKeyFunctions::MakeSettingKey and ESettingAction instead")
	UFUNCTION(BlueprintCallable, meta = (Category = "Settings", AutoCreateRefTerm = "SettingType,SubKey", DeprecatedFunction))
	void ApplySettingByType(UPARAM(meta = (AllowAbstract = false))
								const TSubclassOf<USettingType>& SettingType,
		const FPartialSettingKey& SubKey, const FString& Value, bool bSave = false);

	/**
	 * Apply a setting value by specifying its full key
	 *
	 * @param Key - The full key of the setting
	 * @param Value - The value to apply
	 * @param bSave - Whether to also save the setting after applying
	 * @deprecated Use SetSetting with ESettingAction::ApplyOnly instead
	 */
	UE_DEPRECATED(2.0, "Use SetSetting with ESettingAction instead")
	UFUNCTION(BlueprintCallable, meta = (Category = "Settings", DeprecatedFunction))
	void ApplySettingByKey(const FFullSettingKey& Key, const FString& Value, bool bSave = false);

	/**
	 * Save a setting value by specifying the setting type and sub-key
	 * This will also apply the setting if it hasn't been applied yet
	 *
	 * @param SettingType - Class of the setting type
	 * @param SubKey - Partial key extension to identify the specific setting
	 * @param Value - The value to save
	 * @deprecated Use SetSetting with USettingKeyFunctions::MakeSettingKey and ESettingAction::ApplyAndSave instead
	 */
	UE_DEPRECATED(2.0, "Use SetSetting with USettingKeyFunctions::MakeSettingKey and ESettingAction::ApplyAndSave instead")
	UFUNCTION(BlueprintCallable, meta = (Category = "Settings", AutoCreateRefTerm = "SettingType,SubKey", DeprecatedFunction))
	void SaveSettingByType(UPARAM(meta = (AllowAbstract = false))
							   const TSubclassOf<USettingType>& SettingType,
		const FPartialSettingKey& SubKey, const FString& Value);

	/**
	 * Save a setting value by specifying its full key
	 * This will also apply the setting if it hasn't been applied yet
	 *
	 * @param Key - The full key of the setting
	 * @param Value - The value to save
	 * @deprecated Use SetSetting with ESettingAction::ApplyAndSave instead
	 */
	UE_DEPRECATED(2.0, "Use SetSetting with ESettingAction::ApplyAndSave instead")
	UFUNCTION(BlueprintCallable, meta = (Category = "Settings", DeprecatedFunction))
	void SaveSettingByKey(const FFullSettingKey& Key, const FString& Value);

	/**
	 * Get the settings controlled by the given setting
	 * These are settings that are automatically updated when the given setting changes
	 *
	 * @param Key - The full key of the setting
	 * @return Array of setting keys that are controlled by this setting
	 */
	UFUNCTION(BlueprintPure, meta = (Category = "Settings"))
	TArray<FFullSettingKey> GetControlledSettings(const FFullSettingKey& Key) const;

	// ========================================
	// NEW SIMPLIFIED API (Recommended)
	// ========================================

	/**
	 * Set a setting value with clear action specification
	 * This is the recommended way to modify settings
	 *
	 * @param Key - The setting key (can be partial or full)
	 * @param Value - The value to set
	 * @param Action - How to handle the setting (apply only, apply and save, etc.)
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings", meta = (CallInEditor = true))
	void SetSetting(const FSettingKey& Key, const FString& Value, ESettingAction Action = ESettingAction::ApplyAndSave);

	/** Get the registry-owned applied value for a setting. */
	UFUNCTION(BlueprintPure, Category = "Settings")
	FString GetAppliedValue(const FSettingKey& Key) const;

	/** Whether the registry currently has an applied value for a setting. */
	UFUNCTION(BlueprintPure, Category = "Settings")
	bool HasAppliedValue(const FSettingKey& Key) const;

	/** Get the persisted saved value for a setting. */
	UFUNCTION(BlueprintPure, Category = "Settings")
	FString GetSavedValue(const FSettingKey& Key) const;

	/**
	 * Get a setting value with clear preference specification
	 * Falls back to the other source if the preferred source has no value
	 *
	 * @param Key - The setting key (can be partial or full)
	 * @param Preference - Whether to prefer applied or saved value (falls back to other if preferred is empty)
	 * @return The setting value as a string
	 */
	UFUNCTION(BlueprintPure, Category = "Settings")
	FString GetSettingValue(const FSettingKey& Key, ESettingValuePreference Preference = ESettingValuePreference::Applied) const;

	/**
	 * Register a native callback for changes to a specific setting's applied value.
	 * If requested, the current value is replayed immediately.
	 */
	void RegisterAppliedValueChangedCallback(const FSettingKey& Key,
		const UObject* Listener,
		TFunction<void(const FSettingReference&, const FString&)> Callback,
		bool bReplayCurrentValue = true);

	/**
	 * Register a Blueprint callback for changes to a specific setting's applied value.
	 * If requested, the current value is replayed immediately.
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings", meta = (DisplayName = "Register Applied Value Changed Callback", AutoCreateRefTerm = "Key,Callback"))
	void RegisterAppliedValueChangedCallback_BP(const FSettingKey& Key,
		FSettingAppliedValueCallback Callback,
		bool bReplayCurrentValue = true);

	/** Remove keyed applied-value callbacks owned by the given listener for a specific setting. */
	UFUNCTION(BlueprintCallable, Category = "Settings", meta = (AutoCreateRefTerm = "Key"))
	void RemoveAppliedValueChangedCallback(UObject* Listener, const FSettingKey& Key);

	/** Remove all keyed applied-value callbacks owned by the given listener. */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void RemoveAllAppliedValueChangedCallbacks(UObject* Listener);

	/**
	 * RAII helper to track in-progress setting applications.
	 * Used to manage contextual default values when parent settings trigger child setting updates.
	 */
	struct FScopedInProgressSettingApplication
	{
		FScopedInProgressSettingApplication(USettingRegistry* Registry, const FFullSettingKey& Setting)
			: Registry(Registry)
		{
			Registry->PushInProgressApplication(Setting);
		}

		~FScopedInProgressSettingApplication()
		{
			Registry->PopInProgressApplication();
		}

		USettingRegistry* Registry;
	};

protected:
	virtual void Tick(float DeltaTime) override;

	virtual TStatId GetStatId() const override;

private:
	friend class USettingBinding;

	/** Current state of the registry. */
	ESettingRegistryState RegistryState = ESettingRegistryState::Uninitialized;

	/** Native listeners notified synchronously after initial target capture and before initial value application. */
	FSimpleMulticastDelegate OnTargetsCaptured;

	/** Tracks whether a caller has configured which SettingType assets belong to this registry. */
	bool bHasConfiguredSettingAssets = false;

	UPROPERTY()
	TArray<USettingType*> SettingAssets;

	UPROPERTY()
	TArray<FFullSettingKey> InProgressSettingApplications;

	UPROPERTY()
	TMap<FFullSettingKey, USettingType*> SettingsPendingRegister;

	UPROPERTY()
	TMap<FFullSettingKey, FRegisteredSetting> RegisteredSettings;

	/**
	 * Cached default values for each registered SettingType, keyed by full setting key.
	 * This lets any registration path apply type-defined defaults before loading saved values.
	 */
	TMap<const USettingType*, TMap<FFullSettingKey, FString>> SettingTypeDefaultValues;

	/** Set of SettingTypes that have been registered (and had their providers executed) */
	TSet<const USettingType*> RegisteredSettingTypes;

	struct FAppliedValueSubscription
	{
		FFullSettingKey Key;
		TWeakObjectPtr<UObject> Listener;
		TFunction<void(const FSettingReference&, const FString&)> NativeCallback;
		FSettingAppliedValueCallback BlueprintCallback;

		bool IsValid() const
		{
			return Listener.IsValid() && (NativeCallback || BlueprintCallback.IsBound());
		}

		void Execute(const FSettingReference& Setting, const FString& Value) const
		{
			if (NativeCallback)
			{
				NativeCallback(Setting, Value);
			}
			if (BlueprintCallback.IsBound())
			{
				BlueprintCallback.ExecuteIfBound(Setting, Value);
			}
		}
	};

	// Multicast delegate fired the next time the registry is initialized
	FSimpleMulticastDelegate OnInitialized;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnAppliedValueChangedNative, const FSettingAppliedValueChangedEvent&);
	FOnAppliedValueChangedNative OnAppliedValueChangedNative;

	TMap<FFullSettingKey, TArray<FAppliedValueSubscription>> AppliedValueSubscriptions;

	TArray<FFullSettingKey> GetRegisteredKeysForSetting(const USettingType* Setting) const;
	const USettingType* FindSettingTypeForKey(const FFullSettingKey& Key) const;
	/** Look up a cached default value produced by a specific SettingType for a full key. */
	bool FindDefaultValueForSetting(const USettingType* SettingType, const FFullSettingKey& Key, FString& OutValue) const;

	/**
	 * Gets the full keys for all allowed child fragments of a setting type.
	 * Used to restrict which child settings can be registered under a parent.
	 */
	TArray<FFullSettingKey> GetAllowedChildKeys(const USettingType* Setting) const;

	bool IsCategorySetting(const FFullSettingKey& Key, const USettingType* Setting) const;

	FSetSettingEvent MakeSetSettingEvent(const FFullSettingKey& Key, const FString& Value);

	void TryRegisterPendingSettings();
	void TryBindRegisteredSettings();

	/**
	 * Registers and binds a batch, captures targets, and prepares explicit startup values in dependency order.
	 */
	bool PrepareSettings(const TMap<FFullSettingKey, USettingType*>& SettingsToRegister,
		TArray<FFullSettingKey>& OutApplyOrder, TMap<FFullSettingKey, FSettingData>& OutExplicitValues);

	/**
	 * Register a single setting key with a setting type.
	 * Raw registration only - does not load/apply saved values.
	 */
	bool RegisterSetting(const FFullSettingKey& Key, const USettingType* SettingAsset);
	bool TryBindSetting(const FFullSettingKey& Key);
	bool CanBindSetting(const FFullSettingKey& Key, const USettingType* SettingAsset) const;

	/**
	 * Establishes a control relationship where one setting controls another.
	 * Enforces DAG constraints to prevent circular dependencies.
	 */
	void RegisterControlledSetting(const FFullSettingKey& Setting, const FFullSettingKey& ControlledSetting);

	/**
	 * Determines if one setting should control another based on key patterns and delegates.
	 */
	bool ShouldRegisterControlledSetting(const FRegisteredSetting& Setting, const FRegisteredSetting& ControlledSetting);

	/**
	 * Conditionally registers a control relationship if ShouldRegisterControlledSetting returns true.
	 */
	void ConditionalRegisterControlledSetting(const FRegisteredSetting& Setting, const FRegisteredSetting& ControlledSetting);

	/** Applies explicit values in controller-first order and adopts current targets for keys without an override. */
	void ApplySettings(const TArray<FFullSettingKey>& ApplyOrder,
		const TMap<FFullSettingKey, FSettingData>& ExplicitValues,
		ESettingAppliedValueChangeSource Source = ESettingAppliedValueChangeSource::Initialization);

	void HandleBindingTargetValueChanged(USettingBinding* Binding);
	void HandleBindingInvalidated(USettingBinding* Binding);
	void UnbindSetting(const FFullSettingKey& Key, ETargetBindingState NewState = ETargetBindingState::None);
	bool IsActiveBinding(const USettingBinding* Binding, const FFullSettingKey& Key) const;

	void SetSettingInternal(const FSettingKey& Key, const FString& Value, ESettingAction Action, ESettingAppliedValueChangeSource Source);
	bool UpdateAppliedValue(const FFullSettingKey& Key, const FString& Value, ESettingAppliedValueChangeSource Source);
	void BroadcastAppliedValueChanged(const FSettingAppliedValueChangedEvent& Event);
	void DispatchAppliedValueSubscriptions(const FSettingAppliedValueChangedEvent& Event);
	void CleanupAppliedValueSubscriptions();
	void CleanupAppliedValueSubscriptions(const FFullSettingKey& Key);
	bool HasAppliedValue(const FFullSettingKey& Key) const;

	/**
	 * Tracks the start of a setting application. Used by FScopedInProgressSettingApplication RAII helper
	 * to manage contextual defaults when parent settings update their children.
	 */
	void PushInProgressApplication(const FFullSettingKey& Setting);

	/**
	 * Marks the end of a setting application. Used by FScopedInProgressSettingApplication RAII helper
	 * to clean up the in-progress tracking stack.
	 */
	void PopInProgressApplication();

	/**
	 * Set the initial default value for a setting
	 * This replaces the baseline default value loaded from the setting asset
	 *
	 * @param Key - The full key of the setting
	 * @param Value - The new initial default value to set
	 * @return True if the default was successfully set
	 */
	bool SetInitialDefaultValue(const FFullSettingKey& Key, const FString& Value);

	/**
	 * Restores all settings to their initial values captured at registry startup.
	 * Used primarily for PIE cleanup to prevent settings from persisting between sessions.
	 */
	void RestoreInitialValues();

	/**
	 * Update ControllerSetDefaultValue for all controlled settings
	 * Called when a controller setting is applied to ensure controlled settings
	 * have their defaults updated to reflect the new baseline
	 *
	 * @param ControllerKey - The key of the controller setting that controls others
	 */
	void UpdateControlledSettingsDefaults(const FFullSettingKey& ControllerKey);

	/**
	 * Check if a SettingType has been registered and had its providers executed.
	 *
	 * @param SettingType - The setting type to check
	 * @return True if the SettingType has been registered
	 */
	bool IsSettingTypeRegistered(const USettingType* SettingType) const;

	/**
	 * Register a SettingType and execute its default providers.
	 * Called the first time a setting of this type is registered.
	 * Caches the SettingType's generated default values by full key for owner resolution and registration.
	 *
	 * @param SettingType - The setting type to register
	 */
	void RegisterSettingType(USettingType* SettingType);

	/**
	 * Helper function to resolve a setting key to a full setting key
	 * Handles the logic of determining whether a key is partial or full and prepends the base key if needed
	 *
	 * @param Key - The setting key to resolve (can be partial or full)
	 * @return The resolved full setting key
	 */
	FFullSettingKey ResolveSettingKey(const FSettingKey& Key) const;
};
