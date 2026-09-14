// Copyright Sam Bonifacio. All Rights Reserved.

#include "Processing/InputModifierAxisDirection.h"

#include "EnhancedActionKeyMapping.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedPlayerInput.h"
#include "Engine/LocalPlayer.h"
#include "Core/AutoSettingsEnhancedInputUserSettings.h"
#include "Utility/EnhancedInputCompatibility.h"
#include "GameFramework/PlayerController.h"
#include "Utility/InputMappingUtils.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

FInputActionValue UInputModifierAxisDirection::ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput,
	FInputActionValue CurrentValue, float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(InputModifierDirectionalIntent_ModifyRaw);

	const FName MappingName = FindMappingName(PlayerInput);
	if (MappingName.IsNone())
	{
		return CurrentValue;
	}

	const SB::AutoSettings::Input::FEnhancedInputSubsystemData Data = SB::AutoSettings::Input::GetEnhancedInputSubsystemData(PlayerInput->GetOwningLocalPlayer());
	if (!Data.bIsValid)
	{
		return CurrentValue;
	}

	// Check stored directional intent first (player-customized)
	const float StoredScale = GetStoredDirectionalScale(Data.UserSettings, MappingName);
	if (StoredScale != 0.0f)
	{
		return CurrentValue * StoredScale;
	}

	// Fall back to default behavior
	return ApplyDefaultDirectionalLogic(PlayerInput, CurrentValue, MappingName);
}

FName UInputModifierAxisDirection::FindMappingName(const UEnhancedPlayerInput* PlayerInput) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(InputModifierDirectionalIntent_FindMappingName);

	// Return cached result if already searched
	if (CachedMappingName.IsSet())
	{
		return CachedMappingName.GetValue();
	}

	const auto Subsystem = PlayerInput->GetOuterAPlayerController()->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

	// Search through all player mappable key mappings to find which one contains this modifier instance
	// This is necessary because modifiers don't have direct back-references to their owning mappings
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(InputModifierDirectionalIntent_MappingSearch);

		TArray<FEnhancedActionKeyMapping> AllMappings;
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(InputModifierDirectionalIntent_GetAllPlayerMappableActionKeyMappings);
			AllMappings = Subsystem->GetAllPlayerMappableActionKeyMappings();
		}

		for (const auto& Mapping : AllMappings)
		{
			// Check modifiers from two sources:
			// 1. Static modifiers defined in the mapping asset
			// 2. Dynamic modifiers added at runtime by the input system
			TArray<UInputModifier*> AllModifiers = Mapping.Modifiers;

			{
				TRACE_CPUPROFILER_EVENT_SCOPE(InputModifierDirectionalIntent_GetActionModifiers);
				const auto ActionInstanceData = PlayerInput->FindActionInstanceData(Mapping.Action);
				AllModifiers.Append(ActionInstanceData->GetModifiers());
			}

			// Search for this specific modifier instance using pointer comparison
			for (const auto& Modifier : AllModifiers)
			{
				if (Modifier == this)
				{
					// Cache the result for future calls
					const FName FoundName = Mapping.GetMappingName();
					CachedMappingName = FoundName;
					return FoundName;
				}
			}
		}
	}

	// Cache the negative result to avoid repeated searches
	CachedMappingName = NAME_None;
	return NAME_None;
}

FKey UInputModifierAxisDirection::FindActuatingKey(const UEnhancedPlayerInput* PlayerInput) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(InputModifierDirectionalIntent_FindActuatingKey);

	const FName MappingName = FindMappingName(PlayerInput);
	if (MappingName.IsNone())
	{
		return EKeys::Invalid;
	}

	auto LocalPlayer = PlayerInput->GetOwningLocalPlayer();

	auto SubsystemData = SB::AutoSettings::Input::GetEnhancedInputSubsystemData(LocalPlayer);

	if (!ensure(SubsystemData.bIsValid))
	{
		return EKeys::Invalid;
	}

	auto CurrentKeyProfile = SB::AutoSettings::Input::GetActiveKeyProfile(SubsystemData.UserSettings);
	const FKeyMappingRow* MappingRow = nullptr;

	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FindKeyMappingRow);
		MappingRow = CurrentKeyProfile->FindKeyMappingRow(MappingName);
	}

	if (!MappingRow)
	{
		// If there is no Mapping Row, the keys have not been customized
		// Return the default key for the mapping
		FKey DefaultKey = SB::AutoSettings::Input::PlayerMapping::GetDefaultKey(LocalPlayer, MappingName);
		return DefaultKey;
	}

	return FindMostActiveKey(PlayerInput, MappingRow);
}

float UInputModifierAxisDirection::GetStoredDirectionalScale(UAutoSettingsEnhancedInputUserSettings* UserSettings, FName MappingName) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(InputModifierDirectionalIntent_GetStoredDirectionalScale);

	if (!UserSettings || !UserSettings->MappingDataMap.Contains(MappingName))
	{
		return 0.0f; // Indicates no stored scale found
	}

	const FMappingMetadata& Metadata = UserSettings->GetMappingMetadata(MappingName);
	return Metadata.bNegate ? SB::AutoSettings::Input::DirectionalScale_Negated : SB::AutoSettings::Input::DirectionalScale_Normal;
}

FInputActionValue UInputModifierAxisDirection::ApplyDefaultDirectionalLogic(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, FName MappingName) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(InputModifierDirectionalIntent_ApplyDefaultDirectionalLogic);

	// For default mappings: determine if this is the original axis for this mapping
	const FKey BoundKey = FindActuatingKey(PlayerInput);
	const FKey DefaultKey = SB::AutoSettings::Input::PlayerMapping::GetDefaultKey(PlayerInput->GetOwningLocalPlayer(), MappingName);

	// Apply directional negation for default axis keys when mapping context indicates backward/negative direction
	// This preserves Enhanced Input's original behavior where axis mappings can be marked as "negated"
	// (e.g., mapping "MoveForward" to negative mouse wheel or gamepad stick direction)
	if (BoundKey == DefaultKey && BoundKey.IsValid() && BoundKey.IsAxis1D())
	{
		const UWorld* World = PlayerInput->GetOwningLocalPlayer()->GetWorld();
		const bool bMappingIsForBackwardAction = SB::AutoSettings::Input::PlayerMapping::IsNegated(MappingName, World);
		if (bMappingIsForBackwardAction)
		{
			return CurrentValue * SB::AutoSettings::Input::DirectionalScale_Negated;
		}
	}
	// For non-default keys or non-axis inputs, pass through unchanged

	return CurrentValue;
}

FKey UInputModifierAxisDirection::FindMostActiveKey(const UEnhancedPlayerInput* PlayerInput, const FKeyMappingRow* MappingRow) const
{
	// Find which of the bound keys is responsible for this input
	// For example, if "Move Left" is bound to "Left Stick X Axis" as well as "D-Pad Left", we have to determine
	// which of those is triggering this modifier
	float BestValue = 0.f;
	FKey BestKey;
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(InputModifierDirectionalIntent_KeySearch);

		for (const auto& Mapping : MappingRow->Mappings)
		{
			const FKey Key = Mapping.GetCurrentKey();
			if (!Key.IsValid())
			{
				continue;
			}

			const auto KeyState = PlayerInput->GetKeyState(Key);
			if (!KeyState)
			{
				continue;
			}

			const float AbsValue = FMath::Abs(KeyState->RawValue.X);
			if (AbsValue > BestValue)
			{
				BestKey = Key;
				BestValue = AbsValue;
			}
		}
	}

	return BestKey;
}
