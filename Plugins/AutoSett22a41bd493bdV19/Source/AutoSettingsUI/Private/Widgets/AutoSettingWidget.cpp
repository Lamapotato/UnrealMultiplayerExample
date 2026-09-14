// Copyright Sam Bonifacio. All Rights Reserved.

#include "Widgets/AutoSettingWidget.h"
#include "AutoSettingsError.h"
#include "AutoSettingsLogs.h"
#if WITH_EDITOR
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#endif
#include "Components/NamedSlot.h"
#include "Engine/GameInstance.h"
#include "Logging/MessageLog.h"
#include "Logging/StructuredLog.h"
#include "Framework/AutoSettingsPlayerSubsystem.h"
#include "Framework/AutoSettingsSubsystem.h"
#include "Framework/SettingRegistry.h"
UAutoSettingWidget::UAutoSettingWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bAutoSave(true)
	, bAutoApply(true)
	, bUpdatingSettingSelection(false)
{
	bIsVariable = false;
	SetIsFocusable(true);
}

UAutoSettingWidget* UAutoSettingWidget::FindInNamedSlot(const UNamedSlot* NamedSlot)
{
	// Check direct children
	for (UWidget* Child : NamedSlot->GetAllChildren())
	{
		if (UAutoSettingWidget* SettingWidget = Cast<UAutoSettingWidget>(Child))
		{
			return SettingWidget;
		}
	}

	// Check further named slots recursively if nested
	for (const UWidget* Child : NamedSlot->GetAllChildren())
	{
		if (const UNamedSlot* ChildNamedSlot = Cast<UNamedSlot>(Child))
		{
			if (UAutoSettingWidget* ChildResult = FindInNamedSlot(ChildNamedSlot))
			{
				return ChildResult;
			}
		}
	}

	// No result
	return nullptr;
}

const USettingType* UAutoSettingWidget::GetSettingType() const
{
	return Setting.SettingType.GetDefaultObject();
}

void UAutoSettingWidget::SetSetting(const FSettingSpecifier& InSetting)
{
	if (!InSetting.SettingType)
	{
		if (!IsDesignTime())
		{
			LogMissingSetting();
		}
		return;
	}

	if (!IsDesignTime())
	{
		if (auto OldSettingType = GetSettingType())
		{
			if (USettingRegistry* OldRegistry = GetRegistry(false))
			{
				FFullSettingKey OldSettingKey = OldRegistry->GetSettingKey(OldSettingType);
				OldSettingKey += Setting.SubKey;
				OldRegistry->RemoveAppliedValueChangedCallback(this, OldSettingKey);
			}
		}
	}

	Setting = InSetting;

	USettingRegistry* Registry = GetRegistry();
	const FFullSettingKey FullSettingKey = GetFullSettingKey();

	// If the setting isn't registered, do so
	if (!Registry->IsSettingRegistered(FullSettingKey))
	{
		// Go through the batch API even for a single key so registration, defaults and
		// saved-value hydration stay in the same synchronous pipeline.
		TMap<FFullSettingKey, USettingType*> SettingsToRegister;
		SettingsToRegister.Add(FullSettingKey, const_cast<USettingType*>(GetSettingType()));
		if (!Registry->RegisterSettings(SettingsToRegister))
		{
			UE_LOGFMT(LogAutoSettings, Error, "Failed to register setting '{0}' for widget '{1}'", FullSettingKey.ToString(), GetNameSafe(this));
			return;
		}
	}

	ResetToSettingValue();

	if (!IsDesignTime())
	{
		FSettingAppliedValueCallback Delegate;
		Delegate.BindDynamic(this, &ThisClass::OnAppliedValueChanged);
		// Widgets intentionally listen to the registry, not the strategy target, so they stay in sync
		// with the authoritative value even when no live target currently exists.
		GetRegistry()->RegisterAppliedValueChangedCallback_BP(GetFullSettingKey(), Delegate, false);
	}
}

void UAutoSettingWidget::Apply()
{
	GetRegistry()->SetSetting(GetFullSettingKey(), CurrentValue, ESettingAction::ApplyOnly);
	bHasUnappliedChange = false;
}

FString UAutoSettingWidget::GetDefaultValue() const
{
	return GetRegistry()->GetCurrentDefaultValue(GetFullSettingKey());
}

bool UAutoSettingWidget::CanResetToDefault() const
{
	return CurrentValue != GetDefaultValue();
}

void UAutoSettingWidget::ResetToDefault()
{
	if (!CanResetToDefault())
	{
		return;
	}

	const FString DefaultValue = GetDefaultValue();
	DispatchUpdateSelection(DefaultValue);
	ApplySettingValue(DefaultValue, true);
}

void UAutoSettingWidget::Save()
{
	// Save through the registry so the value is applied and persisted in one operation.
	GetRegistry()->SetSetting(GetFullSettingKey(), CurrentValue, ESettingAction::ApplyAndSave);

	bHasUnappliedChange = false;
	bHasUnsavedChange = false;
}

void UAutoSettingWidget::Cancel()
{
	// We want to reset to the config value rather than the current CVar value in case we are reverting changes that were already applied
	ResetToSettingValue(true);

	const FString SavedValue = GetRegistry()->GetSettingValue(GetFullSettingKey(), ESettingValuePreference::Saved);
	GetRegistry()->SetSetting(GetFullSettingKey(), SavedValue, ESettingAction::ApplyOnly);

	bHasUnappliedChange = false;
	bHasUnsavedChange = false;
}

FString UAutoSettingWidget::GetDebugName() const
{
	FString DebugName = GetName();
	if (const USettingType* SettingAsset = GetSettingType())
	{
		if (USettingRegistry* Registry = GetRegistry(false))
		{
			FFullSettingKey FullKey = Registry->GetSettingKey(SettingAsset);
			FullKey += Setting.SubKey;
			DebugName += "[" + FullKey.ToString() + "]";
		}
		else
		{
			DebugName += "[" + SettingAsset->GetName() + "]";
		}
	}
	return DebugName;
}

void UAutoSettingWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	// Wait for the registry to initialize
	if (auto Registry = GetRegistry(false))
	{
		Registry->CallOrRegister_OnInitialized(FSimpleDelegate::CreateWeakLambda(this, [this]()
			{
				// At this point the registry is initialized

				SetSetting(Setting);
			}));
	}
}

void UAutoSettingWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UAutoSettingWidget::NativeDestruct()
{
	if (!IsDesignTime())
	{
		// If the registry is valid and initialized (it won't be if the session is ending)
		// remove the keyed applied value callback, otherwise the widget will still get callback events
		auto Registry = GetRegistry(false);
		if (Registry && Registry->IsInitialized())
		{
			Registry->RemoveAppliedValueChangedCallback(this, GetFullSettingKey());
		}
	}

	Super::NativeDestruct();
}

void UAutoSettingWidget::UpdateSelection_Implementation(const FString& Value) {}

void UAutoSettingWidget::ApplySettingValue(FString Value, bool bSaveIfPossible)
{
	// Don't try to apply or save in design time
	// Some widgets can potentially get here through PreConstruct
	if (IsDesignTime())
	{
		return;
	}

	if (!Setting.SettingType)
	{
		LogMissingSetting();
		return;
	}

	// The widget selection is being updated by this widget - don't treat it as a user-triggered change
	if (IsUpdatingSettingSelection())
	{
		return;
	}

	if (CurrentValue != Value)
	{
		CurrentValue = Value;
	}

	// Apply / save - regardless of previous setting value
	// Currently applied or saved value may be different anyway

	const bool bShouldSave = bAutoSave && bSaveIfPossible;

	// Determine the action based on widget settings
	ESettingAction Action;
	if (bAutoApply || bShouldSave)
	{
		Action = bShouldSave ? ESettingAction::ApplyAndSave : ESettingAction::ApplyOnly;
	}
	else
	{
		// Don't apply automatically, just track that we have an unapplied change
		bHasUnappliedChange = true;
		return;
	}

	// Apply through the registry so widget state stays synchronized with the setting pipeline.
	GetRegistry()->SetSetting(GetFullSettingKey(), CurrentValue, Action);

	// Update widget state
	bHasUnappliedChange = false;
	bHasUnsavedChange = (Action == ESettingAction::ApplyOnly);
}

FFullSettingKey UAutoSettingWidget::GetFullSettingKey() const
{
	const USettingType* Asset = GetSettingType();
	if (!ensure(Asset))
	{
		return {};
	}

	FFullSettingKey FullKey = GetRegistry()->GetSettingKey(Asset);
	FullKey += Setting.SubKey;
	return FullKey;
}

USettingRegistry* UAutoSettingWidget::GetRegistry(bool bChecked) const
{
	USettingRegistry* Registry = nullptr;

	// Use domain of the Setting Type, or fall back to Global
	ESettingDomain SettingDomain = ESettingDomain::Global;
	if (const auto SettingType = GetSettingType())
	{
		SettingDomain = SettingType->SettingDomain;
	}

	if (IsDesignTime())
	{
#if WITH_EDITOR
		// If design time, request an appropriate registry from the editor module
		if (GetDesignTimeRegistry.IsBound())
		{
			Registry = GetDesignTimeRegistry.Execute(SettingDomain);
		}

		if (!ensure(Registry))
		{
			UE_LOGFMT(LogAutoSettings, Error, "Failed to find design-time registry");
		}
#endif
	}
	else
	{
		if (SettingDomain == ESettingDomain::Player)
		{
			const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
			if (bChecked)
			{
				check(LocalPlayer);
			}
			if (LocalPlayer)
			{
				auto Subsystem = LocalPlayer->GetSubsystem<UAutoSettingsPlayerSubsystem>();
				if (bChecked)
				{
					check(Subsystem);
				}
				if (Subsystem)
				{
					Registry = Subsystem->GetPlayerSettingRegistry();
				}
			}
		}
		else
		{
			const UGameInstance* GameInstance = GetGameInstance();
			if (bChecked)
			{
				check(GameInstance);
			}
			if (GameInstance)
			{
				Registry = GameInstance->GetSubsystem<UAutoSettingsSubsystem>()->GetGameSettingRegistry();
			}
		}
	}

	if (bChecked)
	{
		check(Registry);
	}

	return Registry;
}

void UAutoSettingWidget::PostLoad()
{
	Super::PostLoad();
	MigrateDeprecatedProperties();
}

void UAutoSettingWidget::PostInitProperties()
{
	Super::PostInitProperties();
	MigrateDeprecatedProperties();
}

void UAutoSettingWidget::MigrateDeprecatedProperties()
{
	bool bMigrated = false;

	if (!SettingKey_DEPRECATED.IsNone())
	{
		SettingKeyExtension_DEPRECATED.Fragments.Add(SettingKey_DEPRECATED);
		SettingKey_DEPRECATED = NAME_None;
		bMigrated = true;
	}

	SettingKeyExtension_DEPRECATED.MigrateDeprecatedProperties();
	if (SettingKeyExtension_DEPRECATED.IsValid())
	{
		SubKey_DEPRECATED.Fragments = SettingKeyExtension_DEPRECATED.Fragments;
		SettingKeyExtension_DEPRECATED = {};
		bMigrated = true;
	}

	if (SettingType_DEPRECATED)
	{
		Setting.SettingType = SettingType_DEPRECATED;
		SettingType_DEPRECATED = nullptr;
		bMigrated = true;
	}

	if (SubKey_DEPRECATED.IsValid())
	{
		Setting.SubKey = SubKey_DEPRECATED;
		SubKey_DEPRECATED = {};
		bMigrated = true;
	}

	if (bMigrated)
	{
// The migration itself must still run at runtime, but the owning blueprint warning
// depends on editor-only metadata that is not available in cooked/runtime builds.
// Guard it so packaging and non-editor targets continue to compile cleanly.
#if WITH_EDITOR
		const UWidgetBlueprintGeneratedClass* OwningWidgetClass = GetTypedOuter<UWidgetBlueprintGeneratedClass>();
		const UObject* OwningWidgetBlueprint = OwningWidgetClass ? OwningWidgetClass->ClassGeneratedBy : nullptr;
		if (OwningWidgetBlueprint)
		{
			FMessageLog("LoadErrors").Warning(FText::Format(NSLOCTEXT("AutoSettings", "AutoSettingWidgetMigrated", "{0} migrated deprecated setting properties. Please compile and save owning widget blueprint {1}."), FText::FromString(GetPathName()), FText::FromString(GetNameSafe(OwningWidgetBlueprint))));
		}
#endif
	}
}

void UAutoSettingWidget::ResetToSettingValue(bool bPreferSavedValue)
{
	CurrentValue = bPreferSavedValue
		? GetRegistry()->GetSettingValue(GetFullSettingKey(), ESettingValuePreference::Saved)
		: GetRegistry()->GetAppliedValue(GetFullSettingKey());
	DispatchUpdateSelection(CurrentValue);
}

void UAutoSettingWidget::DispatchUpdateSelection(const FString& Value)
{
	bUpdatingSettingSelection = true;
	UpdateSelection(Value);
	bUpdatingSettingSelection = false;
}

void UAutoSettingWidget::OnAppliedValueChanged(const FSettingReference& SettingRef, const FString& Value)
{
	if (!ensure(SettingRef.Key == GetFullSettingKey()))
	{
		return;
	}

	if (HasUnappliedChange())
	{
		// If we have an unapplied change, the setting widget was purposefully desynchronized from the registry value and we should ignore updates
		return;
	}

	auto Registry = GetRegistry(false);
	if (!IsValid(Registry) || !Registry->IsInitialized())
	{
		// If we don't have an initialized registry, the game may be shutting down
		// Ignore the setting change in this case
		return;
	}

	UE_LOGFMT(LogAutoSettings, Log, "Widget '{0}' detected setting changed: {1}", GetNameSafe(this), SettingRef.Key.ToString());

	if (Value != CurrentValue)
	{
		CurrentValue = Value;
		DispatchUpdateSelection(CurrentValue);
	}
}

void UAutoSettingWidget::LogMissingSetting() const
{
	SB::AutoSettings::Errors::LogError(FString::Printf(TEXT("%s: Requires a valid Setting asset"), *GetDebugName()));
}

FSettingReference UAutoSettingWidget::GetSettingReference() const
{
	return { GetFullSettingKey(), GetSettingContext() };
}

FSettingContext UAutoSettingWidget::GetSettingContext() const
{
	USettingRegistry* Registry = GetRegistry();
	// Registry should always be initialized before using it as a context
	ensure(Registry->IsInitialized());
	return { Registry };
}
