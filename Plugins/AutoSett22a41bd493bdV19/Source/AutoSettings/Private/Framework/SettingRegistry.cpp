// Copyright Sam Bonifacio. All Rights Reserved.

#include "Framework/SettingRegistry.h"
#include "AutoSettingsError.h"
#include "AutoSettingsLogs.h"
#include "Algo/TopologicalSort.h"
#include "Logging/StructuredLog.h"
#include "Framework/SettingBinding.h"
#include "Framework/SettingType.h"
#include "Framework/SettingBindingStrategy.h"
#include "Framework/SettingKey.h"
#include "Framework/SettingDefaultProvider.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

namespace
{
	// Child SettingTypes are more specific than their parents and should win key ownership.
	bool IsMoreSpecificSettingType(const USettingRegistry* Registry, const USettingType* Candidate, const USettingType* Existing)
	{
		return Candidate && Existing && Registry->GetSettingKey(Candidate).IsChildOf(Registry->GetSettingKey(Existing));
	}

	// Resolve the owning SettingType for a discovered key in one place.
	// If two unrelated SettingTypes claim the same key, warn and keep the first one.
	USettingType* ResolveKeyOwner(const USettingRegistry* Registry, const FFullSettingKey& Key, const TArray<USettingType*>& Candidates)
	{
		USettingType* ResolvedOwner = nullptr;

		for (USettingType* Candidate : Candidates)
		{
			if (!IsValid(Candidate) || !Candidate->bEnabled)
			{
				continue;
			}

			if (!ResolvedOwner)
			{
				ResolvedOwner = Candidate;
				continue;
			}

			if (IsMoreSpecificSettingType(Registry, Candidate, ResolvedOwner))
			{
				ResolvedOwner = Candidate;
				continue;
			}

			if (IsMoreSpecificSettingType(Registry, ResolvedOwner, Candidate))
			{
				continue;
			}

			UE_LOGFMT(LogAutoSettings, Warning, "Ambiguous owner for setting key '{0}': keeping existing owner '{1}', ignoring '{2}'",
				Key.ToString(), GetNameSafe(ResolvedOwner), GetNameSafe(Candidate));
		}

		return ResolvedOwner;
	}
} // namespace

bool USettingRegistry::SetSettingAssets(TArray<USettingType*> InSettingAssets)
{
	if (!ensure(RegistryState == ESettingRegistryState::Uninitialized))
	{
		// Changing the owned assets after initialization would desynchronize every derived cache and
		// registered key, so only allow asset configuration before the first initialization pass.
		return false;
	}

	// Persist the owned asset set on the registry itself so callers can initialize later without
	// rediscovering or re-supplying the same setting list.
	SettingAssets = MoveTemp(InSettingAssets);
	bHasConfiguredSettingAssets = true;
	return true;
}

bool USettingRegistry::Initialize()
{
	if (!ensure(RegistryState == ESettingRegistryState::Uninitialized))
	{
		// Don't allow initializing more than once
		return false;
	}

	if (!bHasConfiguredSettingAssets)
	{
		// Callers must decide which SettingType assets this registry owns before requesting the initial
		// registration/apply pass.
		UE_LOGFMT(LogAutoSettings, Warning, "{0} cannot initialize before setting assets have been configured", GetName());
		return false;
	}

	// Enter the transaction before any registration work so reentrant initialization cannot start a second pass.
	RegistryState = ESettingRegistryState::Initializing;
	UE_LOGFMT(LogAutoSettings, Log, "{0} initializing", GetName());

	const TArray<USettingType*>& Settings = SettingAssets;
	SettingTypeDefaultValues.Empty();

	// Register all SettingTypes early to ensure providers run before settings are registered
	// This ensures that provider-generated default values are available when settings initialize
	UE_LOGFMT(LogAutoSettings, Log, "Pre-registering {0} SettingTypes to execute default providers", Settings.Num());
	for (USettingType* SettingAsset : Settings)
	{
		if (SettingAsset && SettingAsset->bEnabled && !IsSettingTypeRegistered(SettingAsset))
		{
			RegisterSettingType(SettingAsset);
		}
	}
	UE_LOGFMT(LogAutoSettings, Log, "Completed pre-registration of SettingTypes");

	// Collect all possible owners for each discovered key, then resolve the final owner once.
	TMap<FFullSettingKey, TArray<USettingType*>> KeyOwnerCandidates;
	TSet<FFullSettingKey> PersistedDiscoveredKeys;

	for (USettingType* Setting : Settings)
	{
		if (!IsValid(Setting) || !Setting->bEnabled)
		{
			continue;
		}

		TArray<FFullSettingKey> Keys;

		// Add the setting's own key if it has one
		if (!Setting->KeyFragment.IsNone())
		{
			Keys.Add(GetSettingKey(Setting));
		}

		// Add any explicitly allowed child keys
		Keys.Append(GetAllowedChildKeys(Setting));

		// Let the persistence strategy discover additional keys (e.g., from config files)
		if (ensure(Setting->PersistenceStrategy))
		{
			const TArray<FFullSettingKey> DiscoveredKeys = Setting->PersistenceStrategy->FindChildKeys(GetSettingKey(Setting));
			Keys.Append(DiscoveredKeys);
			for (const FFullSettingKey& DiscoveredKey : DiscoveredKeys)
			{
				PersistedDiscoveredKeys.Add(DiscoveredKey);
			}
		}

		// Collect each candidate owner for the single resolution pass below.
		for (FFullSettingKey& Key : Keys)
		{
			KeyOwnerCandidates.FindOrAdd(Key).AddUnique(Setting);
		}

		if (const TMap<FFullSettingKey, FString>* DefaultValues = SettingTypeDefaultValues.Find(Setting))
		{
			// Defaults contribute additional concrete keys, but owner selection still happens
			// once below after all candidate owners from all sources have been collected.
			for (const auto& DefaultPair : *DefaultValues)
			{
				KeyOwnerCandidates.FindOrAdd(DefaultPair.Key).AddUnique(Setting);
			}
		}
	}

	TMap<FFullSettingKey, USettingType*> KeyMap;
	for (const auto& CandidatePair : KeyOwnerCandidates)
	{
		if (USettingType* ResolvedOwner = ResolveKeyOwner(this, CandidatePair.Key, CandidatePair.Value))
		{
			KeyMap.Add(CandidatePair.Key, ResolvedOwner);
		}
	}

	for (const auto& KeyValue : KeyMap)
	{
		if (PersistedDiscoveredKeys.Contains(KeyValue.Key)
			&& KeyValue.Value->BindingStrategy
			&& !KeyValue.Value->BindingStrategy->ValidateConfiguration({ KeyValue.Key, this }))
		{
			UE_LOGFMT(LogAutoSettings, Warning,
				"Skipping discovered persisted key '{0}' for setting asset '{1}' because its configuration is invalid. The persisted data may be stale.",
				KeyValue.Key.ToString(), GetNameSafe(KeyValue.Value));
			continue;
		}

		if (IsCategorySetting(KeyValue.Key, KeyValue.Value))
		{
			// Key is a category, skip
			continue;
		}
		SettingsPendingRegister.Add(KeyValue.Key, KeyValue.Value);
	}

	// Prepare all initial keys and explicit values without applying them so every target is captured first.
	const TMap<FFullSettingKey, USettingType*> SettingsToRegister = SettingsPendingRegister;
	TArray<FFullSettingKey> InitialApplyOrder;
	TMap<FFullSettingKey, FSettingData> ExplicitInitialValues;
	if (!PrepareSettings(SettingsToRegister, InitialApplyOrder, ExplicitInitialValues))
	{
		UE_LOGFMT(LogAutoSettings, Warning, "{0} completed initial setting preparation with errors", GetName());
	}
	for (const auto& KeyValue : SettingsToRegister)
	{
		SettingsPendingRegister.Remove(KeyValue.Key);
	}

	// Publish completed target capture synchronously before initial values are finalized.
	OnTargetsCaptured.Broadcast();
	OnTargetsCaptured.Clear();

	// Persisted and configured values finalize startup after capture listeners run, preserving controller-first order.
	ApplySettings(InitialApplyOrder, ExplicitInitialValues, ESettingAppliedValueChangeSource::Initialization);
	TryBindRegisteredSettings();

	// RegisterSettingType caches provider output; key registration and hydration happen in batches below.

#if WITH_EDITOR
	if (!bDesignTime)
	{
		FEditorDelegates::EndPIE.AddWeakLambda(this, [this](const bool bSimulatingInEditor)
			{
				// PIE shutdown deinitializes while editor systems can still restore values back to their pre-PIE state.
				UE_LOGFMT(LogAutoSettings, Log, "{0} end PIE detected", GetName());
				if (RegistryState == ESettingRegistryState::Initialized)
				{
					Deinitialize();
				}
			});
	}
#endif

	RegistryState = ESettingRegistryState::Initialized;

	// Broadcast delegate to tell other systems about initialization
	OnInitialized.Broadcast();
	// Clear delegate list
	OnInitialized.Clear();
	return true;
}

bool USettingRegistry::Initialize(TArray<USettingType*> InSettingAssets)
{
	// Configure the owned setting assets first, then run the same initialization pipeline exposed by
	// the parameterless overload.
	return SetSettingAssets(MoveTemp(InSettingAssets)) && Initialize();
}

void USettingRegistry::BeginDestroy()
{
	UObject::BeginDestroy();

	if (RegistryState == ESettingRegistryState::Initialized)
	{
		Deinitialize();
	}
}

void USettingRegistry::Deinitialize()
{
	if (!ensure(RegistryState == ESettingRegistryState::Initialized))
	{
		return;
	}

	UE_LOGFMT(LogAutoSettings, Log, "{0} deinitializing", GetName());

#if WITH_EDITOR
	if (!IsRunningGame())
	{
		UE_LOGFMT(LogAutoSettings, Log, "{0}: Deinitializing in editor, restore initial applied values", GetName());
		RestoreInitialValues();
	}
#endif

	for (const auto& RegisteredSettingPair : RegisteredSettings)
	{
		const FRegisteredSetting& RegisteredSetting = RegisteredSettingPair.Value;
		if (RegisteredSetting.TargetBindingState == ETargetBindingState::Bound)
		{
			UnbindSetting(RegisteredSetting.Key);
		}
	}

	RegistryState = ESettingRegistryState::Deinitialized;
	AppliedValueSubscriptions.Empty();
}

void USettingRegistry::CallOrRegister_OnInitialized(const FSimpleDelegate& Delegate)
{
	if (IsInitialized())
	{
		Delegate.Execute();
	}
	else
	{
		OnInitialized.Add(Delegate);
	}
}

void USettingRegistry::Register_OnTargetsCaptured(const FSimpleDelegate& Delegate)
{
	// Capture listeners must be registered before the one-time initialization transaction begins.
	if (!ensureMsgf(RegistryState == ESettingRegistryState::Uninitialized,
		TEXT("Targets-captured listeners must be registered before registry initialization.")))
	{
		return;
	}

	if (!ensureMsgf(Delegate.IsBound(), TEXT("Targets-captured listener must be bound.")))
	{
		return;
	}

	OnTargetsCaptured.Add(Delegate);
}

FString USettingRegistry::GetInitialDefaultValue(const FFullSettingKey& Key) const
{
	if (!ensure(IsSettingRegistered(Key)))
	{
		return FString();
	}
	return RegisteredSettings[Key].InitialDefaultValue;
}

bool USettingRegistry::HasControllerSetDefaultValue(const FFullSettingKey& Key) const
{
	if (!ensure(IsSettingRegistered(Key)))
	{
		return false;
	}
	return !RegisteredSettings[Key].ControllerSetDefaultValue.IsEmpty();
}

FString USettingRegistry::GetControllerSetDefaultValue(const FFullSettingKey& Key) const
{
	if (!ensure(IsSettingRegistered(Key)))
	{
		return FString();
	}
	return RegisteredSettings[Key].ControllerSetDefaultValue;
}

FString USettingRegistry::GetCurrentDefaultValue(const FFullSettingKey& Key) const
{
	if (!ensure(IsSettingRegistered(Key)))
	{
		return "";
	}

	if (HasControllerSetDefaultValue(Key))
	{
		// The default value was overridden by another setting
		return GetControllerSetDefaultValue(Key);
	}

	const USettingType* SettingAsset = FindSettingTypeForKey(Key);

	if (SettingAsset->bOverrideDefaultValue)
	{
		// The setting asset has provided its own default value
		return SettingAsset->DefaultValue;
	}

	// Get the initial value of the setting that was stored by the registry
	return GetInitialDefaultValue(Key);
}

bool USettingRegistry::SetInitialDefaultValue(const FFullSettingKey& Key, const FString& Value)
{
	if (!IsSettingRegistered(Key))
	{
		UE_LOGFMT(LogAutoSettings, Warning, "Cannot set initial default for unregistered setting: {0}", Key.ToString());
		return false;
	}

	FRegisteredSetting& RegisteredSetting = RegisteredSettings[Key];
	RegisteredSetting.InitialDefaultValue = Value;

	UE_LOGFMT(LogAutoSettings, Verbose, "Set initial default for '{0}' = '{1}'", Key.ToString(), Value);
	return true;
}

bool USettingRegistry::IsSettingRegistered(const FFullSettingKey& Key) const
{
	return RegisteredSettings.Contains(Key);
}

bool USettingRegistry::IsSettingRegistered(const FSettingKey& Key) const
{
	return IsSettingRegistered(ResolveSettingKey(Key));
}

bool USettingRegistry::RegisterSetting(const FFullSettingKey& Key, const USettingType* SettingAsset)
{
	if (!ensure(!IsSettingRegistered(Key)))
	{
		return false;
	}

	if (!ensure(SettingAsset))
	{
		return false;
	}

	// SettingTypes must be registered before keys so provider-generated defaults are available.
	if (!ensure(IsSettingTypeRegistered(SettingAsset)))
	{
		UE_LOGFMT(LogAutoSettings, Error, "SettingType '{0}' was not registered during initialization. This is unexpected and may indicate a logic error.",
			SettingAsset->GetName());
		return false;
	}

	auto AllowedChildKeys = GetAllowedChildKeys(SettingAsset);
	if (!AllowedChildKeys.IsEmpty() && !AllowedChildKeys.Contains(Key))
	{
		UE_LOGFMT(LogAutoSettings, Error, "Ignoring '{0}' because it is not in {1} AllowedChildFragments", Key.ToString(), GetNameSafe(SettingAsset));
		return false;
	}

	FRegisteredSetting RegisteredSetting;
	RegisteredSetting.Key = Key;
	RegisteredSetting.SettingType = SettingAsset;
	RegisteredSettings.Add(Key, RegisteredSetting);

	// Register control relationships with other settings
	TArray<FRegisteredSetting> OtherSettings;
	RegisteredSettings.GenerateValueArray(OtherSettings);
	for (auto& OtherSetting : OtherSettings)
	{
		if (OtherSetting.Key != Key)
		{
			ConditionalRegisterControlledSetting(RegisteredSetting, OtherSetting);
			ConditionalRegisterControlledSetting(OtherSetting, RegisteredSetting);
		}
	}

	UE_LOGFMT(LogAutoSettings, Log, "Registered setting '{0}' with setting asset '{1}'", Key.ToString(),
		SettingAsset->GetName());

	return true;
}

bool USettingRegistry::CanBindSetting(const FFullSettingKey& Key, const USettingType* SettingAsset) const
{
	if (!SettingAsset || !SettingAsset->BindingStrategy)
	{
		return false;
	}

	const FSettingReference Setting{ Key, FSettingContext(const_cast<USettingRegistry*>(this)) };
	return SettingAsset->BindingStrategy->ValidateConfiguration(Setting)
		&& SettingAsset->BindingStrategy->IsBindingReady(FSettingContext(const_cast<USettingRegistry*>(this)));
}

bool USettingRegistry::TryBindSetting(const FFullSettingKey& Key)
{
	FRegisteredSetting* RegisteredSetting = RegisteredSettings.Find(Key);
	if (!RegisteredSetting || RegisteredSetting->TargetBindingState == ETargetBindingState::Bound
		|| RegisteredSetting->TargetBindingState == ETargetBindingState::Unsupported)
	{
		return false;
	}

	const USettingType* SettingAsset = RegisteredSetting->SettingType;
	if (!SettingAsset || !SettingAsset->BindingStrategy)
	{
		RegisteredSetting->TargetBindingState = ETargetBindingState::None;
		return false;
	}

	RegisteredSetting->TargetBindingState = ETargetBindingState::Pending;

	const FSettingReference Setting{ Key, FSettingContext(this) };
	if (!SettingAsset->BindingStrategy->ValidateConfiguration(Setting))
	{
		// Invalid configuration is a stable property of this key/strategy pairing, so stop retrying
		// after the first failure. This commonly happens for structural keys like input categories.
		RegisteredSetting->TargetBindingState = ETargetBindingState::Unsupported;
		UE_LOGFMT(LogAutoSettings, Verbose,
			"Skipping target binding for setting '{0}' because its configuration is invalid for strategy '{1}'.",
			Key.ToString(), SettingAsset->BindingStrategy->GetClass()->GetName());
		return false;
	}

	if (!SettingAsset->BindingStrategy->IsBindingReady(FSettingContext(this)))
	{
		UE_LOGFMT(LogAutoSettings, Verbose,
			"Target binding for setting '{0}' is not ready yet for strategy '{1}'.",
			Key.ToString(), SettingAsset->BindingStrategy->GetClass()->GetName());
		return false;
	}

	const FString TargetValue = SettingAsset->BindingStrategy->GetTargetValue(Setting);
	if (RegisteredSetting->InitialDefaultValue.IsEmpty())
	{
		RegisteredSetting->InitialDefaultValue = TargetValue;
	}

	RegisteredSetting->Binding = NewObject<USettingBinding>(this);
	RegisteredSetting->Binding->Initialize(this, Setting, SettingAsset->BindingStrategy, SettingAsset->BindingStrategy->Triggers);
	RegisteredSetting->TargetBindingState = ETargetBindingState::Bound;
	RegisteredSetting->Binding->Begin();

	if (RegisteredSetting->Binding != RegisteredSettings[Key].Binding || !RegisteredSetting->Binding->IsActive()
		|| RegisteredSettings[Key].TargetBindingState != ETargetBindingState::Bound)
	{
		return RegisteredSettings[Key].TargetBindingState == ETargetBindingState::Bound;
	}

	if (RegisteredSetting->AppliedValue.IsSet())
	{
		SetSettingInternal(Key, RegisteredSetting->AppliedValue.GetValue(), ESettingAction::ApplyOnly, ESettingAppliedValueChangeSource::Initialization);
	}
	else if (!TargetValue.IsEmpty())
	{
		UpdateAppliedValue(Key, TargetValue, ESettingAppliedValueChangeSource::Initialization);
	}

	return true;
}

void USettingRegistry::UnbindSetting(const FFullSettingKey& Key, ETargetBindingState NewState)
{
	FRegisteredSetting* RegisteredSetting = RegisteredSettings.Find(Key);
	if (!RegisteredSetting)
	{
		return;
	}

	if (RegisteredSetting->Binding)
	{
		RegisteredSetting->Binding->End();
		RegisteredSetting->Binding = nullptr;
	}

	RegisteredSetting->TargetBindingState = NewState;
}

void USettingRegistry::RegisterControlledSetting(const FFullSettingKey& Setting, const FFullSettingKey& ControlledSetting)
{
	if (!ensure(RegisteredSettings.Contains(Setting)))
	{
		return;
	}

	if (!ensure(RegisteredSettings.Contains(ControlledSetting)))
	{
		return;
	}

	if (RegisteredSettings[Setting].ControlledSettings.Contains(ControlledSetting))
	{
		// Already registered
		return;
	}

	// Prevent circular control dependencies: A cannot control B if B already controls A
	// This maintains a directed acyclic graph of control relationships
	if (!ensure(!RegisteredSettings[ControlledSetting].ControlledSettings.Contains(Setting)))
	{
		// Circular dependency detected
		return;
	}

	RegisteredSettings[Setting].ControlledSettings.Add(ControlledSetting);

	UE_LOGFMT(LogAutoSettings, Log, "Registered controlled setting: '{0}' -> '{1}'", Setting.ToString(), ControlledSetting.ToString());
}

FFullSettingKey USettingRegistry::GetSettingKey(const USettingType* SettingAsset) const
{
	if (!ensure(SettingAsset))
	{
		return {};
	}

	FFullSettingKey Key;

	// Start with the registry base key if any
	Key += BaseKey;

	// Add the setting type's partial key (includes inheritance hierarchy)
	Key += SettingAsset->GetPartialSettingKey();

	ensure(Key.IsValid());
	return Key;
}

void USettingRegistry::ApplySettingByType(const TSubclassOf<USettingType>& SettingClass, const FPartialSettingKey& SubKey, const FString& Value, bool bSave)
{
	// Forward to SetSetting with a key built from the type and sub-key.
	FPartialSettingKey Key = SB::AutoSettings::SettingKeys::MakeSettingKey(SettingClass, SubKey);
	ESettingAction Action = bSave ? ESettingAction::ApplyAndSave : ESettingAction::ApplyOnly;
	SetSetting(Key, Value, Action);
}

void USettingRegistry::ApplySettingByKey(const FFullSettingKey& Key, const FString& Value, bool bSave)
{
	// Forward to SetSetting using the provided full key.
	ESettingAction Action = bSave ? ESettingAction::ApplyAndSave : ESettingAction::ApplyOnly;
	SetSetting(Key, Value, Action);
}

void USettingRegistry::SaveSettingByType(const TSubclassOf<USettingType>& SettingClass, const FPartialSettingKey& SubKey, const FString& Value)
{
	// Forward to SetSetting with a key built from the type and sub-key.
	FPartialSettingKey Key = SB::AutoSettings::SettingKeys::MakeSettingKey(SettingClass, SubKey);
	SetSetting(Key, Value, ESettingAction::ApplyAndSave);
}

void USettingRegistry::SaveSettingByKey(const FFullSettingKey& Key, const FString& Value)
{
	// Forward to SetSetting using the provided full key.
	SetSetting(Key, Value, ESettingAction::ApplyAndSave);
}

TArray<FFullSettingKey> USettingRegistry::GetControlledSettings(const FFullSettingKey& Key) const
{
	if (const FRegisteredSetting* RegisteredSetting = RegisteredSettings.Find(Key))
	{
		return RegisteredSetting->ControlledSettings;
	}
	return {};
}

// ========================================
// NEW SIMPLIFIED API IMPLEMENTATION
// ========================================

/**
 * Resolves partial setting keys to full keys by combining with the registry's base key.
 * Enables short-hand references (e.g. "Volume" -> "Player.0.Volume").
 */
FFullSettingKey USettingRegistry::ResolveSettingKey(const FSettingKey& Key) const
{
	FFullSettingKey FullKey;
	if (Key.Fragments.Num() > 0 && BaseKey.Fragments.Num() > 0 && !Key.IsChildOf(BaseKey))
	{
		// Input is a partial key that needs the base key prepended
		// Combine BaseKey + Key to create full hierarchical path
		FullKey += BaseKey;
		FullKey += Key;
	}
	else
	{
		// Input is already a full key (contains base) or base key is empty
		// Use the key fragments as-is
		FullKey.Fragments = Key.Fragments;
	}
	return FullKey;
}

void USettingRegistry::SetSetting(const FSettingKey& Key, const FString& Value, ESettingAction Action)
{
	SetSettingInternal(Key, Value, Action, ESettingAppliedValueChangeSource::SetSetting);
}

FString USettingRegistry::GetAppliedValue(const FSettingKey& Key) const
{
	const FFullSettingKey FullKey = ResolveSettingKey(Key);

	if (!ensure(IsSettingRegistered(FullKey)))
	{
		return FString();
	}

	if (bDesignTime)
	{
		return GetCurrentDefaultValue(FullKey);
	}

	return RegisteredSettings[FullKey].AppliedValue.Get(FString());
}

bool USettingRegistry::HasAppliedValue(const FSettingKey& Key) const
{
	const FFullSettingKey FullKey = ResolveSettingKey(Key);
	if (!ensure(IsSettingRegistered(FullKey)))
	{
		return false;
	}

	if (bDesignTime)
	{
		return !GetCurrentDefaultValue(FullKey).IsEmpty();
	}

	return HasAppliedValue(FullKey);
}

FString USettingRegistry::GetSavedValue(const FSettingKey& Key) const
{
	const FFullSettingKey FullKey = ResolveSettingKey(Key);

	if (!ensure(IsSettingRegistered(FullKey)))
	{
		return FString();
	}

	if (bDesignTime)
	{
		return GetCurrentDefaultValue(FullKey);
	}

	const USettingType* SettingType = FindSettingTypeForKey(FullKey);
	if (!ensure(SettingType && SettingType->PersistenceStrategy))
	{
		return FString();
	}

	const FSettingData SavedData = SettingType->PersistenceStrategy->GetSavedValue({ FullKey, const_cast<USettingRegistry*>(this) });
	return SavedData.IsValid() ? SavedData.Value : FString();
}

void USettingRegistry::RegisterAppliedValueChangedCallback(const FSettingKey& Key,
	const UObject* Listener,
	TFunction<void(const FSettingReference&, const FString&)> Callback,
	bool bReplayCurrentValue)
{
	if (!ensure(Listener))
	{
		return;
	}

	const FFullSettingKey FullKey = ResolveSettingKey(Key);
	CleanupAppliedValueSubscriptions(FullKey);

	auto& Subscriptions = AppliedValueSubscriptions.FindOrAdd(FullKey);
	Subscriptions.RemoveAll([Listener](const FAppliedValueSubscription& Subscription)
		{
			return !Subscription.Listener.IsValid() || Subscription.Listener.Get() == Listener;
		});

	FAppliedValueSubscription Subscription;
	Subscription.Key = FullKey;
	Subscription.Listener = const_cast<UObject*>(Listener);
	Subscription.NativeCallback = MoveTemp(Callback);
	Subscriptions.Add(MoveTemp(Subscription));

	if (bReplayCurrentValue && IsSettingRegistered(FullKey) && HasAppliedValue(FullKey))
	{
		Subscriptions.Last().Execute({ FullKey, FSettingContext(this) }, GetAppliedValue(FullKey));
	}
}

void USettingRegistry::RegisterAppliedValueChangedCallback_BP(const FSettingKey& Key,
	FSettingAppliedValueCallback Callback,
	bool bReplayCurrentValue)
{
	if (!Callback.IsBound())
	{
		return;
	}

	UObject* Listener = Callback.GetUObject();
	if (!ensure(Listener))
	{
		return;
	}

	const FFullSettingKey FullKey = ResolveSettingKey(Key);
	CleanupAppliedValueSubscriptions(FullKey);

	auto& Subscriptions = AppliedValueSubscriptions.FindOrAdd(FullKey);
	Subscriptions.RemoveAll([Listener](const FAppliedValueSubscription& Subscription)
		{
			return !Subscription.Listener.IsValid() || Subscription.Listener.Get() == Listener;
		});

	FAppliedValueSubscription Subscription;
	Subscription.Key = FullKey;
	Subscription.Listener = Listener;
	Subscription.BlueprintCallback = Callback;
	Subscriptions.Add(MoveTemp(Subscription));

	if (bReplayCurrentValue && IsSettingRegistered(FullKey) && HasAppliedValue(FullKey))
	{
		Subscriptions.Last().Execute({ FullKey, FSettingContext(this) }, GetAppliedValue(FullKey));
	}
}

void USettingRegistry::RemoveAppliedValueChangedCallback(UObject* Listener, const FSettingKey& Key)
{
	if (!Listener)
	{
		return;
	}

	const FFullSettingKey FullKey = ResolveSettingKey(Key);

	if (TArray<FAppliedValueSubscription>* Subscriptions = AppliedValueSubscriptions.Find(FullKey))
	{
		Subscriptions->RemoveAll([Listener](const FAppliedValueSubscription& Subscription)
			{
				return !Subscription.Listener.IsValid() || Subscription.Listener.Get() == Listener;
			});
		if (Subscriptions->Num() == 0)
		{
			AppliedValueSubscriptions.Remove(FullKey);
		}
	}
}

void USettingRegistry::RemoveAllAppliedValueChangedCallbacks(UObject* Listener)
{
	if (!Listener)
	{
		return;
	}

	for (auto It = AppliedValueSubscriptions.CreateIterator(); It; ++It)
	{
		It.Value().RemoveAll([Listener](const FAppliedValueSubscription& Subscription)
			{
				return !Subscription.Listener.IsValid() || Subscription.Listener.Get() == Listener;
			});
		if (It.Value().Num() == 0)
		{
			It.RemoveCurrent();
		}
	}
}

void USettingRegistry::SetSettingInternal(const FSettingKey& Key, const FString& Value, ESettingAction Action, ESettingAppliedValueChangeSource Source)
{
	if (RegistryState == ESettingRegistryState::Deinitialized)
	{
		UE_LOGFMT(LogAutoSettings, Warning, "Attempted to set setting '{0}' on deinitialized registry '{1}'. Operation ignored to prevent saving default values.", Key.ToString(), GetName());
		return;
	}

	const FFullSettingKey FullKey = ResolveSettingKey(Key);

	if (!ensure(IsSettingRegistered(FullKey)))
	{
		return;
	}

	const USettingType* SettingType = FindSettingTypeForKey(FullKey);
	if (!ensure(SettingType))
	{
		return;
	}

	UpdateAppliedValue(FullKey, Value, Source);

	// The registry becomes authoritative before we touch any external target so that any
	// callbacks triggered during application can query the new state immediately.
	// Apply the setting with proper scoping to manage controlled settings
	{
		FScopedInProgressSettingApplication SettingApplication(this, FullKey);
		SettingType->Apply(MakeSetSettingEvent(FullKey, Value));

		// Update ControllerSetDefaultValue for all controlled settings while in scope
		UpdateControlledSettingsDefaults(FullKey);
	}

	// Handle saving based on action
	switch (Action)
	{
		case ESettingAction::ApplyOnly:
			// Nothing more to do - just applied
			break;

		case ESettingAction::ApplyAndSave:
			// Save the main setting directly using the setting type
			SettingType->Save(MakeSetSettingEvent(FullKey, Value));

			// Handle controlled settings cascade: when a controller setting is saved,
			// we also need to save all the settings it controls with their current values
			// Example: When "Overall Quality" is saved, save "Shadow Quality", "Texture Quality", etc.
			const TArray<FFullSettingKey> ControlledSettingKeys = GetControlledSettings(FullKey);

			// Save all controlled settings with recursive saving to handle control chains
			// This ensures that if Setting A controls Setting B, and Setting B controls Setting C,
			// then saving Setting A will also save both B and C with their current applied values
			for (const FFullSettingKey& ControlledKey : ControlledSettingKeys)
			{
				if (IsSettingRegistered(ControlledKey))
				{
					const USettingType* ControlledSettingType = FindSettingTypeForKey(ControlledKey);
					if (ControlledSettingType)
					{
						// Get the registry-owned applied value of the controlled setting.
						// This may have been updated by the controller's bound-system side effects.
						const FString ControlledValue = GetAppliedValue(ControlledKey);
						// Recursively save controlled settings to handle multi-level control chains
						SetSettingInternal(ControlledKey, ControlledValue, ESettingAction::ApplyAndSave, Source);
					}
				}
			}
			break;
	}

	OnSettingApplied.Broadcast(MakeSetSettingEvent(FullKey, Value));
}

FString USettingRegistry::GetSettingValue(const FSettingKey& Key, ESettingValuePreference Preference) const
{
	const FString AppliedValue = GetAppliedValue(Key);
	const FString SavedValue = GetSavedValue(Key);

	return Preference == ESettingValuePreference::Saved
		? (!SavedValue.IsEmpty() ? SavedValue : AppliedValue)
		: (!AppliedValue.IsEmpty() ? AppliedValue : SavedValue);
}

void USettingRegistry::Tick(float DeltaTime)
{
	if (RegistryState != ESettingRegistryState::Initialized)
	{
		// Tick work can create target bindings, so only initialized registries may process pending
		// registration or binding retries.
		return;
	}

	if (SettingsPendingRegister.Num() > 0)
	{
		TryRegisterPendingSettings();
	}

	TryBindRegisteredSettings();
}

TStatId USettingRegistry::GetStatId() const
{
	return {};
}

TArray<FFullSettingKey> USettingRegistry::GetRegisteredKeysForSetting(const USettingType* Setting) const
{
	if (!ensure(Setting))
	{
		return {};
	}

	TArray<FFullSettingKey> Result;
	for (auto& Key : RegisteredSettings)
	{
		if (Key.Value.SettingType == Setting)
		{
			Result.Add(Key.Key);
		}
	}
	return Result;
}

const USettingType* USettingRegistry::FindSettingTypeForKey(const FFullSettingKey& Key) const
{
	if (!ensure(IsSettingRegistered(Key)))
	{
		return {};
	}

	return RegisteredSettings[Key].SettingType;
}

bool USettingRegistry::FindDefaultValueForSetting(const USettingType* SettingType, const FFullSettingKey& Key, FString& OutValue) const
{
	if (!ensure(SettingType))
	{
		return false;
	}

	const TMap<FFullSettingKey, FString>* DefaultValues = SettingTypeDefaultValues.Find(SettingType);
	if (!DefaultValues)
	{
		return false;
	}

	if (const FString* DefaultValue = DefaultValues->Find(Key))
	{
		OutValue = *DefaultValue;
		return true;
	}

	return false;
}

TArray<FFullSettingKey> USettingRegistry::GetAllowedChildKeys(const USettingType* Setting) const
{
	const FFullSettingKey FullBaseKey = GetSettingKey(Setting);

	TArray<FFullSettingKey> Result;
	for (const FSettingKeyFragment& ChildKeyName : Setting->AllowedChildKeyFragments)
	{
		FFullSettingKey ChildFullKey = FullBaseKey;
		ChildFullKey += ChildKeyName;
		Result.Add(ChildFullKey);
	}

	return Result;
}

bool USettingRegistry::IsCategorySetting(const FFullSettingKey& Key, const USettingType* Setting) const
{
	// Determine if the setting key is a category, if so we shouldn't register it

	// If the key isn't the base key if the setting, it's not a category
	if (GetSettingKey(Setting) != Key)
	{
		// If it's not the same, it should be a child
		ensure(Key.IsChildOf(GetSettingKey(Setting)));
		return false;
	}

	// If the setting asset has any children, it's a category
	for (const auto& OtherSetting : SettingAssets)
	{
		if (OtherSetting != Setting && OtherSetting->IsA(Setting->GetClass()))
		{
			return true;
		}
	}

	return false;
}

FSetSettingEvent USettingRegistry::MakeSetSettingEvent(const FFullSettingKey& Key, const FString& Value)
{
	const USettingType* Setting = FindSettingTypeForKey(Key);
	const FSettingData SettingValue{ Value };
	return FSetSettingEvent({ Key, FSettingContext(this) }, SettingValue, Setting->SettingTags);
}

/**
 * Sorts settings by control relationships to ensure controllers are applied before their controlled settings.
 * Uses topological sorting to handle dependency chains and detect circular dependencies.
 */
void SortSettings(const USettingRegistry* Registry, TArray<FFullSettingKey>& Settings)
{
	auto& RegisteredSettings = Registry->GetRegisteredSettings();

	// Create dependency resolver: For each setting, return the settings it controls
	// This builds the directed graph needed for topological sorting
	auto GetControlledSettings = [&](const FFullSettingKey& Setting) -> TArray<FFullSettingKey>
	{
		if (auto RegisteredSetting = RegisteredSettings.Find(Setting))
		{
			return RegisteredSetting->ControlledSettings;
		}
		return {};
	};

	// Apply Kahn's topological sorting algorithm to resolve dependency order
	// Returns false if circular dependencies are detected (impossible to sort)
	if (!Algo::TopologicalSort(Settings, GetControlledSettings))
	{
		// Circular dependency detected - this is a critical error that prevents
		// safe setting application order and could cause infinite update loops
		UE_LOGFMT(LogAutoSettings, Error, "Circular control relationship detected in settings");
	}

	// UE's TopologicalSort returns dependencies-first order (controlled before controllers)
	// We need controllers-first order for proper application sequence, so reverse the array
	Algo::Reverse(Settings);
}

bool USettingRegistry::RegisterSettings(const TMap<FFullSettingKey, USettingType*>& SettingsToRegister)
{
	// Late registration uses the same preparation path, then immediately applies every current default.
	TArray<FFullSettingKey> ApplyOrder;
	TMap<FFullSettingKey, FSettingData> InitialValues;
	const bool bSuccess = PrepareSettings(SettingsToRegister, ApplyOrder, InitialValues);
	for (const FFullSettingKey& SettingKey : ApplyOrder)
	{
		if (InitialValues.Contains(SettingKey))
		{
			continue;
		}

		const FString InitialValue = GetCurrentDefaultValue(SettingKey);
		if (!InitialValue.IsEmpty())
		{
			InitialValues.Add(SettingKey, FSettingData{ InitialValue });
		}
	}

	ApplySettings(ApplyOrder, InitialValues, ESettingAppliedValueChangeSource::Initialization);
	TryBindRegisteredSettings();
	return bSuccess;
}

bool USettingRegistry::PrepareSettings(const TMap<FFullSettingKey, USettingType*>& SettingsToRegister,
	TArray<FFullSettingKey>& OutApplyOrder, TMap<FFullSettingKey, FSettingData>& OutExplicitValues)
{
	// Preparation owns all registration and capture work but leaves target application to its caller.
	OutApplyOrder.Reset();
	OutExplicitValues.Reset();
	for (const auto& KeyValue : SettingsToRegister)
	{
		USettingType* SettingAsset = KeyValue.Value;
		if (!ensure(SettingAsset))
		{
			return false;
		}
	}

	// Controlled-key declarations contribute concrete settings to the same batch as explicitly discovered keys.
	TMap<FFullSettingKey, USettingType*> ExpandedSettings = SettingsToRegister;
	TArray<TPair<FFullSettingKey, USettingType*>> SettingsToInspect;
	SettingsToInspect.Reserve(ExpandedSettings.Num());
	for (const auto& KeyValue : ExpandedSettings)
	{
		SettingsToInspect.Emplace(KeyValue.Key, KeyValue.Value);
	}

	// Process inferred owners as well so controller chains are materialized in one finite pass.
	for (int32 SettingIndex = 0; SettingIndex < SettingsToInspect.Num(); ++SettingIndex)
	{
		const FFullSettingKey SettingKey = SettingsToInspect[SettingIndex].Key;
		USettingType* SettingType = SettingsToInspect[SettingIndex].Value;
		for (const FPartialSettingKey& ControlledKey : SettingType->ControlledSettingKeys)
		{
			if (!ControlledKey.IsValid())
			{
				continue;
			}

			const FFullSettingKey FullControlledKey = ResolveSettingKey(ControlledKey);
			if (IsSettingRegistered(FullControlledKey) || ExpandedSettings.Contains(FullControlledKey))
			{
				continue;
			}

			// A compatible type must contain the key and satisfy any explicit child allowlist.
			TArray<USettingType*> OwnerCandidates;
			for (USettingType* Candidate : SettingAssets)
			{
				if (!IsValid(Candidate) || !Candidate->bEnabled)
				{
					continue;
				}

				const FFullSettingKey CandidateKey = GetSettingKey(Candidate);
				if (FullControlledKey != CandidateKey && !FullControlledKey.IsChildOf(CandidateKey))
				{
					continue;
				}

				const TArray<FFullSettingKey> AllowedChildKeys = GetAllowedChildKeys(Candidate);
				if (!AllowedChildKeys.IsEmpty() && !AllowedChildKeys.Contains(FullControlledKey))
				{
					continue;
				}

				OwnerCandidates.AddUnique(Candidate);
			}

			USettingType* ControlledOwner = ResolveKeyOwner(this, FullControlledKey, OwnerCandidates);
			if (!ControlledOwner)
			{
				UE_LOGFMT(LogAutoSettings, Warning,
					"No enabled SettingType can own controlled setting '{0}' referenced by '{1}'",
					FullControlledKey.ToString(), SettingKey.ToString());
				continue;
			}

			if (IsCategorySetting(FullControlledKey, ControlledOwner))
			{
				continue;
			}

			ExpandedSettings.Add(FullControlledKey, ControlledOwner);
			SettingsToInspect.Emplace(FullControlledKey, ControlledOwner);
		}
	}

	// Every inferred owner participates in normal type registration, binding, hydration, and ordering.
	for (const auto& KeyValue : ExpandedSettings)
	{
		USettingType* SettingAsset = KeyValue.Value;
		if (IsSettingTypeRegistered(SettingAsset))
		{
			continue;
		}

		if (!SettingAsset->bEnabled)
		{
			UE_LOGFMT(LogAutoSettings, Warning,
				"Skipping late registration for disabled SettingType '{0}' while registering '{1}'",
				GetNameSafe(SettingAsset), KeyValue.Key.ToString());
			return false;
		}

		RegisterSettingType(SettingAsset);
	}

	bool bSuccess = true;
	for (const auto& KeyValue : ExpandedSettings)
	{
		const FFullSettingKey& SettingKey = KeyValue.Key;
		USettingType* SettingAsset = KeyValue.Value;

		if (IsSettingRegistered(SettingKey))
		{
			// Existing keys participate so late hydration and dependency ordering remain batch-based.
			if (const FRegisteredSetting* ExistingSetting = RegisteredSettings.Find(SettingKey))
			{
				ensureMsgf(ExistingSetting->SettingType == SettingAsset,
					TEXT("Setting '%s' was requested with setting type '%s' but is already registered with '%s'"),
					*SettingKey.ToString(),
					*GetNameSafe(SettingAsset),
					*GetNameSafe(ExistingSetting->SettingType));
			}
			OutApplyOrder.Add(SettingKey);
			continue;
		}

		if (RegisterSetting(SettingKey, SettingAsset))
		{
			// Binding captures the target before targets-captured listeners can mutate it.
			TryBindSetting(SettingKey);

			FString DefaultValue;
			if (FindDefaultValueForSetting(SettingAsset, SettingKey, DefaultValue)
				&& !SetInitialDefaultValue(SettingKey, DefaultValue))
			{
				UE_LOGFMT(LogAutoSettings, Warning, "Failed to set default for '{0}'", SettingKey.ToString());
				bSuccess = false;
			}

			OutApplyOrder.Add(SettingKey);
		}
		else
		{
			UE_LOGFMT(LogAutoSettings, Error, "Failed to register setting '{0}' of type '{1}'", SettingKey.ToString(), GetNameSafe(SettingAsset));
			bSuccess = false;
		}
	}

	// Prepare only persisted and configured overrides; live target fallback is resolved after prerequisites execute.
	for (const FFullSettingKey& SettingKey : OutApplyOrder)
	{
		if (!ensure(RegisteredSettings.Contains(SettingKey)))
		{
			bSuccess = false;
			continue;
		}

		const USettingType* SettingType = RegisteredSettings[SettingKey].SettingType;
		if (!ensure(SettingType && SettingType->PersistenceStrategy))
		{
			bSuccess = false;
			continue;
		}

		const FSettingData SavedData = SettingType->PersistenceStrategy->GetSavedValue({ SettingKey, this });
		if (SavedData.IsValid() && !SavedData.Value.IsEmpty())
		{
			OutExplicitValues.Add(SettingKey, SavedData);
			continue;
		}

		FString ConfiguredDefault;
		if (SettingType->bOverrideDefaultValue)
		{
			ConfiguredDefault = SettingType->DefaultValue;
		}
		else
		{
			FindDefaultValueForSetting(SettingType, SettingKey, ConfiguredDefault);
		}

		if (!ConfiguredDefault.IsEmpty())
		{
			OutExplicitValues.Add(SettingKey, FSettingData{ ConfiguredDefault });
		}
	}

	// The key array is the ordering authority; maps are used only for keyed value lookup.
	SortSettings(this, OutApplyOrder);
	return bSuccess;
}

void USettingRegistry::TryRegisterPendingSettings()
{
	auto SettingsToRegister = SettingsPendingRegister;

	for (const auto& KeyValue : SettingsToRegister)
	{
		if (IsSettingRegistered(KeyValue.Key))
		{
			continue;
		}

		if (!ensure(KeyValue.Value))
		{
			return;
		}
	}

	RegisterSettings(SettingsToRegister);

	for (const auto& KeyValue : SettingsToRegister)
	{
		SettingsPendingRegister.Remove(KeyValue.Key);
	}
}

void USettingRegistry::TryBindRegisteredSettings()
{
	for (const auto& RegisteredSettingPair : RegisteredSettings)
	{
		TryBindSetting(RegisteredSettingPair.Key);
	}
}

bool USettingRegistry::ShouldRegisterControlledSetting(const FRegisteredSetting& Setting, const FRegisteredSetting& ControlledSetting)
{
	for (auto& SettingTypeControlledKey : Setting.SettingType->ControlledSettingKeys)
	{
		if (!SettingTypeControlledKey.IsValid())
		{
			continue;
		}

		if (ControlledSetting.Key.EndsWith(SettingTypeControlledKey))
		{
			return true;
		}
	}

	FSettingControlResult ControlsOtherSetting;
	OnCheckControlRelationship.Broadcast(Setting, ControlledSetting, ControlsOtherSetting);
	return ControlsOtherSetting.IsControlled();
}

void USettingRegistry::ConditionalRegisterControlledSetting(const FRegisteredSetting& Setting, const FRegisteredSetting& ControlledSetting)
{
	const bool bIsControlled = ShouldRegisterControlledSetting(Setting, ControlledSetting);
	UE_LOGFMT(LogAutoSettings, VeryVerbose, "Controlled Setting: {0}: '{1}' -> '{2}'", bIsControlled, Setting.Key.ToString(), ControlledSetting.Key.ToString());
	if (bIsControlled)
	{
		RegisterControlledSetting(Setting.Key, ControlledSetting.Key);
	}
}

void USettingRegistry::ApplySettings(const TArray<FFullSettingKey>& ApplyOrder,
	const TMap<FFullSettingKey, FSettingData>& ExplicitValues, ESettingAppliedValueChangeSource Source)
{
	// Apply controllers before controlled settings so later child overrides are authoritative.
	for (const FFullSettingKey& SettingKey : ApplyOrder)
	{
		FRegisteredSetting* RegisteredSetting = RegisteredSettings.Find(SettingKey);
		if (!ensure(RegisteredSetting && RegisteredSetting->SettingType))
		{
			continue;
		}

		const USettingType* SettingType = RegisteredSetting->SettingType;
		if (const FSettingData* ExplicitValue = ExplicitValues.Find(SettingKey))
		{
			UE_LOGFMT(LogAutoSettings, Log, "Applying setting key: {0}, asset: {1}", SettingKey.ToString(),
				SettingType->GetName());
			SetSettingInternal(SettingKey, ExplicitValue->Value, ESettingAction::ApplyOnly, Source);
			continue;
		}

		// A handler or earlier controller owns the no-override target state; synchronize it without replaying capture.
		if (RegisteredSetting->TargetBindingState != ETargetBindingState::Bound || !SettingType->BindingStrategy)
		{
			continue;
		}

		const FSettingReference Setting{ SettingKey, FSettingContext(this) };
		const FString TargetValue = SettingType->BindingStrategy->GetTargetValue(Setting);
		if (!TargetValue.IsEmpty())
		{
			UpdateAppliedValue(SettingKey, TargetValue, Source);
		}
	}
}

bool USettingRegistry::IsActiveBinding(const USettingBinding* Binding, const FFullSettingKey& Key) const
{
	const FRegisteredSetting* RegisteredSetting = RegisteredSettings.Find(Key);
	return RegisteredSetting && RegisteredSetting->TargetBindingState == ETargetBindingState::Bound
		&& RegisteredSetting->Binding == Binding && Binding && Binding->IsActive();
}

void USettingRegistry::HandleBindingTargetValueChanged(USettingBinding* Binding)
{
	if (!Binding)
	{
		return;
	}

	const FSettingReference Setting = Binding->GetSetting();
	if (!IsActiveBinding(Binding, Setting.Key))
	{
		return;
	}

	const USettingType* SettingAsset = FindSettingTypeForKey(Setting.Key);
	if (!ensure(SettingAsset && SettingAsset->BindingStrategy))
	{
		return;
	}

	const FString OldValue = RegisteredSettings[Setting.Key].AppliedValue.Get(FString());
	const FString TargetValue = SettingAsset->BindingStrategy->GetTargetValue(Setting);
	UE_LOGFMT(LogAutoSettings, Verbose, "TargetValueChanged for '{0}': '{1}' -> '{2}'",
		Setting.Key.ToString(), OldValue, TargetValue);
	// External targets can drift independently of the registry. Pull that value back in so the
	// registry remains the single source of truth for future readers and subscribers.
	const bool bAppliedValueChanged = UpdateAppliedValue(Setting.Key, TargetValue, ESettingAppliedValueChangeSource::TargetSync);
	if (!bAppliedValueChanged)
	{
		UE_LOGFMT(LogAutoSettings, Verbose, "TargetValueChanged for '{0}' was a no-op - ControllerSetDefaultValue NOT updated",
			Setting.Key.ToString());
		return;
	}

	if (InProgressSettingApplications.Num() > 0)
	{
		// If there's any settings currently being applied
		ensure(InProgressSettingApplications.Num() == 1);
		const FFullSettingKey InProgressSetting = InProgressSettingApplications[0];
		if (Setting.Key != InProgressSetting)
		{
			const TArray<FFullSettingKey> ControlledSettings = GetControlledSettings(InProgressSetting);
			if (!ControlledSettings.Contains(Setting.Key))
			{
				UE_LOGFMT(LogAutoSettings, Verbose,
					"TargetValueChanged for '{0}' occurred during '{1}' but there is no control relationship - ControllerSetDefaultValue NOT updated",
					Setting.Key.ToString(), InProgressSetting.ToString());
				return;
			}

			// A child setting is being applied
			// Update the contextual default value of the child setting to the current value that was set by the parent
			FString& Default = RegisteredSettings[Setting.Key].ControllerSetDefaultValue;
			const FString OldDefault = Default;
			Default = TargetValue;
			UE_LOGFMT(LogAutoSettings, Verbose, "Updated ControllerSetDefaultValue for '{0}': '{1}' -> '{2}' (triggered by controller '{3}')",
				Setting.Key.ToString(), OldDefault, Default, InProgressSetting.ToString());
		}
	}
	else
	{
		UE_LOGFMT(LogAutoSettings, Verbose, "TargetValueChanged for '{0}' but no in-progress application - ControllerSetDefaultValue NOT updated",
			Setting.Key.ToString());
	}
}

void USettingRegistry::HandleBindingInvalidated(USettingBinding* Binding)
{
	if (!Binding)
	{
		return;
	}

	const FSettingReference Setting = Binding->GetSetting();
	if (!IsActiveBinding(Binding, Setting.Key))
	{
		return;
	}

	UE_LOGFMT(LogAutoSettings, Verbose, "Binding invalidated for '{0}', rebinding", Setting.Key.ToString());
	UnbindSetting(Setting.Key, ETargetBindingState::Pending);
	TryBindSetting(Setting.Key);
}

bool USettingRegistry::UpdateAppliedValue(const FFullSettingKey& Key, const FString& Value, ESettingAppliedValueChangeSource Source)
{
	if (!ensure(IsSettingRegistered(Key)))
	{
		return false;
	}

	FRegisteredSetting& RegisteredSetting = RegisteredSettings[Key];
	if (RegisteredSetting.AppliedValue.IsSet() && RegisteredSetting.AppliedValue.GetValue() == Value)
	{
		return false;
	}

	FSettingAppliedValueChangedEvent Event;
	Event.Setting = { Key, FSettingContext(this) };
	Event.OldValue = RegisteredSetting.AppliedValue.Get(FString());
	Event.NewValue = Value;
	Event.Source = Source;

	RegisteredSetting.AppliedValue = Value;

	if (RegistryState == ESettingRegistryState::Initialized)
	{
		BroadcastAppliedValueChanged(Event);
	}

	return true;
}

void USettingRegistry::BroadcastAppliedValueChanged(const FSettingAppliedValueChangedEvent& Event)
{
	OnAppliedValueChangedNative.Broadcast(Event);
	OnAppliedValueChanged.Broadcast(Event);
	DispatchAppliedValueSubscriptions(Event);
}

void USettingRegistry::DispatchAppliedValueSubscriptions(const FSettingAppliedValueChangedEvent& Event)
{
	CleanupAppliedValueSubscriptions(Event.Setting.Key);

	if (TArray<FAppliedValueSubscription>* Subscriptions = AppliedValueSubscriptions.Find(Event.Setting.Key))
	{
		// Iterate over a copy so callbacks can safely unsubscribe/re-register while we dispatch.
		const TArray<FAppliedValueSubscription> SubscriptionsCopy = *Subscriptions;
		for (const FAppliedValueSubscription& Subscription : SubscriptionsCopy)
		{
			if (!Subscription.IsValid())
			{
				continue;
			}

			Subscription.Execute(Event.Setting, Event.NewValue);
		}
	}
}

void USettingRegistry::CleanupAppliedValueSubscriptions()
{
	for (auto It = AppliedValueSubscriptions.CreateIterator(); It; ++It)
	{
		It.Value().RemoveAll([](const FAppliedValueSubscription& Subscription)
			{
				return !Subscription.IsValid();
			});
		if (It.Value().Num() == 0)
		{
			It.RemoveCurrent();
		}
	}
}

void USettingRegistry::CleanupAppliedValueSubscriptions(const FFullSettingKey& Key)
{
	if (TArray<FAppliedValueSubscription>* Subscriptions = AppliedValueSubscriptions.Find(Key))
	{
		Subscriptions->RemoveAll([](const FAppliedValueSubscription& Subscription)
			{
				return !Subscription.IsValid();
			});
		if (Subscriptions->Num() == 0)
		{
			AppliedValueSubscriptions.Remove(Key);
		}
	}
}

void USettingRegistry::PushInProgressApplication(const FFullSettingKey& Setting)
{
	ensure(InProgressSettingApplications.Num() == 0);
	InProgressSettingApplications.Add(Setting);
	UE_LOGFMT(LogAutoSettings, Verbose, "[SCOPED] Starting in-progress application for '{0}'", Setting.ToString());
}

void USettingRegistry::PopInProgressApplication()
{
	ensure(InProgressSettingApplications.Num() == 1);
	const FFullSettingKey PoppedSetting = InProgressSettingApplications.Last();
	InProgressSettingApplications.RemoveAt(InProgressSettingApplications.Num() - 1);
	UE_LOGFMT(LogAutoSettings, Verbose, "[SCOPED] Ending in-progress application for '{0}'", PoppedSetting.ToString());
}

void USettingRegistry::RestoreInitialValues()
{
	// Restore initial setting values
	// This is so when a PIE session ends, the CVars are restored to their original values, which is important because they could be stored again as the initial values in the next session

	// Should only run in editor
	check(!IsRunningGame());

	TArray<FFullSettingKey> RemainingSettings;
	for (const auto& RegisteredSettingPair : RegisteredSettings)
	{
		if (RegisteredSettingPair.Value.TargetBindingState == ETargetBindingState::Bound && RegisteredSettingPair.Value.AppliedValue.IsSet())
		{
			// Only settings with an active target binding can be meaningfully restored back onto a target.
			RemainingSettings.Add(RegisteredSettingPair.Key);
		}
	}

	FFullSettingKey CurrentSettingBeingApplied;

	// Register a delegate so that we know when child settings are modified while restoring initials
	const FDelegateHandle AppliedCallbackHandle = OnAppliedValueChangedNative.AddWeakLambda(this,
		[this, &RemainingSettings, &CurrentSettingBeingApplied](const FSettingAppliedValueChangedEvent& AppliedValueChangedEvent)
		{
			const FFullSettingKey ModifiedSettingKey = AppliedValueChangedEvent.Setting.Key;
			if (ModifiedSettingKey != CurrentSettingBeingApplied)
			{
				// A child setting was modified as a result of restoring the parent setting
				if (!RemainingSettings.Contains(ModifiedSettingKey) && GetAppliedValue(ModifiedSettingKey) != RegisteredSettings[ModifiedSettingKey].InitialDefaultValue)
				{
					// The child is no longer in the queue (was already restored) but its value now differs from its default
					// Add the child setting to the queue so it is restored to its real initial value again
					RemainingSettings.AddUnique(ModifiedSettingKey);
				}
			}
		});

	// Process queue and apply
	while (RemainingSettings.Num() > 0)
	{
		CurrentSettingBeingApplied = RemainingSettings[0];
		const auto& RegisteredSetting = RegisteredSettings[CurrentSettingBeingApplied];
		const USettingType* Asset = RegisteredSetting.SettingType;
		FString CurrentValue = RegisteredSetting.AppliedValue.Get(FString());
		if (RegisteredSetting.TargetBindingState == ETargetBindingState::Bound && Asset && Asset->BindingStrategy)
		{
			CurrentValue = Asset->BindingStrategy->GetTargetValue({ CurrentSettingBeingApplied, this });
			UpdateAppliedValue(CurrentSettingBeingApplied, CurrentValue, ESettingAppliedValueChangeSource::TargetSync);
		}

		if (CurrentValue != RegisteredSetting.InitialDefaultValue)
		{
			UE_LOGFMT(LogAutoSettings, Log, "Restoring initial value of setting key: {0}, asset: {1}",
				CurrentSettingBeingApplied.ToString(), Asset->GetName());
			SetSettingInternal(CurrentSettingBeingApplied, RegisteredSetting.InitialDefaultValue, ESettingAction::ApplyOnly, ESettingAppliedValueChangeSource::Initialization);
		}
		RemainingSettings.RemoveAt(0);
	}

	OnAppliedValueChangedNative.Remove(AppliedCallbackHandle);

	UE_LOGFMT(LogAutoSettings, Log, "Finished restoring initial applied values");
}

void USettingRegistry::UpdateControlledSettingsDefaults(const FFullSettingKey& ParentKey)
{
	// Only update if we're within a scoped application (the parent is being applied)
	if (InProgressSettingApplications.Num() == 0)
	{
		return;
	}

	const TArray<FFullSettingKey> ControlledSettings = GetControlledSettings(ParentKey);

	for (const FFullSettingKey& ControlledKey : ControlledSettings)
	{
		if (RegisteredSettings.Contains(ControlledKey))
		{
			if (!RegisteredSettings[ControlledKey].AppliedValue.IsSet())
			{
				// Controlled settings that have not resolved an applied value yet do not participate in
				// contextual default propagation.
				continue;
			}

			FString CurrentValue = RegisteredSettings[ControlledKey].AppliedValue.GetValue();
			const USettingType* ControlledType = FindSettingTypeForKey(ControlledKey);
			if (RegisteredSettings[ControlledKey].TargetBindingState == ETargetBindingState::Bound && ControlledType && ControlledType->BindingStrategy)
			{
				CurrentValue = ControlledType->BindingStrategy->GetTargetValue({ ControlledKey, this });
				UpdateAppliedValue(ControlledKey, CurrentValue, ESettingAppliedValueChangeSource::TargetSync);
			}

			// Update the ControllerSetDefaultValue to reflect the new baseline set by the controller
			FString& Default = RegisteredSettings[ControlledKey].ControllerSetDefaultValue;
			const FString OldDefault = Default;
			Default = CurrentValue;

			UE_LOGFMT(LogAutoSettings, Verbose, "Updated ControllerSetDefaultValue for controlled setting '{0}': '{1}' -> '{2}' (controller: '{3}')",
				ControlledKey.ToString(), OldDefault, CurrentValue, ParentKey.ToString());
		}
	}
}

bool USettingRegistry::HasAppliedValue(const FFullSettingKey& Key) const
{
	if (!ensure(IsSettingRegistered(Key)))
	{
		return false;
	}

	return RegisteredSettings[Key].AppliedValue.IsSet();
}

bool USettingRegistry::IsSettingTypeRegistered(const USettingType* SettingType) const
{
	return RegisteredSettingTypes.Contains(SettingType);
}

void USettingRegistry::RegisterSettingType(USettingType* SettingType)
{
	if (!ensure(SettingType))
	{
		return;
	}

	if (!ensure(!IsSettingTypeRegistered(SettingType)))
	{
		UE_LOGFMT(LogAutoSettings, Error, "SettingType '{0}' is already registered - this indicates a logic error",
			SettingType->GetName());
		return;
	}

	UE_LOGFMT(LogAutoSettings, Log, "Registering SettingType '{0}' with {1} default providers",
		SettingType->GetName(), SettingType->DefaultProviders.Num());

	// Mark this SettingType as registered
	RegisteredSettingTypes.Add(SettingType);

	UE_LOGFMT(LogAutoSettings, Log, "Getting default values for SettingType '{0}'",
		SettingType->GetName());

	// Cache fully resolved keys per SettingType so any registration path can reuse the same
	// defaults without re-executing providers or adding separate initialization-only state.
	TMap<FFullSettingKey, FString> FullDefaults;
	for (const auto& DefaultPair : SettingType->GetDefaultValues())
	{
		const FPartialSettingKey& SettingTypeKey = DefaultPair.Key;
		const FString& Value = DefaultPair.Value;

		// Resolve to full key by prepending registry base key
		FFullSettingKey FullKey = ResolveSettingKey(SettingTypeKey);
		FullDefaults.Add(FullKey, Value);
		UE_LOGFMT(LogAutoSettings, Verbose, "Cached default for '{0}' = '{1}'",
			FullKey.ToString(), Value);
	}

	SettingTypeDefaultValues.Add(SettingType, MoveTemp(FullDefaults));

	UE_LOGFMT(LogAutoSettings, Log, "SettingType '{0}' generated {1} default settings",
		SettingType->GetName(), SettingTypeDefaultValues[SettingType].Num());
}
