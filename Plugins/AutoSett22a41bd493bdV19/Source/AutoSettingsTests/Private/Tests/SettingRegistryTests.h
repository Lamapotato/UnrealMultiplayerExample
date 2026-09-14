// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/SettingBindingStrategy.h"

#include "SettingRegistryTests.generated.h"

/**
 * Binding strategy that records registry-created binding lifetimes without adding external triggers.
 */
UCLASS(HideDropdown, NotBlueprintable)
class AUTOSETTINGSTESTS_API UTestRegistryLifecycleBindingStrategy : public USettingBindingStrategy
{
	GENERATED_BODY()

public:
	/** Count each binding start so the test can detect binds after registry shutdown. */
	UPROPERTY(Transient)
	int32 BeginBindingCount = 0;

	virtual FString GetTargetValue_Implementation(const FSettingReference& Setting) override;
	virtual void BeginBinding_Implementation(const FSettingReference& Setting, USettingBinding* Binding) override;
};
