// Copyright Sam Bonifacio. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "CQTest.h"
#include "Editor.h"
#include "EditorTestAssetContext.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "DirectoryWatcherModule.h"
#include "IDirectoryWatcher.h"
#include "LevelEditorSubsystem.h"
#include "Misc/OutputDevice.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"

namespace
{
	// Convert a map package name to the physical package file so diagnostics can be matched to this temp map.
	FString GetMapPackageFileName(const FString& PackageName)
	{
		return FPaths::ConvertRelativePathToFull(FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetMapPackageExtension()));
	}

	// Capture AssetRegistry warnings and errors that reference one physical package file during the assertion window.
	class FScopedAssetRegistryPackageDiagnostics final : public FOutputDevice
	{
	public:
		// Register immediately so diagnostics emitted by cleanup and subsequent watcher ticks are captured.
		explicit FScopedAssetRegistryPackageDiagnostics(FString InMapPackageFileName)
			: MapPackageFileName(MoveTemp(InMapPackageFileName))
		{
			check(GLog);
			GLog->AddOutputDevice(this);
		}

		// Remove the output device before test teardown proceeds to avoid observing unrelated later logs.
		~FScopedAssetRegistryPackageDiagnostics() override
		{
			if (GLog)
			{
				GLog->RemoveOutputDevice(this);
			}
		}

		// Store only AssetRegistry diagnostics that are warnings-or-worse and mention the exact temp map file.
		void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override
		{
			const bool bIsAssetRegistry = Category == FName(TEXT("LogAssetRegistry"));
			const bool bIsWarningOrWorse = (Verbosity & ELogVerbosity::VerbosityMask) <= ELogVerbosity::Warning;
			const FString Message(V);

			if (bIsAssetRegistry && bIsWarningOrWorse && Message.Contains(MapPackageFileName))
			{
				Diagnostics.Add(Message);
			}
		}

		// Report whether the deleted package produced any diagnostics that a clean cleanup must avoid.
		bool HasDiagnostics() const
		{
			return !Diagnostics.IsEmpty();
		}

		// Join captured diagnostics so assertion output identifies the exact package-file signal.
		FString GetDiagnosticsText() const
		{
			return FString::Join(Diagnostics, TEXT("\n"));
		}

	private:
		// Physical map-package file path scopes the capture to the temp map created by this test.
		FString MapPackageFileName;

		// Captured diagnostics remain ordered so failure output follows the log sequence.
		TArray<FString> Diagnostics;
	};

	// Pump the filesystem watcher used by AssetRegistry so pending package-file events are observed inside the test.
	void TickDirectoryWatcher()
	{
		FDirectoryWatcherModule& DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
		if (IDirectoryWatcher* DirectoryWatcher = DirectoryWatcherModule.Get())
		{
			DirectoryWatcher->Tick(1.0f);
		}
	}

	// Pump AssetRegistry after directory-watcher events so registry state catches up before assertions run.
	void TickAssetRegistry()
	{
		FAssetRegistryModule::TickAssetRegistry(1.0f);
	}
}

TEST_CLASS(AssetContext, "SB.Automation")
{
	// Shared cleanup ownership lets the test body and CQTest cleanup both ask for cleanup safely.
	TSharedPtr<SB::Editor::Tests::FEditorTestAssetContext> AssetContext;

	BEFORE_EACH()
	{
		// Create one unique temp-asset context per test and register teardown before creating editor content.
		AssetContext = SB::Editor::Tests::FEditorTestAssetContext::Create(*TestRunner);

		TestCommandBuilder.CleanUpWith(TEXT("Clean tracked SBAutomationAutoSettings editor test assets"), [this]()
			{
				if (AssetContext.IsValid())
				{
					AssetContext->Cleanup();
					AssetContext.Reset();
				}
			});
	}

	// Cleanup should fully settle editor-created maps so delayed AssetRegistry watcher ticks cannot resurrect them.
	TEST_METHOD(CleanupDoesNotResurrectDeletedMapAfterDirectoryWatcherTick)
	{
		ASSERT_THAT(IsNotNull(GEditor, TEXT("Editor asset cleanup tests require GEditor.")));
		ASSERT_THAT(IsTrue(AssetContext.IsValid(), TEXT("Test asset context should be initialized before creating a temp map.")));

		// Create a saved map through the editor subsystem so cleanup exercises the real map package lifecycle.
		ULevelEditorSubsystem* LevelEditorSubsystem = GEditor->GetEditorSubsystem<ULevelEditorSubsystem>();
		ASSERT_THAT(IsNotNull(LevelEditorSubsystem, TEXT("Level editor subsystem should be available for temp map creation.")));

		const FString TempMapPackageName = AssetContext->GetDirectoryPath() / TEXT("NewMap");
		ASSERT_THAT(IsTrue(LevelEditorSubsystem->NewLevel(TempMapPackageName), TEXT("Temp map should be created under the asset context directory.")));

		UWorld* World = GEditor->GetEditorWorldContext().World();
		ASSERT_THAT(IsNotNull(World, TEXT("Editor world should point at the newly created temp map.")));

		const FString MapPackageFileName = GetMapPackageFileName(World->GetOutermost()->GetName());
		AssetContext->TrackCreatedAsset(*World);

		ASSERT_THAT(IsTrue(LevelEditorSubsystem->SaveCurrentLevel(), TEXT("Temp map should save before cleanup deletes it.")));

		// Leave the temp map before deletion so the editor can unload and delete the map package.
		ASSERT_THAT(IsTrue(LevelEditorSubsystem->LoadLevel(TEXT("/Engine/Maps/Entry")), TEXT("Entry map should load before temp map cleanup.")));

		FScopedAssetRegistryPackageDiagnostics AssetRegistryDiagnostics(MapPackageFileName);
		ASSERT_THAT(IsTrue(AssetContext->Cleanup(), TEXT("Asset context cleanup should delete the tracked temp map.")));
		AssetContext.Reset();

		// Force the delayed file notification path that can reintroduce a deleted map into AssetRegistry state.
		TickDirectoryWatcher();
		TickAssetRegistry();

		ASSERT_THAT(IsFalse(AssetRegistryDiagnostics.HasDiagnostics(), FString::Printf(
			TEXT("Cleanup should not let AssetRegistry emit warnings or errors for the deleted temp map. Diagnostics=%s"),
			*AssetRegistryDiagnostics.GetDiagnosticsText())));
	}
};

#endif
