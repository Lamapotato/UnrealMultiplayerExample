// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "HAL/IConsoleManager.h"
#include "ConsoleFunctionLibrary.generated.h"

// Dynamic delegate declarations for Blueprint interoperability
DECLARE_DYNAMIC_DELEGATE_OneParam(FIntConsoleVariableChangedSignature, int32, NewValue);
DECLARE_DYNAMIC_DELEGATE_OneParam(FBoolConsoleVariableChangedSignature, bool, NewValue);
DECLARE_DYNAMIC_DELEGATE_OneParam(FFloatConsoleVariableChangedSignature, float, NewValue);
DECLARE_DYNAMIC_DELEGATE_OneParam(FStringConsoleVariableChangedSignature, const FString&, NewValue);

/**
 * Blueprint function library for console variable operations
 * Provides UFUNCTION wrappers around ConsoleUtilities for Blueprint access
 */
UCLASS()
class AUTOSETTINGSCONSOLE_API UConsoleFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * General helper functions
	 */

	// True if the CVar with the specified name is registered
	UFUNCTION(BlueprintPure, Category = "Console", meta = (Keywords = "cvar"))
	static bool IsConsoleVariableRegistered(FName Name);

	/**
	 * Register functions
	 */

	// Register a new integer CVar
	UFUNCTION(BlueprintCallable, Category = "Console", meta = (Keywords = "cvar"))
	static void RegisterIntConsoleVariable(FName Name, int32 DefaultValue, const FString& Help, FIntConsoleVariableChangedSignature ChangedCallback, bool bExecuteCallback);

	// Register a new boolean CVar
	UFUNCTION(BlueprintCallable, Category = "Console", meta = (Keywords = "cvar"))
	static void RegisterBoolConsoleVariable(FName Name, bool DefaultValue, const FString& Help, FBoolConsoleVariableChangedSignature ChangedCallback, bool bExecuteCallback);

	// Register a new float CVar
	UFUNCTION(BlueprintCallable, Category = "Console", meta = (Keywords = "cvar"))
	static void RegisterFloatConsoleVariable(FName Name, float DefaultValue, const FString& Help, FFloatConsoleVariableChangedSignature ChangedCallback, bool bExecuteCallback);

	// Register a new string CVar
	UFUNCTION(BlueprintCallable, Category = "Console", meta = (Keywords = "cvar"))
	static void RegisterStringConsoleVariable(FName Name, const FString& DefaultValue, const FString& Help, FStringConsoleVariableChangedSignature ChangedCallback, bool bExecuteCallback);

	/**
	 * Register console variable changed callbacks
	 */

	// Register a callback function for an integer CVar
	// @param bExecuteCallback If true, will immediately fire the callback with current value
	UFUNCTION(BlueprintCallable, Category = "Console", meta = (Keywords = "cvar"))
	static void RegisterIntConsoleVariableChangedCallback(FName Name, FIntConsoleVariableChangedSignature ChangedCallback,
		bool bExecuteCallback);

	// Register a callback function for a bool CVar
	// @param bExecuteCallback If true, will immediately fire the callback with current value
	UFUNCTION(BlueprintCallable, Category = "Console", meta = (Keywords = "cvar"))
	static void RegisterBoolConsoleVariableChangedCallback(FName Name, FBoolConsoleVariableChangedSignature ChangedCallback,
		bool bExecuteCallback);

	// Register a callback function for a float CVar
	// @param bExecuteCallback If true, will immediately fire the callback with current value
	UFUNCTION(BlueprintCallable, Category = "Console", meta = (Keywords = "cvar"))
	static void RegisterFloatConsoleVariableChangedCallback(FName Name, FFloatConsoleVariableChangedSignature ChangedCallback,
		bool bExecuteCallback);

	// Register a callback function for a string CVar
	// @param bExecuteCallback If true, will immediately fire the callback with current value
	UFUNCTION(BlueprintCallable, Category = "Console", meta = (Keywords = "cvar"))
	static void RegisterStringConsoleVariableChangedCallback(FName Name, FStringConsoleVariableChangedSignature ChangedCallback,
		bool bExecuteCallback);

	/**
	 * Get console variable current value
	 */

	// Get current value of integer CVar
	UFUNCTION(BlueprintPure, Category = "Console", meta = (Keywords = "cvar"))
	static int32 GetIntConsoleVariable(FName Name);

	// Get current value of bool CVar
	UFUNCTION(BlueprintPure, Category = "Console", meta = (Keywords = "cvar"))
	static bool GetBoolConsoleVariable(FName Name);

	// Get current value of float CVar
	UFUNCTION(BlueprintPure, Category = "Console", meta = (Keywords = "cvar"))
	static float GetFloatConsoleVariable(FName Name);

	// Get current value of string CVar
	UFUNCTION(BlueprintPure, Category = "Console", meta = (Keywords = "cvar"))
	static FString GetStringConsoleVariable(FName Name);

	/**
	 * Set console variable value
	 */

	// Set value of integer CVar
	UFUNCTION(BlueprintCallable, Category = "Console", meta = (Keywords = "cvar"))
	static void SetIntConsoleVariable(FName Name, int32 Value);

	// Set value of bool CVar
	UFUNCTION(BlueprintCallable, Category = "Console", meta = (Keywords = "cvar"))
	static void SetBoolConsoleVariable(FName Name, bool Value);

	// Set value of float CVar
	UFUNCTION(BlueprintCallable, Category = "Console", meta = (Keywords = "cvar"))
	static void SetFloatConsoleVariable(FName Name, float Value);

	// Set value of string CVar
	UFUNCTION(BlueprintCallable, Category = "Console", meta = (Keywords = "cvar"))
	static void SetStringConsoleVariable(FName Name, FString Value);
};