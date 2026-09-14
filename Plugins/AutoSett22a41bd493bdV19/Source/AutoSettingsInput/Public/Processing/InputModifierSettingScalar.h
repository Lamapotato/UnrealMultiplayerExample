// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/SettingKey.h"
#include "InputModifiers.h"

#include "InputModifierSettingScalar.generated.h"

class ULocalPlayer;
class USettingRegistry;

/**
 * Scales player input by the value of a player-scoped setting.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Scale By Setting"))
class AUTOSETTINGSINPUT_API UInputModifierSettingScalar : public UInputModifier
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FSettingSpecifier Setting;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (ClampMin = "0.0"))
	float DefaultScalar = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bAffectX = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bAffectY = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bAffectZ = true;

	virtual FInputActionValue ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime) override;

private:
	FSettingKey ResolveScalarSettingKey() const;
	const USettingRegistry* ResolveRegistry(const UEnhancedPlayerInput* PlayerInput, const FSettingKey& ResolvedSettingKey) const;
	float ResolveScalar(const UEnhancedPlayerInput* PlayerInput) const;

	mutable TWeakObjectPtr<const ULocalPlayer> CachedLocalPlayer;
	mutable TWeakObjectPtr<const USettingRegistry> CachedRegistry;
	mutable FSettingKey CachedResolvedSettingKey;
};
