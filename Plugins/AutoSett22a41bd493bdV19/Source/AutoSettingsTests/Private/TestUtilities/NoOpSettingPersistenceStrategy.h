// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/SettingPersistenceStrategy.h"

#include "NoOpSettingPersistenceStrategy.generated.h"

/**
 * Keeps registry tests independent from generated config files and other persistent state.
 */
UCLASS(HideDropdown, NotBlueprintable)
class AUTOSETTINGSTESTS_API UTestNoOpSettingPersistenceStrategy : public USettingPersistenceStrategy
{
	GENERATED_BODY()

public:
	virtual FSettingData GetSavedValue_Implementation(const FSettingReference& Setting) const override
	{
		// Tests start from live target values, so no persisted value participates.
		return {};
	}

	virtual TArray<FFullSettingKey> FindKeys_Implementation() const override
	{
		// Tests declare their keys directly, so persistence discovers none.
		return {};
	}

	virtual void OnSave_Implementation(const FSetSettingEvent& Event) override
	{
		// Saving intentionally leaves persistent state untouched.
	}
};
