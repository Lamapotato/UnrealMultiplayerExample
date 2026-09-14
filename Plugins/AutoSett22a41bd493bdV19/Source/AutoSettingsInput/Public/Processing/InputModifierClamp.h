// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputModifiers.h"
#include "InputModifierClamp.generated.h"

/**
 *
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Clamp"))
class AUTOSETTINGSINPUT_API UInputModifierClamp : public UInputModifier
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = Clamp)
	bool bClampMin = true;

	UPROPERTY(EditAnywhere, Category = Clamp, meta = (EditCondition = bClampMin))
	FVector Min;

	UPROPERTY(EditAnywhere, Category = Clamp)
	bool bClampMax = true;

	UPROPERTY(EditAnywhere, Category = Clamp, meta = (EditCondition = bClampMax))
	FVector Max = FVector(1.f);

	virtual FInputActionValue ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime) override;
};
