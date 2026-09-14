// Copyright Sam Bonifacio. All Rights Reserved.

#include "SBAutomationAutoSettingsCommon.h"

// Compile automation helper implementations only when development automation support is enabled.
#if WITH_DEV_AUTOMATION_TESTS

#include "SBAutomationAutoSettingsLogs.h"
#include "Tests/AutomationCommon.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "AssetRegistry/AssetData.h"

// Currently a copy of GetAnyGameWorld() in AutomationCommon.cpp, which itself has a comment about being a temporary solution
UWorld* SBAutomationAutoSettingsCommon::GetAnyGameWorld()
{
	UWorld* TestWorld = nullptr;
    const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
    for ( const FWorldContext& Context : WorldContexts )
    {
    	if ( ( ( Context.WorldType == EWorldType::PIE ) || ( Context.WorldType == EWorldType::Game ) ) && ( Context.World() != NULL ) )
    	{
    		TestWorld = Context.World();
    		break;
    	}
    }

    return TestWorld;
}

bool SBAutomationAutoSettingsCommon::OpenMap(FAutomationTestBase* Test, const FString& MapPath)
{
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FAssetData> ExistingAssets;
	AssetRegistryModule.Get().GetAssetsByPackageName(FName(MapPath), ExistingAssets);

	if(ExistingAssets.Num() == 0)
	{
	    Test->AddError(TEXT("No asset found at path"));
		return false;
	}

	const FTopLevelAssetPath MapAssetClassPath = ExistingAssets[0].AssetClassPath;

	if(MapAssetClassPath != UWorld::StaticClass()->GetClassPathName())
	{
		Test->AddError(TEXT("Asset was not a world"));
		return false;
	}

	const FString MapAssetName = ExistingAssets[0].AssetName.ToString();
	UE_LOG(LogSBAutomationAutoSettings, Display, TEXT("Found map asset: %s"), *MapAssetName);

	const bool bCanProceed = AutomationOpenMap(MapPath);
	if(!bCanProceed)
	{
		Test->AddError(TEXT("Open map failed"));
		return false;
	}

	ADD_LATENT_AUTOMATION_COMMAND(FDelegateCommand([Test, MapAssetName]()
	{
		const UWorld* World = GetAnyGameWorld();

		Test->TestEqual(TEXT("Loaded map name"), World->GetName(), MapAssetName);
	}));

	return true;
}

bool FDelegateCommand::Update()
{
	Function();
	return true;
}

bool FWaitTickCommand::Update()
{
	return true;
}

#endif
