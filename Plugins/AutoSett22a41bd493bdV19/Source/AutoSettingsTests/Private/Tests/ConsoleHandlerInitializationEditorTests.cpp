// Copyright Sam Bonifacio. All Rights Reserved.

#include "ConsoleHandlerInitializationEditorTests.h"

#include "ConsoleUtilities.h"
#include "HAL/IConsoleManager.h"
#include "Misc/ConsoleVariableSettingBindingStrategy.h"

#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "AutoSettingsConsoleEngineSubsystem.h"
#include "CQTest.h"
#include "Engine/Engine.h"
#include "Framework/SettingRegistry.h"
#include "TestUtilities/ScopedConsoleVariables.h"
#include "UObject/StrongObjectPtr.h"
#include "Utility/SettingRegistryKeyUtils.h"
#endif

static const FName DriverName = TEXT("AutoSettings.Tests.HandlerInitialization.Driver");
static const FName TargetName = TEXT("AutoSettings.Tests.HandlerInitialization.Target");
static constexpr int32 InitialTargetValue = 4;

// Records callback execution so a saved-value assertion cannot pass when handler activation is missing entirely.
static bool bHandlerExecuted = false;

UTestConsoleHandlerInitializationVariable::UTestConsoleHandlerInitializationVariable()
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		Name = DriverName;
	}
}

IConsoleVariable* UTestConsoleHandlerInitializationVariable::RegisterConsoleVariable()
{
	return SB::AutoSettings::Console::RegisterVariable<int32>(Name, 0, TEXT("Drives handler initialization coverage."));
}

void UTestConsoleHandlerInitializationVariable::RegisterChangedCallback()
{
	SB::AutoSettings::Console::RegisterVariableChangedCallback<int32>(Name,
		[](const int32 NewValue)
		{
			bHandlerExecuted = true;
			SB::AutoSettings::Console::SetValue<int32>(TargetName, NewValue);
		},
		bCallOnChangedWhenRegistered);
}

FSettingData UTestConsoleHandlerInitializationPersistenceStrategy::GetSavedValue_Implementation(
	const FSettingReference& /*Setting*/) const
{
	// Return test-owned data directly so registry initialization exercises normal persistence precedence.
	return SavedData;
}

UTestConsoleHandlerInitializationSettingType::UTestConsoleHandlerInitializationSettingType(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bEnabled = false;
	SettingDomain = ESettingDomain::Global;
	KeyFragment = FSettingKeyFragment(TEXT("ConsoleHandlerInitializationTarget"));

	auto* ConsoleVariableBindingStrategy = ObjectInitializer.CreateDefaultSubobject<UConsoleVariableSettingBindingStrategy>(
		ObjectInitializer.GetObj(), TEXT("ConsoleVariableBindingStrategy"));
	ConsoleVariableBindingStrategy->ConsoleVariableName = TargetName;
	BindingStrategy = ConsoleVariableBindingStrategy;
	PersistenceStrategy = ObjectInitializer.CreateDefaultSubobject<UTestConsoleHandlerInitializationPersistenceStrategy>(
		ObjectInitializer.GetObj(), TEXT("TestPersistenceStrategy"));
}

#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS

namespace
{
	using SB::AutoSettings::Tests::FScopedAutoSettingsConsoleVariableClass;
}

TEST_CLASS_WITH_FLAGS(HandlerInitialization, "SB.AutoSettings.Console",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
{
	// Console variables use native lifetime while transient UObjects model the production registry transaction.
	TUniquePtr<TAutoConsoleVariable<int32>> TargetVariable;
	TUniquePtr<FScopedAutoSettingsConsoleVariableClass> DriverVariable;
	TStrongObjectPtr<UTestConsoleHandlerInitializationVariable> HandlerVariable;
	TStrongObjectPtr<USettingRegistry> Registry;
	TStrongObjectPtr<UTestConsoleHandlerInitializationSettingType> SettingDefinition;

	BEFORE_EACH()
	{
		// Each scenario owns its process-wide console names and leaves native discovery inert outside the fixture.
		ASSERT_THAT(IsFalse(SB::AutoSettings::Console::IsVariableRegistered(DriverName)));
		ASSERT_THAT(IsFalse(SB::AutoSettings::Console::IsVariableRegistered(TargetName)));
		UAutoSettingsConsoleEngineSubsystem* ConsoleSubsystem =
			GEngine ? GEngine->GetEngineSubsystem<UAutoSettingsConsoleEngineSubsystem>() : nullptr;
		ASSERT_THAT(IsTrue(ConsoleSubsystem != nullptr));

		TargetVariable = MakeUnique<TAutoConsoleVariable<int32>>(*TargetName.ToString(), InitialTargetValue,
			TEXT("Records handler initialization ordering."));
		TargetVariable->AsVariable()->Set(InitialTargetValue, ECVF_SetByCode);
		DriverVariable = MakeUnique<FScopedAutoSettingsConsoleVariableClass>(*ConsoleSubsystem,
			UTestConsoleHandlerInitializationVariable::StaticClass(), DriverName);
		ASSERT_THAT(IsTrue(DriverVariable->IsRegistered()));

		// Transient production objects exercise capture, callback activation, value application, and restoration directly.
		HandlerVariable = TStrongObjectPtr<UTestConsoleHandlerInitializationVariable>(
			NewObject<UTestConsoleHandlerInitializationVariable>());
		Registry = TStrongObjectPtr<USettingRegistry>(NewObject<USettingRegistry>());
		SettingDefinition = TStrongObjectPtr<UTestConsoleHandlerInitializationSettingType>(
			NewObject<UTestConsoleHandlerInitializationSettingType>(Registry.Get()));
		ASSERT_THAT(IsTrue(HandlerVariable.IsValid()));
		ASSERT_THAT(IsTrue(Registry.IsValid()));
		ASSERT_THAT(IsTrue(SettingDefinition.IsValid()));

		SettingDefinition->bEnabled = true;
		CastChecked<UTestConsoleHandlerInitializationPersistenceStrategy>(
			SettingDefinition->PersistenceStrategy)->SavedData = {};
		Registry->BaseKey = SB::AutoSettings::SettingRegistries::GetBaseKey(ESettingDomain::Global);
		Registry->Register_OnTargetsCaptured(FSimpleDelegate::CreateUObject(
			HandlerVariable.Get(), &UAutoSettingsConsoleVariable::RegisterCallback));
		bHandlerExecuted = false;
	}

	AFTER_EACH()
	{
		// Registry teardown must finish while its binding targets and handler-owned callbacks remain valid.
		if (Registry.IsValid() && Registry->IsInitialized())
		{
			Registry->Deinitialize();
		}

		SettingDefinition.Reset();
		Registry.Reset();
		HandlerVariable.Reset();
		DriverVariable.Reset();
		TargetVariable.Reset();
	}

	TEST_METHOD(RestoresCVarValueCapturedBeforeImmediateHandlerRuns)
	{
		// No saved value lets the immediate handler control the target after capture.
		if (!TestRunner->AddErrorIfFalse(Registry->Initialize({ SettingDefinition.Get() }),
			TEXT("Failed to initialize the test setting registry.")))
		{
			return;
		}

		TestRunner->AddErrorIfFalse(bHandlerExecuted,
			TEXT("Immediate handler did not execute during initialization."));
		const int32 HandlerValue = TargetVariable->GetValueOnGameThread();
		TestRunner->AddErrorIfFalse(HandlerValue == 0,
			*FString::Printf(TEXT("Expected the immediate handler to set the target to 0, but it was %d."),
				HandlerValue));

		// Deinitialization restores the target captured before handler activation.
		Registry->Deinitialize();
		const int32 RestoredValue = TargetVariable->GetValueOnGameThread();
		TestRunner->AddErrorIfFalse(RestoredValue == InitialTargetValue,
			*FString::Printf(TEXT("Expected registry teardown to restore the target to %d, but it was %d."),
				InitialTargetValue, RestoredValue));
	}

	TEST_METHOD(PreservesSavedCVarValueWhenImmediateHandlerRuns)
	{
		// This scenario alone supplies a persisted individual value.
		FSettingData SavedData;
		SavedData.Value = TEXT("1");
		CastChecked<UTestConsoleHandlerInitializationPersistenceStrategy>(
			SettingDefinition->PersistenceStrategy)->SavedData = SavedData;

		if (!TestRunner->AddErrorIfFalse(Registry->Initialize({ SettingDefinition.Get() }),
			TEXT("Failed to initialize the test setting registry.")))
		{
			return;
		}

		// Initial value application must run after the immediate handler so persisted state wins.
		TestRunner->AddErrorIfFalse(bHandlerExecuted,
			TEXT("Immediate handler did not execute during initialization."));
		const int32 ActualValue = TargetVariable->GetValueOnGameThread();
		TestRunner->AddErrorIfFalse(ActualValue == 1,
			*FString::Printf(TEXT("Expected the saved target value 1 after handler initialization, but it was %d."),
				ActualValue));
	}
};

#endif
