// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AssetRegistry/AssetData.h"
#include "Misc/AutomationTest.h"
#include "UObject/SoftObjectPath.h"

class UObject;

namespace SB::Editor::Tests
{
	// Look up saved asset metadata without loading so tests can inspect unloaded assets by path.
	SBAUTOMATIONAUTOSETTINGSEDITOR_API FAssetData GetAssetDataByObjectPath(const FSoftObjectPath& AssetPath);

	// Resolve an asset pointer without loading so tests can detect unexpected loads.
	SBAUTOMATIONAUTOSETTINGSEDITOR_API bool IsAssetLoaded(const FSoftObjectPath& AssetPath);

	// Close any editors and unload the package that owns the asset so later assertions start from an unloaded state.
	SBAUTOMATIONAUTOSETTINGSEDITOR_API bool UnloadAsset(const FSoftObjectPath& AssetPath);

	// Track editor-created automation assets for a single test and clean them up with self-reporting verification.
	struct SBAUTOMATIONAUTOSETTINGSEDITOR_API FEditorTestAssetContext
	{
		// Create and initialize a shared context so latent automation commands can safely keep cleanup ownership alive.
		static TSharedRef<FEditorTestAssetContext> Create(FAutomationTestBase& InTestRunner);

		explicit FEditorTestAssetContext(FAutomationTestBase& InTestRunner);

		// Initialize a new per-test directory under /Game/Temp/Automation/<TestName>/<RunId> so assets do not collide across runs.
		void Initialize();

		// Expose the active per-test directory so test-specific creation helpers can save assets under it.
		const FString& GetDirectoryPath() const;

		// Track a saved asset so cleanup can delete and verify it even if the test later unloads it.
		void TrackCreatedAsset(const UObject& Asset);

		// Delete tracked assets, remove the unique directory, and report any cleanup failures through the owning test.
		bool Cleanup();

	private:
		// Report cleanup failures directly through the owning automation test so leaks never depend on call-site boilerplate.
		void ReportCleanupError(const FString& Message);

		// Build a package-safe test directory name from the most specific available test identifier.
		FString MakeTestDirectoryName() const;

		// Build a unique run directory under /Game/Temp/Automation/<TestName>/<RunId>.
		FString MakeUniqueTestDirectory() const;

		// Owning automation test receives cleanup errors so leaked assets fail the test that created them.
		FAutomationTestBase* TestRunner = nullptr;

		// Package directory where the current test should create temporary automation assets.
		FString DirectoryPath;

		// Soft paths keep cleanup independent from whether tracked assets remain loaded.
		TArray<FSoftObjectPath> CreatedAssetPaths;

		// Cleanup is idempotent so test bodies and teardown callbacks can both request it safely.
		bool bCleanupAttempted = false;

		// Cached cleanup result is returned for repeated cleanup calls without re-running destructive editor operations.
		bool bCleanupSucceeded = false;
	};
} // namespace SB::Editor::Tests

#endif
