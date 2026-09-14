// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/**
 * Main module for the AutoSettings plugin.
 */
class FAutoSettingsModule : public IModuleInterface
{
public:
	// Static get module
	static FAutoSettingsModule* Get()
	{
		return FModuleManager::LoadModulePtr<FAutoSettingsModule>("AutoSettings");
	}

protected:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};