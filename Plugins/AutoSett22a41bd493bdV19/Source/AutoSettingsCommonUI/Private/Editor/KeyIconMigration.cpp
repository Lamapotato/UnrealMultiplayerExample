// Copyright Sam Bonifacio. All Rights Reserved.

#include "Engine/Blueprint.h"
#if WITH_EDITOR

#include "AutoSettingsLogs.h"
#include "IContentBrowserSingleton.h"
#include "CommonInputBaseTypes.h"
#include "AutoSettingsInput/Public/AutoSettingsInputConfig.h"

static FAutoConsoleCommand MigrateKeyIconsCommand(
	TEXT("AutoSettings.MigrateKeyIcons"),
	TEXT("Migrate a deprecated Key Icon Set to CommonInput ControllerData asset. First arg must be index of source Key Icon Set in project settings."),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			int32 Index = INDEX_NONE;

			if (Args.Num() >= 0)
			{
				Index = FCString::Atoi(*Args[0]);
			}

			if (Index == INDEX_NONE)
			{
				UE_LOG(LogAutoSettings, Error, TEXT("First argument must be index of deprecated Key Icon Set in project settings"))
				return;
			}

			const UAutoSettingsInputConfig* Config = GetDefault<UAutoSettingsInputConfig>();

			if (!Config->KeyIconSets_DEPRECATED.IsValidIndex(Index))
			{
				UE_LOG(LogAutoSettings, Error, TEXT("Could not find index %i in deprecated KeyIconSets"), Index);
				return;
			}

			TArray<FAssetData> SelectedAssets;
			IContentBrowserSingleton& ContentBrowserSingleton = IContentBrowserSingleton::Get();
			ContentBrowserSingleton.GetSelectedAssets(SelectedAssets);

			TArray<TSubclassOf<UCommonInputBaseControllerData>> SelectedControllerDatas;

			for (FAssetData& Asset : SelectedAssets)
			{
				UBlueprint* Loaded = Cast<UBlueprint>(Asset.GetAsset());
				if (IsValid(Loaded))
				{
					TSubclassOf<UCommonInputBaseControllerData> Subclass{ Loaded->GeneratedClass };
					SelectedControllerDatas.Add(Subclass);
				}
			}

			if (SelectedControllerDatas.Num() != 1)
			{
				UE_LOG(LogAutoSettings, Error, TEXT("Expected exactly 1 selected ControllerData asset, num selected: %i"), SelectedControllerDatas.Num());
			}

			TSubclassOf<UCommonInputBaseControllerData> ControllerDataClass = SelectedControllerDatas[0];
			UCommonInputBaseControllerData* CDO = GetMutableDefault<UCommonInputBaseControllerData>(ControllerDataClass);
			ControllerDataClass->Modify();
			CDO->Modify();

			CDO->InputBrushDataMap.Reset();

			const FKeyIconSet& KeyIconSet = Config->KeyIconSets_DEPRECATED[Index];

			for (const auto& Pair : KeyIconSet.IconMap)
			{
				FCommonInputKeyBrushConfiguration KeyBrushConfiguration;
				KeyBrushConfiguration.Key = Pair.Key;
				KeyBrushConfiguration.KeyBrush.SetResourceObject(Pair.Value.LoadSynchronous());
				CDO->InputBrushDataMap.Add(KeyBrushConfiguration);
			}
		}),
	ECVF_Default);

#endif