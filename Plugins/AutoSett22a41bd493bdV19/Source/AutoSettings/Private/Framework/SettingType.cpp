// Copyright Sam Bonifacio. All Rights Reserved.

#include "Framework/SettingType.h"

#include "Framework/SettingBindingStrategy.h"
#include "Framework/SettingDefaultProvider.h"
#include "Framework/SettingRegistry.h"
#include "AutoSettingsLogs.h"
#include "Logging/StructuredLog.h"

USettingType::USettingType(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PersistenceStrategy = ObjectInitializer.CreateDefaultSubobject<UConfigSettingPersistenceStrategy>(ObjectInitializer.GetObj(),
		TEXT("StorageStrategy"));
}

FString USettingType::GetValue(const FSettingReference& Setting, bool bPreferSavedValue) const
{
	FString SavedValue = PersistenceStrategy ? PersistenceStrategy->GetSavedValue(Setting).Value : FString();
	FString AppliedValue;

	if (Setting.Context.Registry && Setting.Context.Registry->IsSettingRegistered(Setting.Key))
	{
		// Once registered, the registry owns the authoritative in-session value.
		AppliedValue = Setting.Context.Registry->GetAppliedValue(Setting.Key);
	}
	else if (BindingStrategy)
	{
		// Before registration finishes, fall back to the live target so callers can still inspect
		// the current external state without forcing the registry to exist yet.
		AppliedValue = BindingStrategy->GetTargetValue(Setting);
	}

	FString Value = bPreferSavedValue ? SavedValue : AppliedValue;

	if (Value.IsEmpty())
	{
		Value = bPreferSavedValue ? AppliedValue : SavedValue;
	}

	return Value;
}

FPrimaryAssetId USettingType::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(SettingPrimaryAssetTypeName, GetFName());
}

void USettingType::PostLoad()
{
	UObject::PostLoad();

	if (!SettingKey_DEPRECATED.IsNone())
	{
		KeyFragment.Name = SettingKey_DEPRECATED;
		SettingKey_DEPRECATED = {};
	}

	if (!AllowedChildKeys_DEPRECATED.IsEmpty())
	{
		for (FName ChildKey : AllowedChildKeys_DEPRECATED)
		{
			AllowedChildKeyFragments.Add(ChildKey);
		}
		AllowedChildKeys_DEPRECATED.Empty();
	}

	if (!Dependencies_DEPRECATED.IsEmpty())
	{
		ControlledSettingKeys = Dependencies_DEPRECATED;
		Dependencies_DEPRECATED.Empty();
	}
}

void USettingType::Apply(const FSetSettingEvent& Event) const
{
	if (!bEnabled)
	{
		return;
	}

	if (!BindingStrategy)
	{
		return;
	}

	BindingStrategy->SyncToTarget(Event);
}

void USettingType::Save(const FSetSettingEvent& Event) const
{
	if (!bEnabled)
	{
		return;
	}

	if (!PersistenceStrategy)
	{
		return;
	}

	PersistenceStrategy->Save(Event);
}

FPartialSettingKey USettingType::GetPartialSettingKey() const
{
	FPartialSettingKey Key;
	TArray<FSettingKeyFragment> Fragments;

	// The supplied object owns the concrete leaf identity.
	if (!KeyFragment.IsNone())
	{
		Fragments.Add(KeyFragment);
	}

	// Ancestor class defaults contribute the inherited path in root-to-leaf order.
	for (UClass* CurrentClass = GetClass()->GetSuperClass();
		CurrentClass && CurrentClass->IsChildOf<USettingType>() && CurrentClass != USettingType::StaticClass();
		CurrentClass = CurrentClass->GetSuperClass())
	{
		const USettingType* CurrentSetting = GetDefault<USettingType>(CurrentClass);
		if (!CurrentSetting->KeyFragment.IsNone())
		{
			Fragments.Insert(CurrentSetting->KeyFragment, 0);
		}
	}

	// Materialize the ordered fragments into the semantic key type.
	for (const FSettingKeyFragment& Fragment : Fragments)
	{
		Key += Fragment;
	}

	return Key;
}

TMap<FPartialSettingKey, FString> USettingType::GetDefaultValues() const
{
	TMap<FPartialSettingKey, FString> Defaults;

	// Get this SettingType's base key (without registry base key)
	FPartialSettingKey BaseKey = GetPartialSettingKey();

	// 1. Start with static default value if configured
	if (bOverrideDefaultValue && !DefaultValue.IsEmpty())
	{
		// Use the SettingType's base key for the static default
		Defaults.Add(BaseKey, DefaultValue);
		UE_LOGFMT(LogAutoSettings, Verbose, "SettingType '{0}': Added static default value for key '{1}' = '{2}'",
			GetName(), BaseKey.ToString(), DefaultValue);
	}

	// 2. Execute dynamic providers (hardware benchmarking, etc.)
	// Later providers can override earlier ones by using the same complete key
	for (USettingDefaultProvider* Provider : DefaultProviders)
	{
		if (!IsValid(Provider) || !Provider->bEnabled)
		{
			continue;
		}

		UE_LOGFMT(LogAutoSettings, Log, "SettingType '{0}': Executing provider '{1}'",
			GetName(), Provider->GetClass()->GetName());

		TMap<FPartialSettingKey, FString> ProviderDefaults = Provider->GetDefaultValues();

		// Merge provider results, combining each fragment with this SettingType's base key
		for (const auto& ProviderPair : ProviderDefaults)
		{
			const FPartialSettingKey& ProviderFragment = ProviderPair.Key;
			const FString& Value = ProviderPair.Value;

			// Combine this SettingType's base key with the provider's fragment
			FPartialSettingKey CompleteKey = BaseKey;
			CompleteKey += ProviderFragment;

			if (Defaults.Contains(CompleteKey))
			{
				UE_LOGFMT(LogAutoSettings, Verbose, "SettingType '{0}': Provider '{1}' overriding existing default for key '{2}': '{3}' -> '{4}'",
					GetName(), Provider->GetClass()->GetName(), CompleteKey.ToString(), Defaults[CompleteKey], Value);
			}
			else
			{
				UE_LOGFMT(LogAutoSettings, Verbose, "SettingType '{0}': Provider '{1}' setting default for key '{2}' = '{3}'",
					GetName(), Provider->GetClass()->GetName(), CompleteKey.ToString(), Value);
			}

			Defaults.Add(CompleteKey, Value);
		}

		UE_LOGFMT(LogAutoSettings, Log, "SettingType '{0}': Provider '{1}' generated {2} defaults",
			GetName(), Provider->GetClass()->GetName(), ProviderDefaults.Num());
	}

	UE_LOGFMT(LogAutoSettings, Log, "SettingType '{0}': Total default values: {1}",
		GetName(), Defaults.Num());

	return Defaults;
}
