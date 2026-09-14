// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Expose automation helper APIs only in targets that compile development automation support.
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
namespace SBAutomationAutoSettingsCommon
{
	SBAUTOMATIONAUTOSETTINGS_API UWorld* GetAnyGameWorld();

	SBAUTOMATIONAUTOSETTINGS_API bool OpenMap(FAutomationTestBase* Test, const FString& MapPath);
}

// Call the provided function as an automation command
DEFINE_EXPORTED_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(SBAUTOMATIONAUTOSETTINGS_API, FDelegateCommand, TFunction<void()>, Function);

// Does nothing except wait a tick
DEFINE_EXPORTED_LATENT_AUTOMATION_COMMAND(SBAUTOMATIONAUTOSETTINGS_API, FWaitTickCommand);

#endif
