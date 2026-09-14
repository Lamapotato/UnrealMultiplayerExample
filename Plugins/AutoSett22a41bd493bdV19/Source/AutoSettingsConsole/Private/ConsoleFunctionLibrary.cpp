// Copyright Sam Bonifacio. All Rights Reserved.

#include "ConsoleFunctionLibrary.h"
#include "ConsoleUtilities.h"
#include "AutoSettingsLogs.h"
#include "Logging/StructuredLog.h"

bool UConsoleFunctionLibrary::IsConsoleVariableRegistered(FName Name)
{
	return SB::AutoSettings::Console::IsVariableRegistered(Name);
}

void UConsoleFunctionLibrary::RegisterIntConsoleVariable(FName Name, int32 DefaultValue, const FString& Help, FIntConsoleVariableChangedSignature ChangedCallback, bool bExecuteCallback)
{
	SB::AutoSettings::Console::RegisterVariable<int32>(Name, DefaultValue, Help);
	if (ChangedCallback.IsBound())
	{
		RegisterIntConsoleVariableChangedCallback(Name, ChangedCallback, bExecuteCallback);
	}
}

void UConsoleFunctionLibrary::RegisterBoolConsoleVariable(FName Name, bool DefaultValue, const FString& Help, FBoolConsoleVariableChangedSignature ChangedCallback, bool bExecuteCallback)
{
	SB::AutoSettings::Console::RegisterVariable<bool>(Name, DefaultValue, Help);
	if (ChangedCallback.IsBound())
	{
		RegisterBoolConsoleVariableChangedCallback(Name, ChangedCallback, bExecuteCallback);
	}
}

void UConsoleFunctionLibrary::RegisterFloatConsoleVariable(FName Name, float DefaultValue, const FString& Help, FFloatConsoleVariableChangedSignature ChangedCallback, bool bExecuteCallback)
{
	SB::AutoSettings::Console::RegisterVariable<float>(Name, DefaultValue, Help);
	if (ChangedCallback.IsBound())
	{
		RegisterFloatConsoleVariableChangedCallback(Name, ChangedCallback, bExecuteCallback);
	}
}

void UConsoleFunctionLibrary::RegisterStringConsoleVariable(FName Name, const FString& DefaultValue, const FString& Help, FStringConsoleVariableChangedSignature ChangedCallback, bool bExecuteCallback)
{
	SB::AutoSettings::Console::RegisterVariable<FString>(Name, DefaultValue, Help);
	if (ChangedCallback.IsBound())
	{
		RegisterStringConsoleVariableChangedCallback(Name, ChangedCallback, bExecuteCallback);
	}
}

void UConsoleFunctionLibrary::RegisterIntConsoleVariableChangedCallback(FName Name, FIntConsoleVariableChangedSignature ChangedCallback, bool bExecuteCallback)
{
	if (!SB::AutoSettings::Console::IsVariableRegistered(Name))
	{
		UE_LOGFMT(LogAutoSettings, Warning, "Console variable '{0}' is not registered, callback registration may fail", *Name.ToString());
	}

	// Convert dynamic delegate to TFunction for C++ layer
	TFunction<void(int32)> CppCallback = [ChangedCallback](int32 NewValue)
	{
		ChangedCallback.ExecuteIfBound(NewValue);
	};

	SB::AutoSettings::Console::RegisterVariableChangedCallback<int32>(Name, CppCallback, bExecuteCallback);
}

void UConsoleFunctionLibrary::RegisterBoolConsoleVariableChangedCallback(FName Name, FBoolConsoleVariableChangedSignature ChangedCallback, bool bExecuteCallback)
{
	if (!SB::AutoSettings::Console::IsVariableRegistered(Name))
	{
		UE_LOGFMT(LogAutoSettings, Warning, "Console variable '{0}' is not registered, callback registration may fail", *Name.ToString());
	}

	// Convert dynamic delegate to TFunction for C++ layer
	TFunction<void(bool)> CppCallback = [ChangedCallback](bool NewValue)
	{
		ChangedCallback.ExecuteIfBound(NewValue);
	};

	SB::AutoSettings::Console::RegisterVariableChangedCallback<bool>(Name, CppCallback, bExecuteCallback);
}

void UConsoleFunctionLibrary::RegisterFloatConsoleVariableChangedCallback(FName Name, FFloatConsoleVariableChangedSignature ChangedCallback, bool bExecuteCallback)
{
	if (!SB::AutoSettings::Console::IsVariableRegistered(Name))
	{
		UE_LOGFMT(LogAutoSettings, Warning, "Console variable '{0}' is not registered, callback registration may fail", *Name.ToString());
	}

	// Convert dynamic delegate to TFunction for C++ layer
	TFunction<void(float)> CppCallback = [ChangedCallback](float NewValue)
	{
		ChangedCallback.ExecuteIfBound(NewValue);
	};

	SB::AutoSettings::Console::RegisterVariableChangedCallback<float>(Name, CppCallback, bExecuteCallback);
}

void UConsoleFunctionLibrary::RegisterStringConsoleVariableChangedCallback(FName Name, FStringConsoleVariableChangedSignature ChangedCallback, bool bExecuteCallback)
{
	if (!SB::AutoSettings::Console::IsVariableRegistered(Name))
	{
		UE_LOGFMT(LogAutoSettings, Warning, "Console variable '{0}' is not registered, callback registration may fail", *Name.ToString());
	}

	// Convert dynamic delegate to TFunction for C++ layer
	TFunction<void(FString)> CppCallback = [ChangedCallback](const FString& NewValue)
	{
		ChangedCallback.ExecuteIfBound(NewValue);
	};

	SB::AutoSettings::Console::RegisterVariableChangedCallback<FString>(Name, CppCallback, bExecuteCallback);
}

int32 UConsoleFunctionLibrary::GetIntConsoleVariable(FName Name)
{
	if (!SB::AutoSettings::Console::IsVariableRegistered(Name))
	{
		UE_LOGFMT(LogAutoSettings, Warning, "Console variable '{0}' is not registered", *Name.ToString());
		return 0;
	}
	return SB::AutoSettings::Console::GetValue<int32>(Name);
}

bool UConsoleFunctionLibrary::GetBoolConsoleVariable(FName Name)
{
	if (!SB::AutoSettings::Console::IsVariableRegistered(Name))
	{
		UE_LOGFMT(LogAutoSettings, Warning, "Console variable '{0}' is not registered", *Name.ToString());
		return false;
	}
	return SB::AutoSettings::Console::GetValue<bool>(Name);
}

float UConsoleFunctionLibrary::GetFloatConsoleVariable(FName Name)
{
	if (!SB::AutoSettings::Console::IsVariableRegistered(Name))
	{
		UE_LOGFMT(LogAutoSettings, Warning, "Console variable '{0}' is not registered", *Name.ToString());
		return 0.0f;
	}
	return SB::AutoSettings::Console::GetValue<float>(Name);
}

FString UConsoleFunctionLibrary::GetStringConsoleVariable(FName Name)
{
	if (!SB::AutoSettings::Console::IsVariableRegistered(Name))
	{
		UE_LOGFMT(LogAutoSettings, Warning, "Console variable '{0}' is not registered", *Name.ToString());
		return FString();
	}
	return SB::AutoSettings::Console::GetValue<FString>(Name);
}

void UConsoleFunctionLibrary::SetIntConsoleVariable(FName Name, int32 Value)
{
	if (!SB::AutoSettings::Console::IsVariableRegistered(Name))
	{
		UE_LOGFMT(LogAutoSettings, Warning, "Console variable '{0}' is not registered", *Name.ToString());
		return;
	}
	SB::AutoSettings::Console::SetValue<int32>(Name, Value);
}

void UConsoleFunctionLibrary::SetBoolConsoleVariable(FName Name, bool Value)
{
	if (!SB::AutoSettings::Console::IsVariableRegistered(Name))
	{
		UE_LOGFMT(LogAutoSettings, Warning, "Console variable '{0}' is not registered", *Name.ToString());
		return;
	}
	SB::AutoSettings::Console::SetValue<bool>(Name, Value);
}

void UConsoleFunctionLibrary::SetFloatConsoleVariable(FName Name, float Value)
{
	if (!SB::AutoSettings::Console::IsVariableRegistered(Name))
	{
		UE_LOGFMT(LogAutoSettings, Warning, "Console variable '{0}' is not registered", *Name.ToString());
		return;
	}
	SB::AutoSettings::Console::SetValue<float>(Name, Value);
}

void UConsoleFunctionLibrary::SetStringConsoleVariable(FName Name, FString Value)
{
	if (!SB::AutoSettings::Console::IsVariableRegistered(Name))
	{
		UE_LOGFMT(LogAutoSettings, Warning, "Console variable '{0}' is not registered", *Name.ToString());
		return;
	}
	SB::AutoSettings::Console::SetValue<FString>(Name, Value);
}