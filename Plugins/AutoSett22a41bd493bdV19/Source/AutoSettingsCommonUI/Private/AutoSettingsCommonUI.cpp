// Copyright Sam Bonifacio. All Rights Reserved.
#include "AutoSettingsCommonUI.h"
#include "AutoSettingsLogs.h"

#define LOCTEXT_NAMESPACE "FAutoSettingsCommonUIModule"

void FAutoSettingsCommonUIModule::StartupModule()
{
	UE_LOG(LogAutoSettings, Log, TEXT("AutoSettingsCommonUI loaded"));

#if WITH_DEV_AUTOMATION_TESTS
	// Load CommonUI-specific automation coverage only after the optional integration module is active.
	FModuleManager::Get().LoadModule(TEXT("AutoSettingsCommonUITests"));
#endif
}

void FAutoSettingsCommonUIModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAutoSettingsCommonUIModule, AutoSettingsCommonUI)
