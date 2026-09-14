// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputModifiers.h"
#include "Utility/InputMappingUtils.h"
#include "InputModifierAxisDirection.generated.h"

/**
 * Input modifier that conditionally negates input values based on stored directional intent.
 * Used to support player remapping of axis directions (e.g., "stick down" mapped to "move forward").
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Axis Direction"))
class AUTOSETTINGSINPUT_API UInputModifierAxisDirection : public UInputModifier
{
	GENERATED_BODY()

	virtual FInputActionValue ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime) override;

private:
	FName FindMappingName(const UEnhancedPlayerInput* PlayerInput) const;
	FKey FindActuatingKey(const UEnhancedPlayerInput* PlayerInput) const;
	float GetStoredDirectionalScale(UAutoSettingsEnhancedInputUserSettings* UserSettings, FName MappingName) const;
	FInputActionValue ApplyDefaultDirectionalLogic(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, FName MappingName) const;

	FKey FindMostActiveKey(const UEnhancedPlayerInput* PlayerInput, const FKeyMappingRow* MappingRow) const;

	/** Cached mapping name to avoid repeated searches. Unset = not searched, Set = searched (value may be NAME_None). */
	mutable TOptional<FName> CachedMappingName;
};
