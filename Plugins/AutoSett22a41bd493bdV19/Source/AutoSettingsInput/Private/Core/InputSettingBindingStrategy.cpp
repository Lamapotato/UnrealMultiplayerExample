// Copyright Sam Bonifacio. All Rights Reserved.

#include "Core/InputSettingBindingStrategy.h"

#include "Core/AutoSettingsEnhancedInputUserSettings.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "Logging/StructuredLog.h"
#include "AutoSettingsInputLogs.h"
#include "Framework/SettingBinding.h"
#include "Framework/AutoSettingsPlayerSubsystem.h"
#include "Framework/SettingRegistry.h"
#include "Utility/InputMappingUtils.h"

bool UInputSettingBindingStrategy::IsBindingReady_Implementation(const FSettingContext& Context)
{
	// At design time, always allow registration so input settings can be shown in the editor
	if (Context.Registry && Context.Registry->bDesignTime)
	{
		return true;
	}

	// Must have local player to be able to register
	const ULocalPlayer* LocalPlayer = GetLocalPlayer(Context);
	if (!LocalPlayer)
	{
		return false;
	}

	// Must have a player controller to be able to register
	const APlayerController* PlayerController = LocalPlayer->GetPlayerController(nullptr);
	if (!IsValid(PlayerController))
	{
		return false;
	}

	// Must have Enhanced Input User Settings to be able to register
	auto UserSettings = GetEnhancedInputUserSettings(Context);
	if (!IsValid(UserSettings))
	{
		return false;
	}

	return true;
}

bool UInputSettingBindingStrategy::ValidateConfiguration_Implementation(const FSettingReference& Setting)
{
	// Input mapping settings must identify a concrete player-mappable mapping slot.
	if (!IsInputMappingKey(Setting))
	{
		return false;
	}

	// Mapping names are authored in Input Mapping Context assets, so reject stale or misspelled
	// setting keys before the apply path mutates Enhanced Input user settings.
	const FName MappingName = GetMappingName(Setting);
	return SB::AutoSettings::Input::PlayerMapping::GetMappingContext(MappingName) != nullptr;
}

void UInputSettingBindingStrategy::OnSyncToTarget_Implementation(const FSetSettingEvent& Event)
{
	if (!EnsureValidInputMappingKey(Event.Setting))
	{
		return;
	}

	const FInputMappingValue MappingValue = ParseInputValue(Event.Data.Value);

	auto UserSettings = GetEnhancedInputUserSettings(Event.Setting.Context);
	if (!ensure(UserSettings))
	{
		return;
	}

	const FName MappingName = GetMappingName(Event.Setting);
	const EPlayerMappableKeySlot KeySlot = GetKeySlot(Event.Setting);

	// Axis metadata and key remapping are stored in separate places. Apply metadata first so the
	// eventual key mapping sees the correct directional intent for 1D axes.
	ApplyAxisMetadata(UserSettings, MappingValue, MappingName);

	if (!EnsureMappingContextRegistered(UserSettings, MappingName, Event.Setting.Context))
	{
		return;
	}

	ApplyKeyMapping(UserSettings, MappingValue.Key, MappingName, KeySlot);
}

FString UInputSettingBindingStrategy::GetTargetValue_Implementation(const FSettingReference& Setting)
{
	if (!IsInputMappingKey(Setting))
	{
		return FString();
	}

	if (Setting.Context.Registry->bDesignTime)
	{
		return GetDesignTimeDefaultKey(Setting);
	}

	const FKey CurrentKey = SB::AutoSettings::Input::Player::GetCurrentKeyForMapping(
		GetLocalPlayer(Setting.Context), GetMappingName(Setting), GetKeySlot(Setting));

	if (CurrentKey.IsAxis1D())
	{
		return FormatAxisKeyWithScale(CurrentKey, Setting);
	}

	return CurrentKey.ToString();
}

void UInputSettingBindingStrategy::BeginBinding_Implementation(const FSettingReference& Setting, USettingBinding* Binding)
{
	Super::BeginBinding_Implementation(Setting, Binding);

	if (!IsInputMappingKey(Setting))
	{
		return;
	}

	if (Setting.Context.Registry->bDesignTime)
	{
		// Skip at design time as there is no local player
		return;
	}

	const auto UserSettings = GetEnhancedInputUserSettings(Setting.Context);
	if (!ensure(UserSettings))
	{
		return;
	}

	// Fire delegate when applied key is remapped
	UserSettings->OnKeyMappingUpdatedEvent.AddWeakLambda(Binding,
		[this, Setting, Binding](const FPlayerKeyMapping& KeyMapping)
		{
			if (KeyMapping.GetMappingName() == GetMappingName(Setting) && KeyMapping.GetSlot() == GetKeySlot(Setting))
			{
				Binding->NotifyTargetValueChanged();
			}
		});
}

void UInputSettingBindingStrategy::EndBinding_Implementation(const FSettingReference& Setting, USettingBinding* Binding)
{
	if (Setting.Context.Registry->bDesignTime)
	{
		// Skip at design time as there is no local player
		return;
	}

	const auto UserSettings = GetEnhancedInputUserSettings(Setting.Context);
	if (!ensure(UserSettings))
	{
		return;
	}

	UserSettings->OnKeyMappingUpdatedEvent.RemoveAll(Binding);

	Super::EndBinding_Implementation(Setting, Binding);
}

bool UInputSettingBindingStrategy::IsInputMappingKey(const FSettingReference& Setting) const
{
	if (Setting.Key.Fragments.Num() < 2)
	{
		return false;
	}

	return Setting.Key.GetLastFragment().Name.ToString().IsNumeric();
}

bool UInputSettingBindingStrategy::EnsureValidInputMappingKey(const FSettingReference& Setting) const
{
	if (!ensureMsgf(IsInputMappingKey(Setting),
			TEXT("Input setting '%s' is not a valid input mapping key. Expected a mapping key ending in a numeric slot fragment, e.g. '...:MoveForward:0'."),
			*Setting.Key.ToString()))
	{
		return false;
	}

	return true;
}

ULocalPlayer* UInputSettingBindingStrategy::GetLocalPlayer(const FSettingContext& Context) const
{
	const USettingRegistry* Registry = Context.Registry;
	if (!ensure(Registry))
	{
		return nullptr;
	}
	const UAutoSettingsPlayerSubsystem* Subsystem = Registry->GetTypedOuter<UAutoSettingsPlayerSubsystem>();
	if (!ensure(Subsystem))
	{
		return nullptr;
	}
	return Subsystem->GetLocalPlayer();
}

UEnhancedInputLocalPlayerSubsystem* UInputSettingBindingStrategy::GetEnhancedInputSubsystem(
	const FSettingContext& Context) const
{
	const SB::AutoSettings::Input::FEnhancedInputSubsystemData Data = SB::AutoSettings::Input::GetEnhancedInputSubsystemData(GetLocalPlayer(Context));
	return Data.Subsystem;
}

UAutoSettingsEnhancedInputUserSettings* UInputSettingBindingStrategy::GetEnhancedInputUserSettings(
	const FSettingContext& Context) const
{
	return UAutoSettingsEnhancedInputUserSettings::Get(GetLocalPlayer(Context));
}

FName UInputSettingBindingStrategy::GetMappingName(const FSettingReference& Setting) const
{
	// The second last node is the mapping name
	return Setting.Key.GetLastFragment(1).Name;
}

UInputSettingBindingStrategy::FInputMappingValue::FInputMappingValue(const FString& InputValue)
{
	FString KeyString = InputValue;

	FString Left, Right;
	if (InputValue.Split(TEXT(":"), &Left, &Right))
	{
		KeyString = Left;
		AxisScale = FCString::Atof(*Right);
	}

	Key = FKey(FName(KeyString));
	if (Key == SB::AutoSettings::Input::Unbound)
	{
		Key = EKeys::Invalid;
	}
}

FString UInputSettingBindingStrategy::FInputMappingValue::ToString() const
{
	if (AxisScale.IsSet() && Key.IsAxis1D())
	{
		const FString ScaleString = FString::Printf(TEXT("%g"), AxisScale.GetValue());
		return FString::Printf(TEXT("%s:%s"), *Key.ToString(), *ScaleString);
	}
	return Key.ToString();
}

UInputSettingBindingStrategy::FInputMappingValue UInputSettingBindingStrategy::ParseInputValue(const FString& InputValue) const
{
	return FInputMappingValue(InputValue);
}

void UInputSettingBindingStrategy::ApplyAxisMetadata(UAutoSettingsEnhancedInputUserSettings* UserSettings, const FInputMappingValue& MappingValue, const FName& MappingName) const
{
	if (!MappingValue.Key.IsValid())
	{
		return;
	}

	if (MappingValue.Key.IsAxis1D() && MappingValue.AxisScale.IsSet())
	{
		const bool bNegate = (MappingValue.AxisScale.GetValue() < 0.0f);

		FMappingMetadata Metadata;
		Metadata.bNegate = bNegate;
		UserSettings->SetMappingMetadata(MappingName, Metadata);

		UE_LOGFMT(LogAutoSettingsInput, Log, "Stored directional intent for axis mapping {0}: bNegate={1} (AxisScale={2})",
			MappingName, bNegate, MappingValue.AxisScale.GetValue());
	}
	else
	{
		UserSettings->ClearMappingMetadata(MappingName);

		UE_LOGFMT(LogAutoSettingsInput, Log, "Cleared directional intent for mapping {0} (IsAxis1D={1}, HasAxisScale={2})",
			MappingName, MappingValue.Key.IsAxis1D(), MappingValue.AxisScale.IsSet());
	}
}

bool UInputSettingBindingStrategy::EnsureMappingContextRegistered(UAutoSettingsEnhancedInputUserSettings* UserSettings, const FName& MappingName, const FSettingContext& Context) const
{
	const ULocalPlayer* LocalPlayer = GetLocalPlayer(Context);
	const UWorld* World = LocalPlayer ? LocalPlayer->GetWorld() : nullptr;
	const auto MappingContext = SB::AutoSettings::Input::PlayerMapping::GetMappingContext(MappingName, World);

	if (!ensureMsgf(MappingContext,
		TEXT("Input mapping '%s' reached apply without a registered mapping context."),
		*MappingName.ToString()))
	{
		// Validation rejects non-concrete or unresolved mapping keys before apply, so this path means
		// the internal application contract was bypassed or invalidated.
		return false;
	}

	if (!UserSettings->IsMappingContextRegistered(MappingContext))
	{
		UserSettings->RegisterInputMappingContext(MappingContext);
	}

	return true;
}

void UInputSettingBindingStrategy::ApplyKeyMapping(UAutoSettingsEnhancedInputUserSettings* UserSettings, const FKey& NewKey, const FName& MappingName, EPlayerMappableKeySlot KeySlot) const
{
	FMapPlayerKeyArgs Args;
	Args.MappingName = MappingName;
	Args.Slot = KeySlot;
	Args.NewKey = NewKey;

	FGameplayTagContainer FailureReason;

	UE_LOGFMT(LogAutoSettingsInput, Log, "Mapping key {0} to {1} slot {2}", NewKey.ToString(), MappingName, static_cast<uint8>(KeySlot));
	UserSettings->MapPlayerKey(Args, FailureReason);

	ensure(FailureReason.IsEmpty());
}

FString UInputSettingBindingStrategy::GetDesignTimeDefaultKey(const FSettingReference& Setting) const
{
	const FName MappingName = GetMappingName(Setting);
	const EPlayerMappableKeySlot KeySlot = GetKeySlot(Setting);

	const FKey DefaultKey = SB::AutoSettings::Input::PlayerMapping::GetDefaultKey(nullptr, MappingName, KeySlot);

	if (DefaultKey.IsValid())
	{
		return DefaultKey.ToString();
	}

	return {};
}

FString UInputSettingBindingStrategy::FormatAxisKeyWithScale(const FKey& AxisKey, const FSettingReference& Setting) const
{
	const FName MappingName = GetMappingName(Setting);
	const float AxisScale = SB::AutoSettings::Input::PlayerMapping::GetEffectiveDirectionalScale(
		GetLocalPlayer(Setting.Context), MappingName);

	FInputMappingValue Value;
	Value.Key = AxisKey;
	Value.AxisScale = AxisScale;

	return Value.ToString();
}

EPlayerMappableKeySlot UInputSettingBindingStrategy::GetKeySlot(const FSettingReference& Setting) const
{
	// The last node is the key slot
	const FName KeySlotName = Setting.Key.GetLastFragment().Name;
	int32 KeySlot = FCString::Atoi(*KeySlotName.ToString());
	return static_cast<EPlayerMappableKeySlot>(KeySlot);
}
