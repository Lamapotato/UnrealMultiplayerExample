// Copyright Sam Bonifacio. All Rights Reserved.

#include "PIECommands.h"

#include "Editor.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Tests/AutomationEditorCommon.h"

namespace SB::Editor::Tests::PIE
{
	UGameInstance* GetGameInstance()
	{
		UWorld* PlayWorld = GEditor ? GEditor->PlayWorld : nullptr;
		return PlayWorld ? PlayWorld->GetGameInstance() : nullptr;
	}

	TFuture<void> StartAsync()
	{
		const TSharedRef<TPromise<void>> Promise = MakeShared<TPromise<void>>();
		const TSharedRef<FDelegateHandle> PostPIEStartedHandle = MakeShared<FDelegateHandle>();
		TFuture<void> Future = Promise->GetFuture();

		*PostPIEStartedHandle = FEditorDelegates::PostPIEStarted.AddLambda([Promise, PostPIEStartedHandle](bool)
			{
				// Local references survive removing the delegate that owns this callback.
				const TSharedRef<TPromise<void>> PromiseToComplete = Promise;
				const TSharedRef<FDelegateHandle> HandleToRemove = PostPIEStartedHandle;
				PromiseToComplete->SetValue();
				FEditorDelegates::PostPIEStarted.Remove(*HandleToRemove);
			});

		FStartPIECommand(false).Update();
		return Future;
	}

	TFuture<void> StopAsync()
	{
		const TSharedRef<TPromise<void>> Promise = MakeShared<TPromise<void>>();
		TFuture<void> Future = Promise->GetFuture();
		if (!GEditor || !GEditor->PlayWorld)
		{
			Promise->SetValue();
			return Future;
		}

		const TSharedRef<FDelegateHandle> EndPIEHandle = MakeShared<FDelegateHandle>();
		*EndPIEHandle = FEditorDelegates::EndPIE.AddLambda([Promise, EndPIEHandle](bool)
			{
				// Local references survive removing the delegate that owns this callback.
				const TSharedRef<TPromise<void>> PromiseToComplete = Promise;
				const TSharedRef<FDelegateHandle> HandleToRemove = EndPIEHandle;
				PromiseToComplete->SetValue();
				FEditorDelegates::EndPIE.Remove(*HandleToRemove);
			});

		FEndPlayMapCommand().Update();
		return Future;
	}
}
