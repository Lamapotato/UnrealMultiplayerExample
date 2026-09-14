// Copyright Sam Bonifacio. All Rights Reserved.

#include "Misc/PawnPossessedSettingBindingTrigger.h"

#include "AutoSettingsLogs.h"
#include "Framework/AutoSettingsPlayerSubsystem.h"
#include "Framework/SettingBinding.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Logging/StructuredLog.h"

void UPawnPossessedSettingBindingTrigger::Activate_Implementation(const FSettingReference& Setting, USettingBinding* Binding)
{
	Super::Activate_Implementation(Setting, Binding);

	// Reset any stale delegate state before wiring this runtime trigger into the current local-player
	// context.
	UnbindFromPlayerController(Setting);
	if (OwningLocalPlayer && PlayerControllerChangedHandle.IsValid())
	{
		OwningLocalPlayer->OnPlayerControllerChanged().Remove(PlayerControllerChangedHandle);
		PlayerControllerChangedHandle.Reset();
	}
	OwningLocalPlayer = nullptr;

	if (!Binding)
	{
		// Keep this visible in logs because a null binding means the trigger could never register its
		// pawn watcher, which would make possession-based rebinding impossible.
		UE_LOGFMT(LogAutoSettings, Verbose,
			"Skipping pawn possession trigger activation for setting '{0}' because the binding is null.",
			Setting.Key.ToString());
		return;
	}

	OwningLocalPlayer = UAutoSettingsPlayerSubsystem::GetOwningLocalPlayerFromRegistry(Setting.Context.Registry);
	if (!OwningLocalPlayer)
	{
		// Log missing local-player resolution so player-scope context problems are distinguishable from
		// trigger activation problems.
		UE_LOGFMT(LogAutoSettings, Verbose,
			"Skipping pawn possession trigger activation for setting '{0}' because no owning local player was resolved.",
			Setting.Key.ToString());
		return;
	}

	// Listen for controller swaps so the trigger can invalidate and rebind when the local player's
	// active controller changes after initialization.
	PlayerControllerChangedHandle = OwningLocalPlayer->OnPlayerControllerChanged().AddWeakLambda(this,
		[this, Setting, Binding](APlayerController* NewPlayerController)
		{
			UE_LOGFMT(LogAutoSettings, Verbose,
				"Pawn possession trigger '{0}' observed controller change for setting '{1}': '{2}' -> '{3}'.",
				GetPathNameSafe(this),
				Setting.Key.ToString(),
				GetPathNameSafe(BoundPlayerController),
				GetPathNameSafe(NewPlayerController));

			if (Binding)
			{
				Binding->NotifyBindingInvalidated();
			}
		});

	// Subscribe to the current controller immediately so pawn swaps on the active controller still
	// invalidate the binding without waiting for a later controller change.
	BindToPlayerController(Setting, Binding, OwningLocalPlayer->GetPlayerController(nullptr));
}

void UPawnPossessedSettingBindingTrigger::Deactivate_Implementation(const FSettingReference& Setting, USettingBinding* Binding)
{
	// Remove the controller swap watcher first so teardown cannot schedule a new invalidation while
	// the binding is ending.
	if (OwningLocalPlayer && PlayerControllerChangedHandle.IsValid())
	{
		UE_LOGFMT(LogAutoSettings, Verbose,
			"Deactivating pawn possession trigger '{0}' controller-change watcher for setting '{1}' on local player '{2}'.",
			GetPathNameSafe(this),
			Setting.Key.ToString(),
			GetPathNameSafe(OwningLocalPlayer));
		OwningLocalPlayer->OnPlayerControllerChanged().Remove(PlayerControllerChangedHandle);
	}

	PlayerControllerChangedHandle.Reset();
	OwningLocalPlayer = nullptr;

	UnbindFromPlayerController(Setting);

	Super::Deactivate_Implementation(Setting, Binding);
}

void UPawnPossessedSettingBindingTrigger::BindToPlayerController(const FSettingReference& Setting, USettingBinding* Binding,
	APlayerController* PlayerController)
{
	// Swap to the latest controller so the pawn watcher always follows the local player's current
	// active controller.
	UnbindFromPlayerController(Setting);
	BoundPlayerController = PlayerController;
	if (!BoundPlayerController)
	{
		UE_LOGFMT(LogAutoSettings, Verbose,
			"Skipping pawn possession watcher bind for setting '{0}' because the owning local player has no active controller.",
			Setting.Key.ToString());
		return;
	}

	// Record the exact trigger and controller involved so runtime logs can be matched back to the
	// live duplicated trigger instance.
	UE_LOGFMT(LogAutoSettings, Verbose,
		"Activating pawn possession trigger '{0}' for setting '{1}' on controller '{2}'.",
		GetPathNameSafe(this),
		Setting.Key.ToString(),
		GetPathNameSafe(BoundPlayerController));

	PawnChangedHandle = BoundPlayerController->GetOnNewPawnNotifier().AddWeakLambda(this,
		[this, Binding](APawn* NewPawn)
		{
			if (Binding)
			{
				// Log the possession event that drives binding invalidation so the callback can be traced
				// without relying on a Blueprint breakpoint in the duplicated trigger object.
				UE_LOGFMT(LogAutoSettings, Verbose,
					"Pawn possession trigger '{0}' invalidating binding '{1}' because controller received pawn '{2}'.",
					GetPathNameSafe(this),
					GetPathNameSafe(Binding),
					GetPathNameSafe(NewPawn));
				Binding->NotifyBindingInvalidated();
			}
		});
}

void UPawnPossessedSettingBindingTrigger::UnbindFromPlayerController(const FSettingReference& Setting)
{
	if (BoundPlayerController && PawnChangedHandle.IsValid())
	{
		// Log teardown of the possession watcher so rebind cycles clearly show which trigger instance
		// unsubscribed from the controller.
		UE_LOGFMT(LogAutoSettings, Verbose,
			"Deactivating pawn possession trigger '{0}' for setting '{1}' on controller '{2}'.",
			GetPathNameSafe(this),
			Setting.Key.ToString(),
			GetPathNameSafe(BoundPlayerController));
		BoundPlayerController->GetOnNewPawnNotifier().Remove(PawnChangedHandle);
	}

	PawnChangedHandle.Reset();
	BoundPlayerController = nullptr;
}
