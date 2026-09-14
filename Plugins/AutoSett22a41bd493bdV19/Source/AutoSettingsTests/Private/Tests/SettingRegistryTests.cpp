// Copyright Sam Bonifacio. All Rights Reserved.

#include "SettingRegistryTests.h"

#include "Framework/SettingRegistry.h"
#include "TestUtilities/NoOpSettingPersistenceStrategy.h"
#include "TestUtilities/SettingTestType.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "CQTest.h"

FString UTestRegistryLifecycleBindingStrategy::GetTargetValue_Implementation(const FSettingReference& Setting)
{
	// A non-empty target value lets registry initialization establish an applied value for the setting.
	return TEXT("RegistryLifecycleTargetValue");
}

void UTestRegistryLifecycleBindingStrategy::BeginBinding_Implementation(const FSettingReference& Setting, USettingBinding* Binding)
{
	// Count every binding the registry creates so a post-deinitialize bind is directly observable.
	++BeginBindingCount;

	Super::BeginBinding_Implementation(Setting, Binding);
}

TEST_CLASS(SettingRegistry, "SB.AutoSettings.Framework")
{
	TEST_METHOD(DoesNotBindWhenTickedAfterDeinitialize)
	{
		// Build the smallest registry graph that can create an active runtime binding.
		USettingRegistry* Registry = NewObject<USettingRegistry>(GetTransientPackage());
		UTestSettingType* SettingType = NewObject<UTestSettingType>(GetTransientPackage());
		if (!TestRunner->AddErrorIfFalse(Registry != nullptr, TEXT("Failed to create the setting registry."))
			|| !TestRunner->AddErrorIfFalse(SettingType != nullptr, TEXT("Failed to create the lifecycle setting type.")))
		{
			return;
		}

		UTestRegistryLifecycleBindingStrategy* Strategy = NewObject<UTestRegistryLifecycleBindingStrategy>(SettingType);
		UTestNoOpSettingPersistenceStrategy* PersistenceStrategy =
			NewObject<UTestNoOpSettingPersistenceStrategy>(SettingType);
		if (!TestRunner->AddErrorIfFalse(Strategy != nullptr, TEXT("Failed to create the lifecycle binding strategy."))
			|| !TestRunner->AddErrorIfFalse(PersistenceStrategy != nullptr, TEXT("Failed to create the no-op persistence strategy.")))
		{
			return;
		}

		SettingType->bEnabled = true;
		SettingType->SettingDomain = ESettingDomain::Global;
		SettingType->KeyFragment = FSettingKeyFragment(TEXT("RegistryLifecycleTest"));
		SettingType->BindingStrategy = Strategy;
		SettingType->PersistenceStrategy = PersistenceStrategy;

		// Release the registry if an assertion stops the test before the explicit deinitialize step below.
		TestCommandBuilder.OnTearDown([Registry]()
			{
				if (Registry && Registry->IsInitialized())
				{
					Registry->Deinitialize();
				}
			});

		// Initialization should create exactly one active binding for the one registered setting.
		const bool bInitialized = Registry->Initialize({ SettingType });
		if (!TestRunner->AddErrorIfFalse(bInitialized, TEXT("Expected the registry to initialize with the lifecycle setting type."))
			|| !TestRunner->AddErrorIfFalse(Strategy->BeginBindingCount == 1,
				*FString::Printf(TEXT("Expected one binding during initialization, but got %d."), Strategy->BeginBindingCount)))
		{
			return;
		}

		// Deinitialize before ticking so the assertion targets the registry's shutdown tick behavior.
		Registry->Deinitialize();
		if (!TestRunner->AddErrorIfFalse(!Registry->IsInitialized(), TEXT("Expected the registry to be deinitialized after shutdown.")))
		{
			return;
		}

		// Tickable registries can remain alive after shutdown; ticking one must not recreate bindings.
		static_cast<FTickableGameObject*>(Registry)->Tick(0.0f);
		TestRunner->AddErrorIfFalse(Strategy->BeginBindingCount == 1,
			*FString::Printf(TEXT("Expected a deinitialized registry tick not to create bindings, but got %d starts."), Strategy->BeginBindingCount));
	}

	TEST_METHOD(RegistersControlledSettingReferencedByController)
	{
		// Model one generic setting type and one controller without defining scenario-specific reflected classes.
		USettingRegistry* Registry = NewObject<USettingRegistry>(GetTransientPackage());
		UTestSettingType* GenericSettingType = NewObject<UTestSettingType>(GetTransientPackage());
		UTestSettingType* ControllerSettingType = NewObject<UTestSettingType>(GetTransientPackage());

		// The empty generic key represents the registry root that owns arbitrary global child keys.
		Registry->BaseKey += FSettingKeyFragment(TEXT("Global"));
		GenericSettingType->bEnabled = true;
		GenericSettingType->PersistenceStrategy =
			NewObject<UTestNoOpSettingPersistenceStrategy>(GenericSettingType);

		ControllerSettingType->bEnabled = true;
		ControllerSettingType->KeyFragment = FSettingKeyFragment(TEXT("RegistryReferencedController"));
		ControllerSettingType->PersistenceStrategy =
			NewObject<UTestNoOpSettingPersistenceStrategy>(ControllerSettingType);
		const FPartialSettingKey ControlledChildKey(TEXT("RegistryReferencedChild"));
		ControllerSettingType->ControlledSettingKeys.Add(ControlledChildKey);

		// Keep the initialized tickable registry from surviving an assertion exit.
		TestCommandBuilder.OnTearDown([Registry]()
			{
				if (Registry && Registry->IsInitialized())
				{
					Registry->Deinitialize();
				}
			});

		// Make both definitions available for ownership resolution, then register the controller through the public API.
		if (!TestRunner->AddErrorIfFalse(Registry->Initialize({ GenericSettingType, ControllerSettingType }),
			TEXT("Failed to initialize the setting registry.")))
		{
			return;
		}

		const FFullSettingKey ControllerKey = Registry->GetSettingKey(ControllerSettingType);
		TMap<FFullSettingKey, USettingType*> ControllerSettings;
		ControllerSettings.Add(ControllerKey, ControllerSettingType);
		if (!TestRunner->AddErrorIfFalse(Registry->RegisterSettings(ControllerSettings), TEXT("Failed to register the controller setting.")))
		{
			return;
		}

		// A controlled-key declaration is sufficient to register that key through its compatible generic owner.
		FFullSettingKey FullControlledChildKey;
		FullControlledChildKey += Registry->BaseKey;
		FullControlledChildKey += ControlledChildKey;
		const FRegisteredSetting* RegisteredChild = Registry->GetRegisteredSettings().Find(FullControlledChildKey);
		TestRunner->AddErrorIfFalse(RegisteredChild && RegisteredChild->SettingType == GenericSettingType,
			*FString::Printf(TEXT("Expected controller-referenced setting '%s' to register through the generic setting type."),
				*FullControlledChildKey.ToString()));
	}

};

#endif
