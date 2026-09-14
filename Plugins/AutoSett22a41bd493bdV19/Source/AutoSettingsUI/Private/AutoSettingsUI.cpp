// Copyright Sam Bonifacio. All Rights Reserved.

#include "AutoSettingsUI.h"

#define LOCTEXT_NAMESPACE "FAutoSettingsUIModule"

void FAutoSettingsUIModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file
}

void FAutoSettingsUIModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAutoSettingsUIModule, AutoSettingsUI)