// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UAutoSettingsConsoleVariable;

namespace SB::AutoSettings::Console
{
	/**
	 * Get all console variable classes from content assets and loaded reflected C++ classes.
	 * @return Array of concrete console variable classes
	 */
	TArray<UClass*> GetAllConsoleVariableClasses();

	/**
	 * Get all console variable default objects
	 * @return Array of console variable CDOs
	 */
	TArray<UAutoSettingsConsoleVariable*> GetAllConsoleVariableDefaultObjects();
} // namespace SB::AutoSettings::Console