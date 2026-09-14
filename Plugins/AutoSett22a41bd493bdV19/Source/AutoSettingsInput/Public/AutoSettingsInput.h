// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "Processing/AutoSettingsGlobalInputProcessor.h"
#include "Modules/ModuleManager.h"

/**
 * Module for AutoSettings input mapping and rebinding functionality.
 */
class FAutoSettingsInputModule : public IModuleInterface
{
public:
	// Static get module
	static inline FAutoSettingsInputModule* Get()
	{
		return FModuleManager::LoadModulePtr<FAutoSettingsInputModule>("AutoSettingsInput");
	}

protected:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	FAutoSettingsGlobalInputProcessorRegistrar PreProcessorHandler;
};
