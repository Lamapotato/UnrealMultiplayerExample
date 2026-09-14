// Copyright Sam Bonifacio. All Rights Reserved.

#include "AutoSettingsInput.h"

#include "Processing/AutoSettingsGlobalInputProcessor.h"
#include "Misc/CoreDelegates.h"

#define LOCTEXT_NAMESPACE "FAutoSettingsModule"

void FAutoSettingsInputModule::StartupModule()
{
}

void FAutoSettingsInputModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAutoSettingsInputModule, AutoSettingsInput)
