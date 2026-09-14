// Copyright Sam Bonifacio. All Rights Reserved.

#include "Processing/InputModifierSettingScalar.h"

#include "AutoSettingsInputLogs.h"
#include "EnhancedPlayerInput.h"
#include "Engine/LocalPlayer.h"
#include "Framework/AutoSettingsPlayerSubsystem.h"
#include "Framework/SettingRegistry.h"

FInputActionValue UInputModifierSettingScalar::ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput,
	FInputActionValue CurrentValue, float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UInputModifierSettingScalar::ModifyRaw_Implementation);

	if (!PlayerInput)
	{
		return CurrentValue;
	}

	const FSettingKey ResolvedSettingKey = ResolveScalarSettingKey();
	if (!ResolvedSettingKey.IsValid())
	{
		return CurrentValue;
	}

	const float Scalar = ResolveScalar(PlayerInput);

	switch (CurrentValue.GetValueType())
	{
		case EInputActionValueType::Boolean:
			return CurrentValue;

		case EInputActionValueType::Axis1D:
			return bAffectX ? FInputActionValue(CurrentValue.Get<float>() * Scalar) : CurrentValue;

		case EInputActionValueType::Axis2D:
		{
			FVector2D Value = CurrentValue.Get<FVector2D>();
			if (bAffectX)
			{
				Value.X *= Scalar;
			}
			if (bAffectY)
			{
				Value.Y *= Scalar;
			}
			return FInputActionValue(Value);
		}

		case EInputActionValueType::Axis3D:
		{
			FVector Value = CurrentValue.Get<FVector>();
			if (bAffectX)
			{
				Value.X *= Scalar;
			}
			if (bAffectY)
			{
				Value.Y *= Scalar;
			}
			if (bAffectZ)
			{
				Value.Z *= Scalar;
			}
			return FInputActionValue(Value);
		}

		default:
			return CurrentValue;
	}
}

FSettingKey UInputModifierSettingScalar::ResolveScalarSettingKey() const
{
	return Setting.ResolveKey();
}

const USettingRegistry* UInputModifierSettingScalar::ResolveRegistry(const UEnhancedPlayerInput* PlayerInput,
	const FSettingKey& ResolvedSettingKey) const
{
	const ULocalPlayer* LocalPlayer = PlayerInput ? PlayerInput->GetOwningLocalPlayer() : nullptr;
	if (!LocalPlayer)
	{
		CachedLocalPlayer = nullptr;
		CachedRegistry = nullptr;
		CachedResolvedSettingKey = FSettingKey();
		return nullptr;
	}

	if (CachedLocalPlayer.Get() != LocalPlayer || CachedResolvedSettingKey != ResolvedSettingKey)
	{
		CachedLocalPlayer = LocalPlayer;
		CachedResolvedSettingKey = ResolvedSettingKey;
		CachedRegistry = nullptr;

		if (UAutoSettingsPlayerSubsystem* PlayerSubsystem = LocalPlayer->GetSubsystem<UAutoSettingsPlayerSubsystem>())
		{
			CachedRegistry = PlayerSubsystem->GetPlayerSettingRegistry();
		}
	}

	const USettingRegistry* Registry = CachedRegistry.Get();
	if (!Registry || !Registry->IsInitialized() || !Registry->IsSettingRegistered(ResolvedSettingKey))
	{
		return nullptr;
	}

	return Registry;
}

float UInputModifierSettingScalar::ResolveScalar(const UEnhancedPlayerInput* PlayerInput) const
{
	const FSettingKey ResolvedSettingKey = ResolveScalarSettingKey();
	if (!ResolvedSettingKey.IsValid())
	{
		return DefaultScalar;
	}

	const USettingRegistry* Registry = ResolveRegistry(PlayerInput, ResolvedSettingKey);
	if (!Registry)
	{
		return DefaultScalar;
	}

	const FString Value = Registry->GetAppliedValue(ResolvedSettingKey);
	if (Value.IsEmpty())
	{
		return DefaultScalar;
	}

	float ParsedScalar = DefaultScalar;
	if (LexTryParseString(ParsedScalar, *Value))
	{
		return ParsedScalar;
	}

	UE_LOG(LogAutoSettingsInput, Warning,
		TEXT("InputModifierSettingScalar '%s' could not parse setting '%s' value '%s'. Falling back to default scalar %f."),
		*GetName(), *ResolvedSettingKey.ToString(), *Value, DefaultScalar);
	return DefaultScalar;
}
