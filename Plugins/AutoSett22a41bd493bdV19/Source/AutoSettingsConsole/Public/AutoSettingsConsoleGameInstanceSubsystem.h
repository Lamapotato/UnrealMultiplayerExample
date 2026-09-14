// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AutoSettingsConsoleGameInstanceSubsystem.generated.h"

class UAutoSettingsConsoleVariable;

/**
 * Subsystem responsible for handling Blueprint-exposed Console Variable classes at a GameInstance level
 */
UCLASS()
class AUTOSETTINGSCONSOLE_API UAutoSettingsConsoleGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Creates and registers this GameInstance's console-variable handler objects exactly once. */
	void ActivateHandlers();

private:
	/** Owns the activated callback objects for the GameInstance lifetime. */
	UPROPERTY()
	TArray<UAutoSettingsConsoleVariable*> ConsoleVariableObjects;

	/** Prevents duplicate handler discovery and callback registration. */
	UPROPERTY()
	bool bHandlersActivated = false;
};