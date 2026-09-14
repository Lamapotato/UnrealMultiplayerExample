// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "AutoSettingsConsoleEngineSubsystem.generated.h"

/**
 * Engine-level subsystem responsible for tracking native console variables created by UConsoleVariable classes
 */
UCLASS()
class AUTOSETTINGSCONSOLE_API UAutoSettingsConsoleEngineSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	bool IsRegistered(FName Name) const;
	void UnregisterOldConsoleVariables(TArray<FName> CVarNamesToKeep);
	void Add(IConsoleVariable* ConsoleVariable);
	void RegisterConsoleVariables();

	/**
	 * Register native console variables from console variable classes
	 * This only creates the native CVars, no UObjects or callbacks
	 * @return Registration results with success/failure details
	 */
	struct FConsoleVariableRegistrationResult
	{
		/** Console variables that were successfully registered */
		TArray<IConsoleVariable*> RegisteredCVars;

		/** Names that were skipped (already registered by us) */
		TArray<FName> SkippedNames;

		/** Names that failed registration (conflicts with other systems) */
		TArray<FName> FailedNames;
	};

	FConsoleVariableRegistrationResult RegisterNativeConsoleVariables();

	/**
	 * Register or update a single console variable
	 * @param ConsoleVariableClass The console variable class to register
	 * @return true if the console variable was registered or updated, false if skipped
	 */
	bool RegisterConsoleVariable(TSubclassOf<class UAutoSettingsConsoleVariable> ConsoleVariableClass);

	/**
	 * Stop tracking a console-variable class and fully unregister its native CVar when no other class uses that name.
	 * @param ConsoleVariableClass The console-variable class to stop tracking.
	 * @return true if the class was tracked and removed.
	 */
	bool UnregisterConsoleVariable(TSubclassOf<class UAutoSettingsConsoleVariable> ConsoleVariableClass);

#if WITH_EDITOR
	void OnAssetChanged(const FAssetData& AssetData);
	void OnAssetRemoved(const FAssetData& AssetData);
#endif

private:
	/**
	 * Unregister a console variable by name
	 * @param CVarName The name of the console variable to unregister
	 * @return true if the console variable was found and unregistered
	 */
	bool UnregisterConsoleVariableByName(FName CVarName);

	/**
	 * Get the name of a console variable
	 * @param ConsoleVariable The console variable to get the name of
	 * @return The name of the console variable, or NAME_None if invalid
	 */
	static FName GetConsoleVariableName(IConsoleVariable* ConsoleVariable);

	/**
	 * Remove a class from tracking for a specific console variable name
	 * Unregisters the console variable if no other classes are using it
	 * @param ConsoleVariableClass The class to remove (soft reference)
	 * @param CVarName The console variable name
	 */
	void RemoveClassFromCVarTracking(TSoftClassPtr<class UAutoSettingsConsoleVariable> ConsoleVariableClass, FName CVarName);

	// Native console variables that were registered by console variable uobjects under this subsystem
	// This have to be tracked outside of the console variable uobjects so that they persist when the console variable class is recompiled
	// These are tracked at the engine level so that they are not doubled up in the event of multiple GameInstances (play in editor)
	// i.e Because of PIE, many instances of the same UConsoleVariable class can point to a single IConsoleVariable object here
	TArray<IConsoleVariable*> ConsoleVariables;

	// Maps console variable name to the set of soft class references using it
	// Multiple classes can temporarily use the same CVar name (e.g., during duplication)
	// Using soft references allows proper tracking even when assets are unloaded/removed
	TMap<FName, TSet<TSoftClassPtr<class UAutoSettingsConsoleVariable>>> ConsoleVariableNameToClasses;

	// Maps soft class reference to its currently registered name
	// Used to detect when a console variable's name has changed
	// Using soft references allows tracking even when assets are unloaded/removed
	TMap<TSoftClassPtr<class UAutoSettingsConsoleVariable>, FName> ConsoleVariableClassToName;
};