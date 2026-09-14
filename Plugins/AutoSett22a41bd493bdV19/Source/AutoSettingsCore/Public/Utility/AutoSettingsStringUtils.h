// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Utility functions for string operations used throughout AutoSettings.
 */
class AUTOSETTINGSCORE_API FAutoSettingsStringUtils
{
public:
	/** Check if a string represents a truthy value. Returns false only for "0" and "false" (case-insensitive) */
	static bool IsTruthy(const FString& String);
};
