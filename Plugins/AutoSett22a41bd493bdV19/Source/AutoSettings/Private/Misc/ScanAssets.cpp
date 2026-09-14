// Copyright Sam Bonifacio. All Rights Reserved.

#include "Engine/AssetManager.h"
#include "AutoSettingsConsoleVariable.h"
#include "Framework/SettingType.h"

bool IsAssetRulesRegistered(FPrimaryAssetType AssetType)
{
	const FPrimaryAssetId DummySettingAssetId(AssetType, NAME_None);
	const FPrimaryAssetRules GameDataRules = UAssetManager::Get().GetPrimaryAssetRules(DummySettingAssetId);
	return !GameDataRules.IsDefault();
}

// Register asset scanning during engine pre-initialization to ensure AutoSettings assets
// are discovered before other systems try to use them during startup
static FDelayedAutoRegisterHelper ScanAssets(EDelayedRegisterRunPhase::StartOfEnginePreInit, []()
	{
		// Wait for Asset Manager to be fully created before registering asset types
		// This avoids race conditions during engine startup
		UAssetManager::CallOrRegister_OnAssetManagerCreated(FSimpleMulticastDelegate::FDelegate::CreateLambda([]()
			{
				// Only scan if rules aren't already registered to avoid duplicate asset scanning

				if (!IsAssetRulesRegistered(USettingType::SettingPrimaryAssetTypeName))
				{
					// Scan entire /Game directory for SettingType assets
					UAssetManager::Get().ScanPathForPrimaryAssets(USettingType::SettingPrimaryAssetTypeName, "/Game",
						USettingType::StaticClass(), true, false, false);
				}

				if (!IsAssetRulesRegistered(UAutoSettingsConsoleVariable::ConsoleVariablePrimaryAssetTypeName))
				{
					// Scan for console variable assets
					UAssetManager::Get().ScanPathForPrimaryAssets(UAutoSettingsConsoleVariable::ConsoleVariablePrimaryAssetTypeName,
						"/Game",
						UAutoSettingsConsoleVariable::StaticClass(), true, false, false);
				}

				// Mark assets as AlwaysCook to ensure they're included in packaged builds
				// This guarantees settings are available at runtime without needing to be loaded on-demand
				FPrimaryAssetRules AssetRules;
				AssetRules.CookRule = EPrimaryAssetCookRule::AlwaysCook;
				UAssetManager::Get().SetPrimaryAssetTypeRules(USettingType::SettingPrimaryAssetTypeName, AssetRules);
				UAssetManager::Get().SetPrimaryAssetTypeRules(UAutoSettingsConsoleVariable::ConsoleVariablePrimaryAssetTypeName, AssetRules);
			}));
	});