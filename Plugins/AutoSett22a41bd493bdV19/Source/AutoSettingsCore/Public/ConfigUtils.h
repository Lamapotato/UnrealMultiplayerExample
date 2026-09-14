// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Utility functions for configuration file operations and migrations.
 */
class AUTOSETTINGSCORE_API FConfigUtils
{
public:
	/**
	 * Migrate config properties from an old section/config to a new one.
	 *
	 * This function automatically moves all config properties from an old section
	 * to the object's current config section, supporting plugin config refactoring.
	 */
	static void MigrateConfigPropertiesFromSection(UObject* Object, FString OldConfig, FString OldSection);
};
