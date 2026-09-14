// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/SettingType.h"

#include "SettingTestType.generated.h"

/**
 * Minimal concrete shell for manually configured setting objects in tests.
 */
UCLASS(HideDropdown, NotBlueprintable)
class AUTOSETTINGSTESTS_API UTestSettingType : public USettingType
{
	GENERATED_BODY()

public:
	UTestSettingType(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{
		// Loaded-native discovery must ignore the generic empty-key CDO.
		bEnabled = false;
	}
};
