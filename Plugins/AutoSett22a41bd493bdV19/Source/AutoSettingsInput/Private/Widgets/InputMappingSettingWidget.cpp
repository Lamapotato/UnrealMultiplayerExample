// Copyright Sam Bonifacio. All Rights Reserved.

#include "Widgets/InputMappingSettingWidget.h"

#include "Processing/AutoSettingsInputCapturePreProcessor.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingQuery.h"
#include "Utility/InputMappingUtils.h"
#include "MappingIssues/MappingIssueTypes.h"
#include "AutoSettingsInputConfig.h"
#include "AutoSettingsInputLogs.h"
#include "Framework/SettingRegistry.h"
#include "Widgets/KeyLabel.h"
#include "InputMappingContext.h"
#include "Framework/Application/SlateApplication.h"
#include "Core/InputKeyFilter.h"
#include "Core/AutoSettingsEnhancedInputUserSettings.h"

UInputMappingSettingWidget::UInputMappingSettingWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	const UAutoSettingsInputConfig* InputConfig = GetDefault<UAutoSettingsInputConfig>();
	// Widget defaults inherit legacy input config values; runtime capture uses local widget properties.
	EscapeKeys = InputConfig->GetDeprecatedBindingEscapeKeys();
	MouseMoveCaptureDistance = InputConfig->GetDeprecatedMouseMoveCaptureDistance();
	AllowedKeys = InputConfig->GetDeprecatedAllowedKeys();
	DisallowedKeys = InputConfig->GetDeprecatedDisallowedKeys();
}

void UInputMappingSettingWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	UpdateSettingKey();
}

void UInputMappingSettingWidget::NativeDestruct()
{
	if (InProgressIssueResolutionPromise)
	{
		// If there is an in-progress resolution, cancel it
		SetMappingIssueResolution(EMappingIssueResolution::Cancel);
	}

	Super::NativeDestruct();
}

void UInputMappingSettingWidget::SetMapping(FName InMappingName, EPlayerMappableKeySlot InKeySlot)
{
	MappingName = InMappingName;
	KeySlot = InKeySlot;
	UpdateSettingKey();
}

void UInputMappingSettingWidget::UpdateSettingKey()
{
	// Use the action name + key slot as the setting key extension
	auto NewKeyExtension = SB::AutoSettings::Input::PlayerMapping::GetMappingSettingSubKey(MappingName, KeySlot);
	FSettingSpecifier InputMappingSetting = Setting;
	InputMappingSetting.SubKey = NewKeyExtension;
	SetSetting(InputMappingSetting);
}

void UInputMappingSettingWidget::StartCapture()
{
	if (!ensure(!IsCapturing()))
	{
		// Already capturing
		return;
	}

	InputProcessor = MakeShared<FAutoSettingsInputCapturePreProcessor>();
	InputProcessor->OwningLocalPlayer = GetOwningPlayer()->GetLocalPlayer();
	InputProcessor->CaptureMode = CaptureMode;
	InputProcessor->EscapeKeys = EscapeKeys;
	InputProcessor->ClearKeys = ClearKeys;
	InputProcessor->MouseMoveCaptureDistance = MouseMoveCaptureDistance;
	InputProcessor->OnKeySelected.AddWeakLambda(this, [this](FCapturedInput CapturedInput)
		{
			const FCapturedInput ResolvedInput = ResolveCapturedInput(CapturedInput);

			if (!IsKeyAllowed(ResolvedInput.Key))
			{
				// Rejected inputs are policy filters; ending capture is optional.
				UE_LOG(LogAutoSettingsInput, Log, TEXT("BindCapturePrompt: Ignored key rejected by capture policy: %s"),
					*ResolvedInput.Key.ToString());

				if (GetCaptureRejectedAction(ResolvedInput.Key) == EInputCaptureRejectedAction::StopCapturing)
				{
					StopCapture();
				}

				return;
			}

			StopCapture();

			auto Subsystem = GetOwningLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

			const UWorld* World = GetOwningLocalPlayer() ? GetOwningLocalPlayer()->GetWorld() : nullptr;
			FEnhancedActionKeyMapping Mapping = SB::AutoSettings::Input::PlayerMapping::GetMapping(MappingName, World);
			if (!ensure(Mapping.GetMappingName() == MappingName))
			{
				// Couldn't find mapping
				return;
			}

			auto MappingContext = SB::AutoSettings::Input::PlayerMapping::GetMappingContext(MappingName, World);

			ensure(MappingContext);

			// Build a list of contexts to check collisions with
			TArray Contexts = { MappingContext };
			for (const auto& GlobalContext : GetDefault<UAutoSettingsInputConfig>()->GlobalMappingContexts)
			{
				if (GlobalContext.IsNull())
				{
					continue;
				}
				UInputMappingContext* LoadedContext = GlobalContext.LoadSynchronous();
				if (!LoadedContext)
				{
					continue;
				}
				Contexts.Add(LoadedContext);
			}

		// UE's built-in QueryMapKeyInContextSet has limitations:
		// It only checks default keys from mapping context assets, not player-remapped keys
		// (verified in UE 5.4 - GetMappings() returns defaults only, ignoring runtime remaps)
		// AutoSettings implements custom conflict detection to properly handle remapped keys
#if 0
		TArray<FMappingQueryIssue> MappingQueryIssues;
		EMappingQueryResult QueryResult = Subsystem->QueryMapKeyInContextSet(Contexts, MappingContext, Mapping.Action, CapturedInput.Chord.Key, MappingQueryIssues, EMappingQueryIssue::NoIssue);
#endif

			auto ExistingMappingsWithKey = SB::AutoSettings::Input::Player::GetCurrentMappingsInContextsWithKey(GetOwningLocalPlayer(),
				Contexts, ResolvedInput.Key);

			FMappingAttemptInfo MappingAttemptInfo;
			MappingAttemptInfo.DesiredKey = ResolvedInput.Key;

			// Determine directional intent for axis inputs - this is crucial for proper conflict detection
			// Two opposite directions of the same axis (e.g., gamepad stick up vs down) should not conflict
			bool bCapturedNegated = false;
			if (ResolvedInput.Key.IsAxis1D())
			{
				bCapturedNegated = (ResolvedInput.AxisScale < 0.0f); // 0.0f threshold for negative axis direction
			}

			for (const auto& ExistingMapping : ExistingMappingsWithKey)
			{
				if (ExistingMapping.Key.GetMappingName() == MappingName && ExistingMapping.Key.GetSlot() == KeySlot)
				{
					// Skip self
					continue;
				}

				// For axis inputs, check if the directional intent conflicts
				if (ResolvedInput.Key.IsAnalog())
				{
					// Get the directional intent of the existing mapping
					bool bExistingNegated = false;

					// Get the effective directional intent for the existing mapping
					const float ExistingScale = SB::AutoSettings::Input::PlayerMapping::GetEffectiveDirectionalScale(GetOwningLocalPlayer(), ExistingMapping.Key.GetMappingName());
					bExistingNegated = (ExistingScale < 0.0f); // 0.0f threshold for negative axis direction

					// Skip if the directional intents are different (different axis directions)
					if (bCapturedNegated != bExistingNegated)
					{
						continue;
					}
				}

				FMappingIssue Issue;
				Issue.Type = EMappingIssueType::KeyAlreadyMappedToAction;
				Issue.Context = ExistingMapping.Value;
				Issue.Mapping = ExistingMapping.Key;
				MappingAttemptInfo.Issues.Add(Issue);
			}

			if (MappingAttemptInfo.Issues.Num() > 0)
			{
				TWeakObjectPtr<ThisClass> WeakThis = this;
				DetermineResolutionAsync(MappingAttemptInfo).Then([WeakThis, ResolvedInput, MappingAttemptInfo](auto Future)
					{
						if (!WeakThis.IsValid())
						{
							return;
						}

						const EMappingIssueResolution Resolution = Future.Get();

						// Remove existing mappings if keeping new mappings only
						const bool bRemoveExistingMapping = Resolution == EMappingIssueResolution::Replace && MappingAttemptInfo.CanUnbindPreviousMappings();
						const bool bAddNewMapping = Resolution == EMappingIssueResolution::Replace || Resolution == EMappingIssueResolution::KeepAll;

						if (bRemoveExistingMapping)
						{
							// Remove the previous mapping(s)
							for (const auto& Issue : MappingAttemptInfo.Issues)
							{
								// Assemble the setting key for the previous mapping
								FFullSettingKey SettingKey = WeakThis->GetRegistry()->GetSettingKey(WeakThis->GetSettingType());
								SettingKey += SB::AutoSettings::Input::PlayerMapping::GetMappingSettingSubKey(Issue.Mapping.GetMappingName(),
									Issue.Mapping.GetSlot());

								// Apply the setting with special unbound FKey
								if (WeakThis->bAutoSave)
								{
									WeakThis->GetRegistry()->SetSetting(SettingKey, SB::AutoSettings::Input::Unbound.ToString(), ESettingAction::ApplyAndSave);
								}
								else
								{
									WeakThis->GetRegistry()->SetSetting(SettingKey, SB::AutoSettings::Input::Unbound.ToString(), ESettingAction::ApplyOnly);
								}
							}
						}

						if (bAddNewMapping)
						{
							WeakThis->BindKey(ResolvedInput);
						}
					});
			}
			else
			{
				BindKey(ResolvedInput);
			}

			OnChordCaptured.Broadcast(ResolvedInput);
		});
	InputProcessor->OnClearRequested.AddWeakLambda(this, [this]()
		{
			StopCapture();
			// Clear requests bypass validation and conflict resolution because they are commands, not candidate bindings.
			ClearMapping();

			FCapturedInput CapturedInput;
			CapturedInput.Key = SB::AutoSettings::Input::Unbound;
			CapturedInput.AxisScale = 1.f;
			OnChordCaptured.Broadcast(CapturedInput);
		});
	InputProcessor->OnCancelled.AddWeakLambda(this, [this]()
		{
			StopCapture();
		});
	InputProcessor->IsKeyAllowedDelegate.BindWeakLambda(this, [this](const FKey& Key)
		{
			return IsKeyAllowed(Key);
		});

	FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor, 0);

	ensure(IsCapturing());
	OnCapturingChanged.Broadcast(true);
}

void UInputMappingSettingWidget::StopCapture()
{
	if (!ensure(IsCapturing()))
	{
		// Not capturing
		return;
	}

	FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);
	InputProcessor = nullptr;

	ensure(!IsCapturing());
	OnCapturingChanged.Broadcast(false);
}

void UInputMappingSettingWidget::ClearMapping()
{
	// The Unbound sentinel suppresses the default mapping instead of behaving like an unset player override.
	const FString Value = SB::AutoSettings::Input::Unbound.ToString();
	ApplySettingValue(Value);
	UpdateSelection(Value);
}

bool UInputMappingSettingWidget::IsCapturing() const
{
	return InputProcessor.IsValid();
}

void UInputMappingSettingWidget::UpdateSelection_Implementation(const FString& Value)
{
	// Parse the extended format: "KeyName" or "KeyName:AxisScale"
	// Default to positive direction (1.0) when no AxisScale is specified
	FString KeyString = Value;
	float AxisScale = 1.0f; // 1.0f = positive axis direction (e.g., gamepad stick up/right)
	bool bHasAxisScale = false;

	FString Left, Right;
	if (Value.Split(TEXT(":"), &Left, &Right))
	{
		KeyString = Left;
		AxisScale = FCString::Atof(*Right);
		bHasAxisScale = true;
	}

	FKey Key = FKey(FName(KeyString));

	if (Key.IsAxis1D())
	{
		float DisplayScale = 1.0f; // Default positive direction for axis display
		if (bHasAxisScale)
		{
			// Use the stored axis scale for display
			DisplayScale = AxisScale;
		}
		else
		{
			// Fallback: get effective directional scale (stored metadata or original mapping design)
			DisplayScale = SB::AutoSettings::Input::PlayerMapping::GetEffectiveDirectionalScale(GetOwningLocalPlayer(), MappingName);
		}

		const FKey ButtonKey = GetDefault<UAutoSettingsInputConfig>()->GetAxisButton(Key, DisplayScale);
		if (ButtonKey.IsValid())
		{
			Key = ButtonKey;
		}
	}

	OnKeyChanged(Key);
}

FCapturedInput UInputMappingSettingWidget::ResolveCapturedInput(const FCapturedInput& CapturedInput) const
{
	FCapturedInput NormalizedInput = CapturedInput;

	const FKeyScale Axis = GetDefault<UAutoSettingsInputConfig>()->GetAxisKey(NormalizedInput.Key);
	if (Axis.Key.IsValid())
	{
		NormalizedInput.Key = Axis.Key;
		NormalizedInput.AxisScale = Axis.Scale;
	}

	return NormalizedInput;
}

void UInputMappingSettingWidget::OnKeyChanged_Implementation(const FKey& Key) {}

EInputCaptureRejectedAction UInputMappingSettingWidget::GetCaptureRejectedAction_Implementation(const FKey& PrimaryKey) const
{
	// Keep the default decision centralized behind the widget property.
	(void)PrimaryKey;
	return CaptureRejectedAction;
}

bool UInputMappingSettingWidget::IsKeyAllowed_Implementation(const FKey& PrimaryKey)
{
	if (AllowedKeys.Num() > 0)
	{
		bool bHasAllowedKey = false;

		for (const FKey& AllowedKey : AllowedKeys)
		{
			if (PrimaryKey == AllowedKey)
			{
				bHasAllowedKey = true;
				break;
			}
		}

		if (!bHasAllowedKey)
		{
			return false;
		}
	}

	for (const FKey& DisallowedKey : DisallowedKeys)
	{
		if (PrimaryKey == DisallowedKey)
		{
			return false;
		}
	}

	if (!KeyFilter)
	{
		return true;
	}

	return KeyFilter->MatchesKey(PrimaryKey);
}

void UInputMappingSettingWidget::OnEncounteredMappingIssue_Implementation(const FMappingAttemptInfo& Issues)
{
	// Default behavior
	// Immediately set the resolution
	if (Issues.CanUnbindPreviousMappings())
	{
		SetMappingIssueResolution(EMappingIssueResolution::Replace);
	}
	else
	{
		SetMappingIssueResolution(EMappingIssueResolution::KeepAll);
	}
}

void UInputMappingSettingWidget::SetMappingIssueResolution(EMappingIssueResolution Resolution)
{
	if (!InProgressIssueResolutionPromise)
	{
		UE_LOG(LogAutoSettingsInput, Error, TEXT("SetMappingIssueResolution failed, issue resolution not in progress"));
		return;
	}

	InProgressIssueResolutionPromise->SetValue(Resolution);

	InProgressIssueResolutionPromise = nullptr;
}

void UInputMappingSettingWidget::PostLoad()
{
	Super::PostLoad();
}

// #if WITH_EDITOR
// void UInputMappingSettingWidget::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
// {
// 	Super::PostEditChangeProperty(PropertyChangedEvent);
//
// 	UpdateSettingKey();
// }
// #endif

TFuture<EMappingIssueResolution> UInputMappingSettingWidget::DetermineResolutionAsync(const FMappingAttemptInfo& Issues)
{
	if (!ensure(InProgressIssueResolutionPromise == nullptr))
	{
		// Already waiting for a resolution?
		return MakeFulfilledPromise<EMappingIssueResolution>().GetFuture();
	}

	// Set the promise
	InProgressIssueResolutionPromise = MakeShared<TPromise<EMappingIssueResolution>>();

	auto Future = InProgressIssueResolutionPromise->GetFuture();

	OnEncounteredMappingIssue(Issues);

	// Return the future value
	return Future;
}

void UInputMappingSettingWidget::BindKey(const FCapturedInput& CapturedInput)
{
	// Pass the captured input information to the binding strategy
	// Format: "KeyName:AxisScale" so the binding strategy can determine directional intent
	FString Value = CapturedInput.Key.ToString();
	if (CapturedInput.Key.IsAxis1D())
	{
		Value += FString::Printf(TEXT(":%f"), CapturedInput.AxisScale);
	}

	UE_LOG(LogAutoSettingsInput, Log, TEXT("Binding key for mapping '%s': %s (AxisScale=%.2f)"),
		*MappingName.ToString(), *Value, CapturedInput.AxisScale);

	ApplySettingValue(Value);

	// For UI display, use the extended format if it contains axis scale info
	UpdateSelection(Value);
}
