// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "Async/Future.h"
#include "Commands/TestCommandBuilder.h"

namespace SB::Editor::Tests
{
	/** Starts one future-producing operation at its command position and waits for completion. */
	inline void Await(FTestCommandBuilder& CommandBuilder, const TCHAR* Description,
		TFunction<TFuture<void>()> StartAsync, const FTimespan Timeout = FTimespan::FromSeconds(10))
	{
		const TSharedRef<TOptional<TSharedFuture<void>>> Future = MakeShared<TOptional<TSharedFuture<void>>>();
		CommandBuilder
			.Do([Future, StartAsync = MoveTemp(StartAsync)]()
				{
					Future->Emplace(StartAsync());
				})
			.Until(Description, [Future]()
				{
					return Future->IsSet() && Future->GetValue().IsReady();
				}, Timeout);
	}
}
