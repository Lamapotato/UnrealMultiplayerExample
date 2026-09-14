// Copyright Sam Bonifacio. All Rights Reserved.

#include "ConsoleVariableObjectUtils.h"
#include "AutoSettingsConsoleVariable.h"
#include "Utility/AutoSettingsAssetUtils.h"

namespace SB::AutoSettings::Console
{
	TArray<UClass*> GetAllConsoleVariableClasses()
	{
		// Registration owns configuration validation so invalid classes remain observable instead of disappearing here.
		return FAutoSettingsAssetUtils::LoadAndGetAllClasses<UAutoSettingsConsoleVariable>(
			UAutoSettingsConsoleVariable::ConsoleVariablePrimaryAssetTypeName);
	}

	TArray<UAutoSettingsConsoleVariable*> GetAllConsoleVariableDefaultObjects()
	{
		TArray<UAutoSettingsConsoleVariable*> DefaultObjects;
		for (UClass* Class : GetAllConsoleVariableClasses())
		{
			DefaultObjects.Add(Class->GetDefaultObject<UAutoSettingsConsoleVariable>());
		}
		return DefaultObjects;
	}
} // namespace SB::AutoSettings::Console