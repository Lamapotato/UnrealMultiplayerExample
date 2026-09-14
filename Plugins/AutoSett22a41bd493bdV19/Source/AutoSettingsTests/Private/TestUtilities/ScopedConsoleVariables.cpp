// Copyright Sam Bonifacio. All Rights Reserved.

#include "TestUtilities/ScopedConsoleVariables.h"

#include "AutoSettingsConsoleEngineSubsystem.h"
#include "AutoSettingsConsoleVariable.h"

namespace SB::AutoSettings::Tests
{
	FScopedAutoSettingsConsoleVariableClass::FScopedAutoSettingsConsoleVariableClass(
		UAutoSettingsConsoleEngineSubsystem& InSubsystem,
		TSubclassOf<UAutoSettingsConsoleVariable> InClass,
		const FName InName)
		: Subsystem(&InSubsystem)
		, ConsoleVariableClass(InClass)
	{
		Definition = ConsoleVariableClass
			? ConsoleVariableClass->GetDefaultObject<UAutoSettingsConsoleVariable>()
			: nullptr;
		if (!Definition)
		{
			return;
		}

		// The native fixture stays undiscoverable until its test scope supplies an explicit name.
		PreviousName = Definition->Name;
		Definition->Name = InName;
		bRegistered = Subsystem->RegisterConsoleVariable(ConsoleVariableClass);
	}

	FScopedAutoSettingsConsoleVariableClass::~FScopedAutoSettingsConsoleVariableClass()
	{
		// The subsystem owns native CVar tracking; remove the class before restoring its inert definition.
		if (bRegistered && Subsystem)
		{
			Subsystem->UnregisterConsoleVariable(ConsoleVariableClass);
		}
		if (Definition)
		{
			Definition->Name = PreviousName;
		}
	}
}
