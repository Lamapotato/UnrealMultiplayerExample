// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "Framework/SettingBindingStrategy.h"

#include "PawnPossessedSettingBindingTriggerTests.generated.h"

class APlayerController;

/**
 * Minimal LocalPlayer test double that can swap its active controller and emit the same delegate
 * that runtime code listens to.
 */
UCLASS(HideDropdown, NotBlueprintable)
class AUTOSETTINGSTESTS_API UTestPawnPossessedTriggerLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()

public:
	/** Update the active controller pointer, then broadcast the public controller-change delegate. */
	void SetTestPlayerController(APlayerController* NewPlayerController);
};

/**
 * Test binding strategy that records each bind and unbind while still delegating trigger activation
 * and teardown to the production base implementation.
 */
UCLASS(HideDropdown, NotBlueprintable)
class AUTOSETTINGSTESTS_API UTestPawnPossessedTriggerBindingStrategy : public USettingBindingStrategy
{
	GENERATED_BODY()

public:
	/** Count each active binding start so the test can observe invalidation-driven rebinds. */
	UPROPERTY(Transient)
	int32 BeginBindingCount = 0;

	/** Count each binding teardown so the test can prove the old controller subscription was removed. */
	UPROPERTY(Transient)
	int32 EndBindingCount = 0;

	/** Cache the controller resolved during the latest bind so assertions can verify which controller owns the binding. */
	UPROPERTY(Transient)
	TObjectPtr<APlayerController> LastControllerSeen;

	virtual FString GetTargetValue_Implementation(const FSettingReference& Setting) override;
	virtual void BeginBinding_Implementation(const FSettingReference& Setting, USettingBinding* Binding) override;
	virtual void EndBinding_Implementation(const FSettingReference& Setting, USettingBinding* Binding) override;
};
