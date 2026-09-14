// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AutoSettingsConsoleVariable.h"
#include "Framework/SettingPersistenceStrategy.h"
#include "Framework/SettingType.h"

#include "ConsoleHandlerInitializationEditorTests.generated.h"

UCLASS(HideDropdown, NotBlueprintable)
class AUTOSETTINGSTESTS_API UTestConsoleHandlerInitializationVariable : public UAutoSettingsConsoleVariable
{
	GENERATED_BODY()

public:
	UTestConsoleHandlerInitializationVariable();

protected:
	virtual IConsoleVariable* RegisterConsoleVariable() override;
	virtual void RegisterChangedCallback() override;
};

/**
 * Supplies a test-owned saved value without reading or writing process-wide config.
 */
UCLASS(HideDropdown, NotBlueprintable)
class AUTOSETTINGSTESTS_API UTestConsoleHandlerInitializationPersistenceStrategy : public USettingPersistenceStrategy
{
	GENERATED_BODY()

public:
	/** Empty data represents a setting with no saved value. */
	UPROPERTY(Transient)
	FSettingData SavedData;

	virtual FSettingData GetSavedValue_Implementation(const FSettingReference& Setting) const override;
};

UCLASS(HideDropdown, NotBlueprintable)
class AUTOSETTINGSTESTS_API UTestConsoleHandlerInitializationSettingType : public USettingType
{
	GENERATED_BODY()

public:
	UTestConsoleHandlerInitializationSettingType(const FObjectInitializer& ObjectInitializer);
};
