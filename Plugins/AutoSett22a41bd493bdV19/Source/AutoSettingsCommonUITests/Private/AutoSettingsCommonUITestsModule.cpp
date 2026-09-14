// Copyright Sam Bonifacio. All Rights Reserved.

#include "Modules/ModuleManager.h"

/**
 * Developer-only module that owns automation coverage for optional CommonUI integration so the core
 * AutoSettings test module stays independent of CommonUI binaries.
 */
class FAutoSettingsCommonUITestsModule final : public IModuleInterface
{
};

IMPLEMENT_MODULE(FAutoSettingsCommonUITestsModule, AutoSettingsCommonUITests)
