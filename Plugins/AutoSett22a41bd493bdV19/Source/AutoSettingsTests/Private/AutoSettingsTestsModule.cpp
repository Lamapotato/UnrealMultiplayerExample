// Copyright Sam Bonifacio. All Rights Reserved.

#include "Modules/ModuleManager.h"

/**
 * Developer-only module that owns AutoSettings automation coverage so runtime modules stay free of
 * test-only dependencies.
 */
class FAutoSettingsTestsModule final : public IModuleInterface
{
};

IMPLEMENT_MODULE(FAutoSettingsTestsModule, AutoSettingsTests)
