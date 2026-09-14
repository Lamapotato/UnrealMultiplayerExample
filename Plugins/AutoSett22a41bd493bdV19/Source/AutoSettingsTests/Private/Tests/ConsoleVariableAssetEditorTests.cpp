// Copyright Sam Bonifacio. All Rights Reserved.

#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS

#include "CQTest.h"

#include "EditorTestAssetContext.h"
#include "AutoSettingsConsoleEngineSubsystem.h"
#include "AutoSettingsConsoleVariable.h"
#include "ConsoleUtilities.h"
#include "Engine/AssetManager.h"
#include "Engine/Blueprint.h"
#include "HAL/IConsoleManager.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/Guid.h"
#include "Misc/PackageName.h"
#include "Processing/InputModifierSettingScalar.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/SavePackage.h"

using SB::Editor::Tests::FEditorTestAssetContext;
using SB::Editor::Tests::GetAssetDataByObjectPath;
using SB::Editor::Tests::IsAssetLoaded;
using SB::Editor::Tests::UnloadAsset;

namespace SB::AutoSettings::Editor::Tests
{
	// Generate a stable unique token per test instance so assets and console variables never collide with prior runs.
	static FString MakeUniqueToken()
	{
		return FGuid::NewGuid().ToString(EGuidFormats::Digits);
	}

	// Build a unique console variable name so the asset manager never sees stale duplicate primary asset IDs.
	static FName MakeUniqueCVarName(const FString& Prefix)
	{
		return FName(*FString::Printf(TEXT("Automation.AutoSettings.Console.%s.%s"), *Prefix, *MakeUniqueToken()));
	}

	// Poll expected registration transitions without counting test waits as production console lookup pressure.
	static bool IsVariableRegisteredUntracked(const FName Name)
	{
		IConsoleObject* ExistingConsoleObject = IConsoleManager::Get().FindConsoleObject(
			*Name.ToString(), /* bTrackFrequentCalls */ false);
		return ExistingConsoleObject && !ExistingConsoleObject->TestFlags(ECVF_Unregistered);
	}

	// Save the Blueprint package so the asset registry sees the new or updated asset through the normal editor code path.
	static bool CompileAndSaveBlueprint(UBlueprint& Blueprint)
	{
		FBlueprintEditorUtils::MarkBlueprintAsModified(&Blueprint);
		FKismetEditorUtilities::CompileBlueprint(&Blueprint, EBlueprintCompileOptions::SkipGarbageCollection);

		UPackage* Package = Blueprint.GetOutermost();
		if (!Package)
		{
			return false;
		}

		Package->MarkPackageDirty();

		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.Error = GLog;

		const FString PackageFileName = FPackageName::LongPackageNameToFilename(
			Package->GetName(),
			FPackageName::GetAssetPackageExtension());

		const bool bSaved = UPackage::SavePackage(Package, nullptr, *PackageFileName, SaveArgs);
		FlushAsyncLoading();
		return bSaved;
	}

	// Create a Blueprint package inside the supplied directory so teardown can delete the whole directory instead of individual assets.
	static UBlueprint* CreateBlueprintAsset(const FString& DirectoryPath, const FString& AssetName, UClass* ParentClass)
	{
		const FString PackagePath = DirectoryPath / AssetName;
		UPackage* Package = CreatePackage(*PackagePath);
		if (!Package)
		{
			return nullptr;
		}

		Package->FullyLoad();

		return FKismetEditorUtilities::CreateBlueprint(
			ParentClass,
			Package,
			FName(*AssetName),
			BPTYPE_Normal,
			UBlueprint::StaticClass(),
			UBlueprintGeneratedClass::StaticClass(),
			TEXT("SB_AutoSettings_ConsoleVariableAssetTests"));
	}

	// Update the bool console variable defaults so the real asset-registry callback registers the authored value.
	static bool ConfigureBooleanConsoleVariable(UBlueprint& Blueprint, const FName Name, bool DefaultValue)
	{
		UConsoleVariable_Boolean* Defaults = Blueprint.GeneratedClass
			? Blueprint.GeneratedClass->GetDefaultObject<UConsoleVariable_Boolean>()
			: nullptr;

		if (!Defaults)
		{
			return false;
		}

		Defaults->Name = Name;
		Defaults->Help = FString::Printf(TEXT("Automation test console variable %s"), *Name.ToString());
		Defaults->DefaultValue = DefaultValue;
		Defaults->bCallOnChangedWhenRegistered = false;
		return true;
	}

	// Create and save a bool console variable Blueprint inside the tracked test directory.
	static UBlueprint* CreateBooleanBlueprint(FEditorTestAssetContext& Context, const FString& AssetName, const FName CVarName, bool DefaultValue)
	{
		UBlueprint* Blueprint = CreateBlueprintAsset(Context.GetDirectoryPath(), AssetName, UConsoleVariable_Boolean::StaticClass());
		if (!Blueprint)
		{
			return nullptr;
		}

		if (!ConfigureBooleanConsoleVariable(*Blueprint, CVarName, DefaultValue))
		{
			return nullptr;
		}

		if (!CompileAndSaveBlueprint(*Blueprint))
		{
			return nullptr;
		}

		Context.TrackCreatedAsset(*Blueprint);
		return Blueprint;
	}

	// Create and save an unrelated Blueprint in the tracked test directory so no-load regression coverage can target it directly.
	static UBlueprint* CreateUnrelatedBlueprint(FEditorTestAssetContext& Context, const FString& AssetName)
	{
		UBlueprint* Blueprint = CreateBlueprintAsset(Context.GetDirectoryPath(), AssetName, UInputModifierSettingScalar::StaticClass());
		if (!Blueprint || !CompileAndSaveBlueprint(*Blueprint))
		{
			return nullptr;
		}

		Context.TrackCreatedAsset(*Blueprint);
		return Blueprint;
	}
} // namespace SB::AutoSettings::Editor::Tests

using namespace SB::AutoSettings::Editor::Tests;

TEST_CLASS_WITH_FLAGS(Registration, "SB.AutoSettings.Console.Assets",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
{
	// Create a console variable Blueprint, let the asset-registry callback register its native CVar, then verify the authored default value.
	TEST_METHOD(RegistersOnSave)
	{
		const TSharedRef<FEditorTestAssetContext> Context = MakeShared<FEditorTestAssetContext>(*TestRunner);
		const FName CVarName = MakeUniqueCVarName(TEXT("Register"));

		Context->Initialize();

		TestCommandBuilder
			// Always tear down the temporary asset directory so repeated automation runs never reuse old assets.
			.OnTearDown([Context]()
				{
					Context->Cleanup();
				})
			// Save the Blueprint through normal editor APIs so the real asset-registry callback path performs registration.
			.Do([Context, CVarName, this]()
				{
					UBlueprint* Blueprint = CreateBooleanBlueprint(*Context, TEXT("Registration"), CVarName, true);
					TestRunner->AddErrorIfFalse(Blueprint != nullptr, TEXT("Registration test created and saved a console variable Blueprint"));
				})
			// Wait for the observable subsystem effect instead of reaching into the subsystem directly.
			.Until([CVarName]()
				{
					return IsVariableRegisteredUntracked(CVarName);
				},
				FTimespan::FromSeconds(30))
			// Assert both registration and the authored value once the callback path has completed.
			.Then([CVarName, this]()
				{
					ASSERT_THAT(IsTrue(SB::AutoSettings::Console::IsVariableRegistered(CVarName)));
					ASSERT_THAT(IsTrue(SB::AutoSettings::Console::GetValue<bool>(CVarName)));
				});
	}
};

TEST_CLASS_WITH_FLAGS(Updates, "SB.AutoSettings.Console.Assets",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
{
	// Save with one name, update the same Blueprint defaults, then resave and verify the old CVar is replaced by the new one.
	TEST_METHOD(ReregistersOnRename)
	{
		const TSharedRef<FEditorTestAssetContext> Context = MakeShared<FEditorTestAssetContext>(*TestRunner);
		const FName OldName = MakeUniqueCVarName(TEXT("RenameOld"));
		const FName NewName = MakeUniqueCVarName(TEXT("RenameNew"));
		const TSharedRef<TStrongObjectPtr<UBlueprint>> Blueprint = MakeShared<TStrongObjectPtr<UBlueprint>>();

		Context->Initialize();

		TestCommandBuilder
			// Release the strong Blueprint reference before deleting the temporary asset directory during teardown.
			.OnTearDown([Context, Blueprint]()
				{
					Blueprint->Reset();
					Context->Cleanup();
				})
			// Create the initial asset and let the first asset-registry update register the original CVar name.
			.Do([Context, Blueprint, OldName, this]()
				{
					Blueprint->Reset(CreateBooleanBlueprint(*Context, TEXT("Rename"), OldName, false));
					TestRunner->AddErrorIfFalse(Blueprint->Get() != nullptr, TEXT("Rename test created the initial console variable Blueprint"));
				})
			.Until([OldName]()
				{
					return IsVariableRegisteredUntracked(OldName);
				},
				FTimespan::FromSeconds(30))
			// Mutate the Blueprint defaults and save again so the rename flows through the same editor callback path as production.
			.Then([Blueprint, NewName, this]()
				{
					if (!TestRunner->AddErrorIfFalse(Blueprint->Get() != nullptr, TEXT("Rename test kept the Blueprint alive for update")))
					{
						return;
					}

					if (!TestRunner->AddErrorIfFalse(ConfigureBooleanConsoleVariable(*Blueprint->Get(), NewName, true), TEXT("Rename test updated the Blueprint default object")))
					{
						return;
					}

					TestRunner->AddErrorIfFalse(CompileAndSaveBlueprint(*Blueprint->Get()), TEXT("Rename test saved the modified Blueprint"));
				})
			// Wait until the old registration disappears and the new one takes over.
			.Until([OldName, NewName]()
				{
					return IsVariableRegisteredUntracked(NewName) && !IsVariableRegisteredUntracked(OldName);
				},
				FTimespan::FromSeconds(30))
			// Verify the rename produced the expected native state and value.
			.Then([OldName, NewName, this]()
				{
					ASSERT_THAT(IsFalse(SB::AutoSettings::Console::IsVariableRegistered(OldName)));
					ASSERT_THAT(IsTrue(SB::AutoSettings::Console::IsVariableRegistered(NewName)));
					ASSERT_THAT(IsTrue(SB::AutoSettings::Console::GetValue<bool>(NewName)));
				});
	}
};

TEST_CLASS_WITH_FLAGS(Removal, "SB.AutoSettings.Console.Assets",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
{
	// Delete the temporary asset directory and wait for the asset-registry removal callback to unregister the native CVar.
	TEST_METHOD(UnregistersOnDelete)
	{
		const TSharedRef<FEditorTestAssetContext> Context = MakeShared<FEditorTestAssetContext>(*TestRunner);
		const FName CVarName = MakeUniqueCVarName(TEXT("Remove"));
		const TSharedRef<bool> bDeletionRequested = MakeShared<bool>(false);

		Context->Initialize();

		TestCommandBuilder
			// Keep teardown idempotent so explicit cleanup in the test body and CQTest teardown can both run safely.
			.OnTearDown([Context]()
				{
					Context->Cleanup();
				})
			// Create the console variable asset and wait for the initial registration before deleting it.
			.Do([Context, CVarName, this]()
				{
					UBlueprint* Blueprint = CreateBooleanBlueprint(*Context, TEXT("Removal"), CVarName, true);
					TestRunner->AddErrorIfFalse(Blueprint != nullptr, TEXT("Removal test created and saved a console variable Blueprint"));
				})
			.Until([CVarName]()
				{
					return IsVariableRegisteredUntracked(CVarName);
				},
				FTimespan::FromSeconds(30))
			// Delete the full temporary directory with the editor asset subsystem so the real removal callback does the unregister work.
			.Then([Context, bDeletionRequested]()
				{
					Context->Cleanup();
					*bDeletionRequested = true;
				})
			// Wait for the observable unregister side effect before asserting final state.
			.Until([CVarName, bDeletionRequested]()
				{
					return *bDeletionRequested && !IsVariableRegisteredUntracked(CVarName);
				},
				FTimespan::FromSeconds(30))
			// Confirm the native CVar is gone after deletion completes.
			.Then([CVarName, this]()
				{
					ASSERT_THAT(IsFalse(SB::AutoSettings::Console::IsVariableRegistered(CVarName)));
				});
	}
};

TEST_CLASS_WITH_FLAGS(Loading, "SB.AutoSettings.Console.Assets",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
{
	// Trigger the subsystem directly on an unrelated Blueprint asset and verify the asset stays unloaded afterward.
	TEST_METHOD(DoesNotLoadUnrelatedBlueprint)
	{
		const TSharedRef<FEditorTestAssetContext> Context = MakeShared<FEditorTestAssetContext>(*TestRunner);
		const TSharedRef<TStrongObjectPtr<UBlueprint>> UnrelatedBlueprint = MakeShared<TStrongObjectPtr<UBlueprint>>();
		const TSharedRef<FSoftObjectPath> UnrelatedBlueprintPath = MakeShared<FSoftObjectPath>();
		const TSharedRef<FAssetData> UnrelatedBlueprintAssetData = MakeShared<FAssetData>();

		Context->Initialize();

		TestCommandBuilder
			// Release the strong Blueprint reference before deleting the temporary directory so teardown can remove the asset cleanly.
			.OnTearDown([Context, UnrelatedBlueprint]()
				{
					UnrelatedBlueprint->Reset();
					Context->Cleanup();
				})
			// Save an unrelated Blueprint asset that the subsystem must ignore.
			.Do([Context, UnrelatedBlueprint, UnrelatedBlueprintPath, this]()
				{
					UnrelatedBlueprint->Reset(CreateUnrelatedBlueprint(*Context, TEXT("UnrelatedBlueprint")));
					if (!TestRunner->AddErrorIfFalse(UnrelatedBlueprint->Get() != nullptr, TEXT("No-load test created the unrelated Blueprint")))
					{
						return;
					}

					*UnrelatedBlueprintPath = FSoftObjectPath(UnrelatedBlueprint->Get());
				})
			// Wait until the asset registry knows about the saved Blueprint before we unload it for the no-load assertion.
			.Until([UnrelatedBlueprintPath]()
				{
					return GetAssetDataByObjectPath(*UnrelatedBlueprintPath).IsValid();
				},
				FTimespan::FromSeconds(30))
			// Cache the callback payload before unloading so the subsystem sees the same AssetData it would receive from the registry.
			.Then([UnrelatedBlueprintPath, UnrelatedBlueprintAssetData, this]()
				{
					*UnrelatedBlueprintAssetData = GetAssetDataByObjectPath(*UnrelatedBlueprintPath);
					TestRunner->AddErrorIfFalse(UnrelatedBlueprintAssetData->IsValid(), TEXT("No-load test cached Asset Registry data for the unrelated Blueprint before unload"));
				})
			// Start from a known unloaded state so any subsequent load is attributable to the subsystem under test.
			.Then([UnrelatedBlueprint, UnrelatedBlueprintPath, this]()
				{
					if (!TestRunner->AddErrorIfFalse(!UnrelatedBlueprintPath->ToString().IsEmpty(), TEXT("No-load test captured the unrelated Blueprint path")))
					{
						return;
					}

					UnrelatedBlueprint->Reset();
					if (!TestRunner->AddErrorIfFalse(UnloadAsset(*UnrelatedBlueprintPath), TEXT("No-load test unloaded the unrelated Blueprint package")))
					{
						return;
					}

					TestRunner->AddErrorIfFalse(!IsAssetLoaded(*UnrelatedBlueprintPath), TEXT("No-load test confirmed the unrelated Blueprint is unloaded before triggering the subsystem"));
				})
			// Call the exact subsystem behavior under test using asset data, without waiting on an editor callback race.
			.Then([UnrelatedBlueprintAssetData, this]()
				{
					UAutoSettingsConsoleEngineSubsystem* EngineSubsystem = GEngine ? GEngine->GetEngineSubsystem<UAutoSettingsConsoleEngineSubsystem>() : nullptr;
					if (!TestRunner->AddErrorIfFalse(EngineSubsystem != nullptr, TEXT("No-load test resolved the AutoSettings console engine subsystem")))
					{
						return;
					}

					if (!TestRunner->AddErrorIfFalse(UAssetManager::IsInitialized() && UAssetManager::Get().HasInitialScanCompleted(), TEXT("No-load test requires the asset manager initial scan to be complete")))
					{
						return;
					}

					if (!TestRunner->AddErrorIfFalse(UnrelatedBlueprintAssetData->IsValid(), TEXT("No-load test retained Asset Registry data for the unrelated Blueprint")))
					{
						return;
					}

					EngineSubsystem->OnAssetChanged(*UnrelatedBlueprintAssetData);
				})
			// If the subsystem eagerly loads unrelated Blueprints, the asset will now be resident and this assertion will fail.
			.Then([UnrelatedBlueprintPath, this]()
				{
					ASSERT_THAT(IsFalse(IsAssetLoaded(*UnrelatedBlueprintPath)));
				});
	}
};

#endif
