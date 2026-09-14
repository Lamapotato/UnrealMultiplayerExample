// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/SettingBindingTrigger.h"
#include "PawnPossessedSettingBindingTrigger.generated.h"

class APlayerController;
class ULocalPlayer;

/**
 * Invalidates a player-scoped binding whenever its owning player controller possesses a new pawn.
 */
UCLASS(EditInlineNew, Blueprintable)
class AUTOSETTINGS_API UPawnPossessedSettingBindingTrigger : public USettingBindingTrigger
{
	GENERATED_BODY()

public:
	virtual void Activate_Implementation(const FSettingReference& Setting, USettingBinding* Binding) override;
	virtual void Deactivate_Implementation(const FSettingReference& Setting, USettingBinding* Binding) override;

private:
	/** Bind to the current owning player controller so pawn swaps can invalidate the active binding. */
	void BindToPlayerController(const FSettingReference& Setting, USettingBinding* Binding, APlayerController* PlayerController);

	/** Remove the pawn-change watcher from the currently bound player controller, if any. */
	void UnbindFromPlayerController(const FSettingReference& Setting);

	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> OwningLocalPlayer;

	/** The current controller whose pawn changes should invalidate this binding. */
	UPROPERTY(Transient)
	TObjectPtr<APlayerController> BoundPlayerController;

	/** Handle for the owning local player's controller-swap notifications. */
	FDelegateHandle PlayerControllerChangedHandle;

	/** Handle for the current player controller's pawn-swap notifications. */
	FDelegateHandle PawnChangedHandle;
};
