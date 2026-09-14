// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"

class UAutoSettingsConsoleEngineSubsystem;
class UAutoSettingsConsoleVariable;

namespace SB::AutoSettings::Tests
{
	/** Activates one native AutoSettings console-variable class for a test fixture lifetime. */
	class FScopedAutoSettingsConsoleVariableClass final
	{
	public:
		FScopedAutoSettingsConsoleVariableClass(UAutoSettingsConsoleEngineSubsystem& InSubsystem,
			TSubclassOf<UAutoSettingsConsoleVariable> InClass, FName InName);
		~FScopedAutoSettingsConsoleVariableClass();

		FScopedAutoSettingsConsoleVariableClass(const FScopedAutoSettingsConsoleVariableClass&) = delete;
		FScopedAutoSettingsConsoleVariableClass& operator=(const FScopedAutoSettingsConsoleVariableClass&) = delete;

		/** Reports whether the production subsystem accepted the class registration. */
		bool IsRegistered() const { return bRegistered; }

	private:
		/** Production owner used to unregister the class after PIE shutdown. */
		UAutoSettingsConsoleEngineSubsystem* Subsystem = nullptr;

		/** Native fixture class registered with the production subsystem. */
		TSubclassOf<UAutoSettingsConsoleVariable> ConsoleVariableClass;

		/** Class definition whose configured name is restored after the test. */
		UAutoSettingsConsoleVariable* Definition = nullptr;

		/** Name present on the class definition before the scope activated it. */
		FName PreviousName;

		/** True only when this scope owns a successful subsystem registration. */
		bool bRegistered = false;
	};
}
