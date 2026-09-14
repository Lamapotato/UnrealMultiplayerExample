// Copyright Sam Bonifacio. All Rights Reserved.

#include "AutoSettings.h"
#include "AutoSettingsLogs.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FAutoSettingsModule"

void FAutoSettingsModule::StartupModule()
{
	UE_LOG(LogAutoSettings, Log, TEXT("AutoSettings loaded"));

	if (IPluginManager::Get().FindPlugin("CommonUI").IsValid())
	{
		// Load the optional CommonUI integration module
		FModuleManager::Get().LoadModule("AutoSettingsCommonUI");
	}
}

void FAutoSettingsModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAutoSettingsModule, AutoSettings)