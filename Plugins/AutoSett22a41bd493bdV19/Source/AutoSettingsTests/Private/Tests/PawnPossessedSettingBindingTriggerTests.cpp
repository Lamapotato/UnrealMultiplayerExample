// Copyright Sam Bonifacio. All Rights Reserved.

#include "PawnPossessedSettingBindingTriggerTests.h"

#include "Engine/Engine.h"
#include "Framework/AutoSettingsPlayerSubsystem.h"
#include "Framework/SettingBinding.h"
#include "Framework/SettingRegistry.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Misc/PawnPossessedSettingBindingTrigger.h"
#include "TestUtilities/SettingTestType.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "CQTest.h"

void UTestPawnPossessedTriggerLocalPlayer::SetTestPlayerController(APlayerController* NewPlayerController)
{
	// Route controller swaps through the engine's normal player path so both the cached controller
	// pointer and the public controller-change delegate stay in sync.
	SwitchController(NewPlayerController);
}

FString UTestPawnPossessedTriggerBindingStrategy::GetTargetValue_Implementation(const FSettingReference& Setting)
{
	// Encode the currently resolved controller into the target value so the binding can still execute
	// a realistic sync path during the test.
	const APlayerController* PlayerController = UAutoSettingsPlayerSubsystem::GetOwningPlayerControllerFromSetting(Setting);
	return GetNameSafe(PlayerController);
}

void UTestPawnPossessedTriggerBindingStrategy::BeginBinding_Implementation(const FSettingReference& Setting, USettingBinding* Binding)
{
	// Record which controller the registry bound against before activating production triggers.
	++BeginBindingCount;
	LastControllerSeen = UAutoSettingsPlayerSubsystem::GetOwningPlayerControllerFromSetting(Setting);

	Super::BeginBinding_Implementation(Setting, Binding);
}

void UTestPawnPossessedTriggerBindingStrategy::EndBinding_Implementation(const FSettingReference& Setting, USettingBinding* Binding)
{
	// Count teardown calls so the test can prove invalidation actually unbound the previous watcher.
	++EndBindingCount;

	Super::EndBinding_Implementation(Setting, Binding);
}

namespace SB::AutoSettings::Tests
{
	// Build a transient player-scoped registry whose outer chain matches the production subsystem
	// ownership lookup used by the pawn-possessed trigger.
	static USettingRegistry* CreatePlayerRegistry(UTestPawnPossessedTriggerLocalPlayer& LocalPlayer)
	{
		UAutoSettingsPlayerSubsystem* PlayerSubsystem = NewObject<UAutoSettingsPlayerSubsystem>(&LocalPlayer);
		USettingRegistry* Registry = NewObject<USettingRegistry>(PlayerSubsystem);
		Registry->BaseKey += FSettingKeyFragment(TEXT("Player"));
		Registry->BaseKey += FSettingKeyFragment(TEXT("Automation"));
		return Registry;
	}
} // namespace SB::AutoSettings::Tests

using namespace SB::AutoSettings::Tests;

TEST_CLASS(PawnPossessed, "SB.AutoSettings.BindingTriggers")
{
	TEST_METHOD(RebindsWhenLocalPlayerControllerChanges)
	{
		// Assemble a fully transient runtime object graph so the test exercises the shipping binding and
		// trigger code without requiring persistent assets.
		UTestPawnPossessedTriggerLocalPlayer* LocalPlayer = NewObject<UTestPawnPossessedTriggerLocalPlayer>(GEngine);
		APlayerController* OldController = NewObject<APlayerController>(GetTransientPackage(), TEXT("OldController"));
		APlayerController* NewController = NewObject<APlayerController>(GetTransientPackage(), TEXT("NewController"));
		APawn* OldPawn = NewObject<APawn>(GetTransientPackage(), TEXT("OldPawn"));
		APawn* IgnoredPawn = NewObject<APawn>(GetTransientPackage(), TEXT("IgnoredPawn"));
		APawn* NewPawn = NewObject<APawn>(GetTransientPackage(), TEXT("NewPawn"));
		UTestSettingType* SettingType = NewObject<UTestSettingType>(GetTransientPackage());
		USettingRegistry* Registry = LocalPlayer ? CreatePlayerRegistry(*LocalPlayer) : nullptr;
		UTestPawnPossessedTriggerBindingStrategy* Strategy = SettingType
			? NewObject<UTestPawnPossessedTriggerBindingStrategy>(SettingType)
			: nullptr;
		UPawnPossessedSettingBindingTrigger* Trigger = Strategy
			? NewObject<UPawnPossessedSettingBindingTrigger>(Strategy)
			: nullptr;

		if (!TestRunner->AddErrorIfFalse(LocalPlayer != nullptr, TEXT("Failed to create the LocalPlayer test double."))
			|| !TestRunner->AddErrorIfFalse(OldController != nullptr, TEXT("Failed to create the original player controller."))
			|| !TestRunner->AddErrorIfFalse(NewController != nullptr, TEXT("Failed to create the replacement player controller."))
			|| !TestRunner->AddErrorIfFalse(OldPawn != nullptr, TEXT("Failed to create the pawn used for the original controller."))
			|| !TestRunner->AddErrorIfFalse(IgnoredPawn != nullptr, TEXT("Failed to create the pawn used to verify the old controller is ignored after the swap."))
			|| !TestRunner->AddErrorIfFalse(NewPawn != nullptr, TEXT("Failed to create the pawn used for the replacement controller."))
			|| !TestRunner->AddErrorIfFalse(SettingType != nullptr, TEXT("Failed to create the transient test setting type."))
			|| !TestRunner->AddErrorIfFalse(Registry != nullptr, TEXT("Failed to create the player-scoped setting registry."))
			|| !TestRunner->AddErrorIfFalse(Strategy != nullptr, TEXT("Failed to create the recording binding strategy."))
			|| !TestRunner->AddErrorIfFalse(Trigger != nullptr, TEXT("Failed to create the pawn-possessed binding trigger.")))
		{
			return;
		}

		SettingType->bEnabled = true;
		SettingType->SettingDomain = ESettingDomain::Player;
		SettingType->KeyFragment = FSettingKeyFragment(TEXT("PawnPossessedTriggerTest"));
		Strategy->Triggers.Add(Trigger);
		SettingType->BindingStrategy = Strategy;

		// Always deinitialize the transient registry so live delegates are released before GC cleans up
		// the helper objects created by the test body.
		TestCommandBuilder.OnTearDown([Registry]()
			{
				if (Registry && Registry->IsInitialized())
				{
					Registry->Deinitialize();
				}
			});

		// Bind against the first controller before the registry initializes so the runtime trigger starts
		// from the same state as a player-scoped setting during normal play.
		LocalPlayer->SetTestPlayerController(OldController);
		Registry->Initialize({ SettingType });

		// Format assertion messages with both expected and observed state so a failing automation run
		// immediately shows which part of the rebind lifecycle diverged.
		auto ExpectIntEqual = [this](const int32 Actual, const int32 Expected, const TCHAR* Label, const TCHAR* ValueName)
		{
			return TestRunner->AddErrorIfFalse(
				Actual == Expected,
				*FString::Printf(TEXT("%s Expected %s to be %d, but it was %d."), Label, ValueName, Expected, Actual));
		};

		// Include both controller names so controller ownership failures show the stale and expected
		// binding targets directly in the assertion output.
		auto ExpectControllerEqual = [this](const APlayerController* Actual, const APlayerController* Expected, const TCHAR* Label)
		{
			return TestRunner->AddErrorIfFalse(
				Actual == Expected,
				*FString::Printf(TEXT("%s Expected controller '%s', but got '%s'."), Label, *GetNameSafe(Expected), *GetNameSafe(Actual)));
		};

		if (!ExpectIntEqual(Strategy->BeginBindingCount, 1, TEXT("The initial bind should run exactly once."), TEXT("begin-bind count"))
			|| !ExpectIntEqual(Strategy->EndBindingCount, 0, TEXT("The initial bind should not immediately tear itself down."), TEXT("end-bind count"))
			|| !ExpectControllerEqual(Strategy->LastControllerSeen, OldController, TEXT("The initial bind should target the original controller.")))
		{
			return;
		}

		// A pawn change on the currently bound controller should still invalidate and rebind.
		OldController->GetOnNewPawnNotifier().Broadcast(OldPawn);
		if (!ExpectIntEqual(Strategy->BeginBindingCount, 2, TEXT("When the original controller gets a new pawn, the binding should rebind exactly once."), TEXT("begin-bind count"))
			|| !ExpectIntEqual(Strategy->EndBindingCount, 1, TEXT("When the original controller gets a new pawn, the previous binding should be torn down exactly once."), TEXT("end-bind count"))
			|| !ExpectControllerEqual(Strategy->LastControllerSeen, OldController, TEXT("After the original controller gets a new pawn, the binding should still belong to the original controller.")))
		{
			return;
		}

		// Swapping the LocalPlayer to a different controller is the regression path under test.
		LocalPlayer->SetTestPlayerController(NewController);
		if (!ExpectIntEqual(Strategy->BeginBindingCount, 3, TEXT("After the LocalPlayer switches controllers, the binding should invalidate and create one new bind."), TEXT("begin-bind count"))
			|| !ExpectIntEqual(Strategy->EndBindingCount, 2, TEXT("After the LocalPlayer switches controllers, the old binding should be torn down exactly once."), TEXT("end-bind count"))
			|| !ExpectControllerEqual(Strategy->LastControllerSeen, NewController, TEXT("After the LocalPlayer switches controllers, the binding should move to the new controller instead of staying on the old one.")))
		{
			return;
		}

		// Once rebound, the old controller must no longer be able to invalidate the binding.
		OldController->GetOnNewPawnNotifier().Broadcast(IgnoredPawn);
		if (!ExpectIntEqual(Strategy->BeginBindingCount, 3, TEXT("Once the binding has moved to the new controller, the old controller should no longer trigger another bind."), TEXT("begin-bind count"))
			|| !ExpectIntEqual(Strategy->EndBindingCount, 2, TEXT("Once the binding has moved to the new controller, the old controller should no longer tear the binding down."), TEXT("end-bind count")))
		{
			return;
		}

		// The new controller should now own the pawn-change invalidation path.
		NewController->GetOnNewPawnNotifier().Broadcast(NewPawn);
		if (!ExpectIntEqual(Strategy->BeginBindingCount, 4, TEXT("When the new controller gets a new pawn, it should trigger one more bind."), TEXT("begin-bind count"))
			|| !ExpectIntEqual(Strategy->EndBindingCount, 3, TEXT("When the new controller gets a new pawn, it should tear down the previous binding exactly once."), TEXT("end-bind count"))
			|| !ExpectControllerEqual(Strategy->LastControllerSeen, NewController, TEXT("After the new controller gets a new pawn, the binding should still belong to the new controller.")))
		{
			return;
		}
	}
};

#endif
