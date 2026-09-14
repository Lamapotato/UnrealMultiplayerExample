// Copyright Sam Bonifacio. All Rights Reserved.

#include "ConsoleUtilities.h"
#include "AutoSettingsError.h"

namespace SB::AutoSettings::Console
{
	IConsoleVariable* GetVariable(FName Name)
	{
		IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*Name.ToString());

		if (!CVar)
		{
			SB::AutoSettings::Errors::LogError(FString::Printf(TEXT("Tried to find console variable '%s' that doesn't exist"),
				*Name.ToString()));
		}

		return CVar;
	}

	bool IsVariableRegistered(FName Name)
	{
		IConsoleObject* ExistingConsoleObject = IConsoleManager::Get().FindConsoleObject(*Name.ToString());

		if (!ExistingConsoleObject)
		{
			return false;
		}

		bool bIsUnregistered = ExistingConsoleObject->TestFlags(ECVF_Unregistered);
		return !bIsUnregistered;
	}
} // namespace SB::AutoSettings::Console