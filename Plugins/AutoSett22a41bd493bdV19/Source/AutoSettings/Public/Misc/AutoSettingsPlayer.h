// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AutoSettingsPlayer.generated.h"

UINTERFACE(MinimalAPI)
class UAutoSettingsPlayer : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for AutoSettings to interact with a PlayerController
 */
class AUTOSETTINGS_API IAutoSettingsPlayer
{
	GENERATED_BODY()

public:
	static FName GetUniquePlayerIdentifier(const APlayerController* PlayerController);
	static FName GetUniquePlayerIdentifier_Default(const APlayerController* PlayerController);

	// This should return a unique identifier for the player that can be used as a key to uniquely store the player's settings when there are settings for multiple local players being stored in the same location
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Auto Settings")
	FName GetUniquePlayerIdentifier() const;
	virtual FName GetUniquePlayerIdentifier_Implementation() const;
};