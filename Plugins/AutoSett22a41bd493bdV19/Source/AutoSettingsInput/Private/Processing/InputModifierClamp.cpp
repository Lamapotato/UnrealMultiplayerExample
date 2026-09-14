// Copyright Sam Bonifacio. All Rights Reserved.

#include "Processing/InputModifierClamp.h"

FInputActionValue UInputModifierClamp::ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput,
	FInputActionValue CurrentValue, float DeltaTime)
{
	const FVector Val = CurrentValue.Get<FVector>();
	const FVector EffectiveMin = bClampMin ? Min : Val;
	const FVector EffectiveMax = bClampMax ? Max : Val;
	const FVector Clamped = ClampVector(Val, EffectiveMin, EffectiveMax);
	return Clamped;
}
