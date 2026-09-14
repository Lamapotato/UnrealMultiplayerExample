// Copyright Sam Bonifacio. All Rights Reserved.

#include "EditorTestAssetContext.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AssetRegistry/AssetRegistryModule.h"
#include "DirectoryWatcherModule.h"
#include "Editor.h"
#include "HAL/FileManager.h"
#include "FileHelpers.h"
#include "IDirectoryWatcher.h"
#include "Misc/EngineVersionComparison.h"
#include "Misc/Guid.h"
#include "Misc/PackageName.h"
#if !UE_VERSION_OLDER_THAN(5, 8, 0)
#include "Misc/ScopedCVar.h"
#endif
#include "Modules/ModuleManager.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Subsystems/EditorAssetSubsystem.h"
#include "Templates/Function.h"

namespace SB::Editor::Tests
{
	namespace
	{
		// Keep all test-created assets under a temporary automation root so editor test runs do not pollute real content.
		constexpr const TCHAR* TestAutomationAssetRoot = TEXT("/Game/Temp/Automation");

		// Cleanup helpers report failures through this callback so the context owns result state and test reporting.
		using FCleanupErrorReporter = TFunctionRef<void(const FString& Message)>;

		// Generate a stable unique token per test instance so temporary asset directories never collide across runs.
		FString MakeUniqueAssetContextToken()
		{
			return FGuid::NewGuid().ToString(EGuidFormats::Digits);
		}

		// Collapse arbitrary test names into package-safe folder names by keeping alphanumerics and replacing everything else with underscores.
		FString MakePackageSafeSegment(const FString& Source)
		{
			FString Result;
			Result.Reserve(Source.Len());

			for (const TCHAR Character : Source)
			{
				Result.AppendChar(FChar::IsAlnum(Character) ? Character : TEXT('_'));
			}

			while (Result.ReplaceInline(TEXT("__"), TEXT("_"), ESearchCase::CaseSensitive) > 0)
			{
			}

			while (Result.StartsWith(TEXT("_")))
			{
				Result.RightChopInline(1, EAllowShrinking::No);
			}

			while (Result.EndsWith(TEXT("_")))
			{
				Result.LeftChopInline(1, EAllowShrinking::No);
			}

			return Result.IsEmpty() ? TEXT("UnnamedTest") : Result;
		}

		// Pump the filesystem watcher that feeds AssetRegistry package-file notifications.
		bool TickDirectoryWatcher()
		{
			FDirectoryWatcherModule& DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
			IDirectoryWatcher* DirectoryWatcher = DirectoryWatcherModule.Get();
			if (!ensureMsgf(DirectoryWatcher, TEXT("DirectoryWatcher must be available for automation asset cleanup.")))
			{
				return false;
			}

			DirectoryWatcher->Tick(1.0f);
			return true;
		}

		// Pump AssetRegistry after filesystem notifications so package state catches up to disk changes.
		void TickAssetRegistry()
		{
			FAssetRegistryModule::TickAssetRegistry(1.0f);
		}

		// Drain async loading, garbage collection, filesystem notifications, and registry work at cleanup phase boundaries.
		bool PumpAssetFilesystemEvents()
		{
			FlushAsyncLoading();
			CollectGarbage(RF_NoFlags);

			const bool bDirectoryWatcherTicked = TickDirectoryWatcher();
			TickAssetRegistry();

			return bDirectoryWatcherTicked;
		}

		// Delete all assets in the owned automation directory through Unreal's editor asset lifecycle.
		void DeleteAutomationDirectory(const FString& DirectoryPath, FCleanupErrorReporter ReportCleanupError)
		{
			// Skip cleanup when the context did not create a physical directory or it has already disappeared.
			const FString DirectoryFileName = DirectoryPath.IsEmpty()
				? FString()
				: FPackageName::LongPackageNameToFilename(DirectoryPath, TEXT(""));
			if (DirectoryFileName.IsEmpty() || !IFileManager::Get().DirectoryExists(*DirectoryFileName))
			{
				return;
			}

			// Flush queued file notifications before a forced scan establishes the directory's complete asset set.
			if (!PumpAssetFilesystemEvents())
			{
				ReportCleanupError(TEXT("Cleanup could not synchronize automation assets before deleting their directory."));
				return;
			}

			// Populate directory enumeration synchronously because editor deletion discovers assets through Asset Registry paths.
			FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
			AssetRegistryModule.Get().ScanPathsSynchronous({ DirectoryPath }, true);

			UEditorAssetSubsystem* EditorAssetSubsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
			if (!EditorAssetSubsystem)
			{
				ReportCleanupError(TEXT("Cleanup could not access the editor asset subsystem."));
				return;
			}

			// The optimized reference check can falsely mark force-deleted test packages as referenced,
			// causing ObjectTools to skip their .uasset files; use the legacy check for deterministic cleanup.
#if !UE_VERSION_OLDER_THAN(5, 8, 0)
			FScopedCVar<bool> UseLegacyDeletionReferencer(TEXT("Editor.UseLegacyGetReferencersForDeletion"), true);
#endif

			// The subsystem owns asset loading, package deletion, registry removal, and empty-directory removal.
			if (!EditorAssetSubsystem->DeleteDirectory(DirectoryPath))
			{
				ReportCleanupError(FString::Printf(TEXT("Cleanup could not delete automation directory '%s'."), *DirectoryPath));
			}
		}
	} // namespace

	FAssetData GetAssetDataByObjectPath(const FSoftObjectPath& AssetPath)
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		return AssetRegistryModule.Get().GetAssetByObjectPath(AssetPath);
	}

	bool IsAssetLoaded(const FSoftObjectPath& AssetPath)
	{
		return AssetPath.ResolveObject() != nullptr;
	}

	bool UnloadAsset(const FSoftObjectPath& AssetPath)
	{
		UObject* Asset = AssetPath.ResolveObject();
		if (!Asset)
		{
			return true;
		}

		if (GEditor)
		{
			if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
			{
				AssetEditorSubsystem->CloseAllEditorsForAsset(Asset);
			}
		}

		UPackage* Package = Asset->GetOutermost();
		if (!Package)
		{
			return false;
		}

		bool bAnyPackagesUnloaded = false;
		FText ErrorMessage;
		UEditorLoadingAndSavingUtils::UnloadPackages({ Package }, bAnyPackagesUnloaded, ErrorMessage);

		FlushAsyncLoading();
		CollectGarbage(RF_NoFlags);
		return !IsAssetLoaded(AssetPath);
	}

	TSharedRef<FEditorTestAssetContext> FEditorTestAssetContext::Create(FAutomationTestBase& InTestRunner)
	{
		// Shared ownership keeps the context alive until queued latent cleanup commands have run.
		TSharedRef<FEditorTestAssetContext> AssetContext = MakeShared<FEditorTestAssetContext>(InTestRunner);
		AssetContext->Initialize();
		return AssetContext;
	}

	FEditorTestAssetContext::FEditorTestAssetContext(FAutomationTestBase& InTestRunner)
		: TestRunner(&InTestRunner)
	{
	}

	void FEditorTestAssetContext::Initialize()
	{
		DirectoryPath = MakeUniqueTestDirectory();
		CreatedAssetPaths.Reset();
		bCleanupAttempted = false;
		bCleanupSucceeded = false;
	}

	const FString& FEditorTestAssetContext::GetDirectoryPath() const
	{
		return DirectoryPath;
	}

	void FEditorTestAssetContext::TrackCreatedAsset(const UObject& Asset)
	{
		CreatedAssetPaths.AddUnique(FSoftObjectPath(&Asset));
	}

	bool FEditorTestAssetContext::Cleanup()
	{
		if (bCleanupAttempted)
		{
			return bCleanupSucceeded;
		}

		bCleanupAttempted = true;
		bCleanupSucceeded = true;

		if (!GEditor)
		{
			bCleanupSucceeded = CreatedAssetPaths.IsEmpty() && DirectoryPath.IsEmpty();
			if (!bCleanupSucceeded)
			{
				ReportCleanupError(TEXT("Cleanup could not run because GEditor was unavailable."));
			}

			return bCleanupSucceeded;
		}

		// Mark all helper-reported failures on the idempotent cleanup result and mirror them into the owning automation test.
		const auto ReportCleanupFailure = [this](const FString& Message)
		{
			bCleanupSucceeded = false;
			ReportCleanupError(Message);
		};

		DeleteAutomationDirectory(DirectoryPath, ReportCleanupFailure);

		return bCleanupSucceeded;
	}

	void FEditorTestAssetContext::ReportCleanupError(const FString& Message)
	{
		if (TestRunner)
		{
			TestRunner->AddError(Message);
		}
	}

	FString FEditorTestAssetContext::MakeTestDirectoryName() const
	{
		// Prefer the per-case context so directory names stay compact, then fall back to broader test identifiers when needed.
		FString SourceName = TestRunner ? TestRunner->GetTestContext() : FString();
		if (SourceName.IsEmpty() && TestRunner)
		{
			SourceName = TestRunner->GetBeautifiedTestName();
		}

		if (SourceName.IsEmpty() && TestRunner)
		{
			SourceName = TestRunner->GetTestName();
		}

		return MakePackageSafeSegment(SourceName);
	}

	FString FEditorTestAssetContext::MakeUniqueTestDirectory() const
	{
		return FString::Printf(TEXT("%s/%s/%s"), TestAutomationAssetRoot, *MakeTestDirectoryName(), *MakeUniqueAssetContextToken());
	}
} // namespace SB::Editor::Tests

#endif
