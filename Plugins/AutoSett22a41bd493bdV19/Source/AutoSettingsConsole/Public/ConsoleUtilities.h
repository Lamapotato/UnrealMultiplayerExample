// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/IConsoleManager.h"
#include "AutoSettingsLogs.h"
#include "Logging/StructuredLog.h"

/**
 * Console variable utilities
 */
namespace SB::AutoSettings::Console
{
	/**
	 * General helper functions
	 */

	/** Returns the CVar with the specified name */
	AUTOSETTINGSCONSOLE_API IConsoleVariable* GetVariable(FName Name);

	/** True if the CVar with the specified name is registered */
	AUTOSETTINGSCONSOLE_API bool IsVariableRegistered(FName Name);

	/**
	 * Template functions for console variable operations
	 */

	/** Register a new console variable with default value */
	template <typename TValue>
	IConsoleVariable* RegisterVariable(FName Name, const TValue& DefaultValue, const FString& Help);

	/** Register a callback for console variable changes */
	template <typename TValue>
	void RegisterVariableChangedCallback(FName Name, TFunction<void(TValue)> ChangedCallback, bool bExecuteCallback);

	/** Get console variable current value */
	template <typename TValue>
	TValue GetValue(FName Name);

	/** Set console variable value */
	template <typename TValue>
	void SetValue(FName Name, const TValue& Value);

	/**
	 * Implementation details - template traits for variable types
	 * Provides type-specific value getters
	 */
	template <typename TValue>
	struct VariableTraits;

	template <>
	struct VariableTraits<bool>
	{
		static bool GetValue(const IConsoleVariable* CVar) { return CVar->GetBool(); }
	};

	template <>
	struct VariableTraits<int32>
	{
		static int32 GetValue(const IConsoleVariable* CVar) { return CVar->GetInt(); }
	};

	template <>
	struct VariableTraits<float>
	{
		static float GetValue(const IConsoleVariable* CVar) { return CVar->GetFloat(); }
	};

	template <>
	struct VariableTraits<FString>
	{
		static FString GetValue(const IConsoleVariable* CVar) { return CVar->GetString(); }
	};
} // namespace SB::AutoSettings::Console

// Template implementations
template <typename TValue>
IConsoleVariable* SB::AutoSettings::Console::RegisterVariable(FName Name, const TValue& DefaultValue, const FString& Help)
{
	return IConsoleManager::Get().RegisterConsoleVariable(*Name.ToString(), DefaultValue, *Help);
}

template <typename TValue>
void SB::AutoSettings::Console::RegisterVariableChangedCallback(FName Name, TFunction<void(TValue)> ChangedCallback, bool bExecuteCallback)
{
	if (!IsVariableRegistered(Name))
	{
		UE_LOGFMT(LogAutoSettings, Error, "Cannot add callback for console variable '{0}' - variable is not registered", *Name.ToString());
		return;
	}

	auto ConsoleVariable = GetVariable(Name);
	if (!ConsoleVariable)
	{
		return;
	}

	// Use a raw lambda since TFunction doesn't have a UObject for weak references
	// The console variable system manages the lifetime of these callbacks
	ConsoleVariable->OnChangedDelegate().AddLambda(
		[ChangedCallback](const IConsoleVariable* ConsoleVariable)
		{
			ChangedCallback(VariableTraits<TValue>::GetValue(ConsoleVariable));
		});

	if (bExecuteCallback)
	{
		ChangedCallback(VariableTraits<TValue>::GetValue(ConsoleVariable));
	}
}

template <typename TValue>
TValue SB::AutoSettings::Console::GetValue(FName Name)
{
	if (!IsVariableRegistered(Name))
	{
		return TValue{};
	}

	IConsoleVariable* CVar = GetVariable(Name);
	if (!CVar)
	{
		return TValue{};
	}

	return VariableTraits<TValue>::GetValue(CVar);
}

template <typename TValue>
void SB::AutoSettings::Console::SetValue(FName Name, const TValue& Value)
{
	if (!IsVariableRegistered(Name))
	{
		return;
	}

	IConsoleVariable* CVar = GetVariable(Name);
	if (!CVar)
	{
		return;
	}

	// Handle string case specially - SetWithCurrentPriority expects const TCHAR* for strings
	if constexpr (std::is_same_v<TValue, FString>)
	{
		CVar->SetWithCurrentPriority(*Value);
	}
	else
	{
		CVar->SetWithCurrentPriority(Value);
	}

	IConsoleManager::Get().CallAllConsoleVariableSinks();
}