// Copyright Sam Bonifacio. All Rights Reserved.

#include "Framework/SettingRegistry.h"
#include "HAL/IConsoleManager.h"
#include "Misc/ConsoleVariableSettingBindingStrategy.h"
#include "TestUtilities/NoOpSettingPersistenceStrategy.h"
#include "TestUtilities/SettingTestType.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "CQTest.h"

// Initial-value restoration is an editor teardown contract and must only execute in editor automation contexts.
TEST_CLASS_WITH_FLAGS(SettingRegistryEditor, "SB.AutoSettings.Framework",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
{
	TEST_METHOD(RestoresControlledSettingAfterControllerSideEffect)
	{
		// Create a transient registry with separate controller and child definitions.
		USettingRegistry* Registry = NewObject<USettingRegistry>(GetTransientPackage());
		UTestSettingType* ChildSettingType = NewObject<UTestSettingType>(GetTransientPackage());
		UTestSettingType* ControllerSettingType = NewObject<UTestSettingType>(GetTransientPackage());
		if (!TestRunner->AddErrorIfFalse(Registry != nullptr, TEXT("Failed to create the setting registry."))
			|| !TestRunner->AddErrorIfFalse(ChildSettingType != nullptr, TEXT("Failed to create the controlled setting type."))
			|| !TestRunner->AddErrorIfFalse(ControllerSettingType != nullptr, TEXT("Failed to create the controller setting type.")))
		{
			return;
		}

		UConsoleVariableSettingBindingStrategy* ChildBindingStrategy =
			NewObject<UConsoleVariableSettingBindingStrategy>(ChildSettingType);
		UTestNoOpSettingPersistenceStrategy* ChildPersistenceStrategy =
			NewObject<UTestNoOpSettingPersistenceStrategy>(ChildSettingType);
		UConsoleVariableSettingBindingStrategy* ControllerBindingStrategy =
			NewObject<UConsoleVariableSettingBindingStrategy>(ControllerSettingType);
		UTestNoOpSettingPersistenceStrategy* ControllerPersistenceStrategy =
			NewObject<UTestNoOpSettingPersistenceStrategy>(ControllerSettingType);
		if (!TestRunner->AddErrorIfFalse(ChildBindingStrategy != nullptr, TEXT("Failed to create the controlled binding strategy."))
			|| !TestRunner->AddErrorIfFalse(ChildPersistenceStrategy != nullptr, TEXT("Failed to create the controlled persistence strategy."))
			|| !TestRunner->AddErrorIfFalse(ControllerBindingStrategy != nullptr, TEXT("Failed to create the controller binding strategy."))
			|| !TestRunner->AddErrorIfFalse(ControllerPersistenceStrategy != nullptr, TEXT("Failed to create the controller persistence strategy.")))
		{
			return;
		}

		ChildSettingType->bEnabled = true;
		ChildSettingType->SettingDomain = ESettingDomain::Global;
		ChildSettingType->KeyFragment = FSettingKeyFragment(TEXT("RegistryRestoreChild"));
		ChildSettingType->BindingStrategy = ChildBindingStrategy;
		ChildSettingType->PersistenceStrategy = ChildPersistenceStrategy;

		ControllerSettingType->bEnabled = true;
		ControllerSettingType->SettingDomain = ESettingDomain::Global;
		ControllerSettingType->KeyFragment = FSettingKeyFragment(TEXT("RegistryRestoreController"));
		FPartialSettingKey ControlledChildKey;
		ControlledChildKey += FSettingKeyFragment(TEXT("RegistryRestoreChild"));
		ControllerSettingType->ControlledSettingKeys.Add(ControlledChildKey);
		ControllerSettingType->BindingStrategy = ControllerBindingStrategy;
		ControllerSettingType->PersistenceStrategy = ControllerPersistenceStrategy;

		// Register independent CVar targets before binding captures their initial values.
		const FFullSettingKey ChildKey = Registry->GetSettingKey(ChildSettingType);
		const FFullSettingKey ControllerKey = Registry->GetSettingKey(ControllerSettingType);
		const FName ChildVariableName = ChildKey.GetLastFragment().Name;
		const FName ControllerVariableName = ControllerKey.GetLastFragment().Name;
		if (!TestRunner->AddErrorIfFalse(IConsoleManager::Get().FindConsoleVariable(*ChildVariableName.ToString()) == nullptr,
			TEXT("The controlled test CVar was already registered."))
			|| !TestRunner->AddErrorIfFalse(IConsoleManager::Get().FindConsoleVariable(*ControllerVariableName.ToString()) == nullptr,
				TEXT("The controller test CVar was already registered.")))
		{
			return;
		}

		const TSharedRef<TAutoConsoleVariable<int32>> ChildVariable = MakeShared<TAutoConsoleVariable<int32>>(
			*ChildVariableName.ToString(), 1, TEXT("Controlled target for registry restoration coverage."));
		const TSharedRef<TAutoConsoleVariable<int32>> ControllerVariable = MakeShared<TAutoConsoleVariable<int32>>(
			*ControllerVariableName.ToString(), 2, TEXT("Controller target for registry restoration coverage."));
		ControllerVariable->AsVariable()->OnChangedDelegate().AddLambda(
			[ChildVariable](IConsoleVariable* ChangedVariable)
			{
				// The controller target synchronously drives the child target through the real CVar notification path.
				ChildVariable->AsVariable()->Set(ChangedVariable->GetInt(), ECVF_SetByCode);
			});

		// Keep CVar targets alive until active bindings are released, including assertion exits.
		TestCommandBuilder.OnTearDown([Registry, ChildVariable, ControllerVariable]()
			{
				if (Registry && Registry->IsInitialized())
				{
					Registry->Deinitialize();
				}
			});

		// Separate late-registration batches put the child before its controller in the restoration queue.
		TMap<FFullSettingKey, USettingType*> ChildSettings;
		ChildSettings.Add(ChildKey, ChildSettingType);
		TMap<FFullSettingKey, USettingType*> ControllerSettings;
		ControllerSettings.Add(ControllerKey, ControllerSettingType);
		if (!TestRunner->AddErrorIfFalse(Registry->Initialize({}), TEXT("Failed to initialize the empty setting registry."))
			|| !TestRunner->AddErrorIfFalse(Registry->RegisterSettings(ChildSettings), TEXT("Failed to register the controlled setting."))
			|| !TestRunner->AddErrorIfFalse(Registry->RegisterSettings(ControllerSettings), TEXT("Failed to register the controller setting.")))
		{
			return;
		}

		// Prove the controller side effect establishes the bound child's applied value through its CVar notification.
		Registry->SetSetting(ControllerKey, TEXT("3"), ESettingAction::ApplyOnly);
		if (!TestRunner->AddErrorIfFalse(ChildVariable->GetValueOnGameThread() == 3,
			TEXT("Expected the controller application to update the controlled CVar.")))
		{
			return;
		}

		// Shutdown must leave every target at its independently captured value, regardless of queue order.
		Registry->Deinitialize();
		const int32 ActualChildValue = ChildVariable->GetValueOnGameThread();
		TestRunner->AddErrorIfFalse(ActualChildValue == 1,
			*FString::Printf(TEXT("Expected the controlled CVar to restore 1, but it was %d."), ActualChildValue));
	}
};

#endif
