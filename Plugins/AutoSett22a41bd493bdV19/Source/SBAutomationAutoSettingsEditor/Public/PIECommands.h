// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "Async/Future.h"

class UGameInstance;

namespace SB::Editor::Tests::PIE
{
	/** Returns the game instance owned by the active PIE world. */
	SBAUTOMATIONAUTOSETTINGSEDITOR_API UGameInstance* GetGameInstance();

	/** Starts PIE and completes when PostPIEStarted is published. */
	SBAUTOMATIONAUTOSETTINGSEDITOR_API TFuture<void> StartAsync();

	/** Requests PIE shutdown and completes when EndPIE is published. */
	SBAUTOMATIONAUTOSETTINGSEDITOR_API TFuture<void> StopAsync();
}
