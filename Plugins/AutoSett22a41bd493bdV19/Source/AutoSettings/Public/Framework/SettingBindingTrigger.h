// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/SettingTypes.h"
#include "SettingBindingTrigger.generated.h"

class USettingBinding;

/**
 * Reusable trigger template for runtime binding events. Strategy-owned trigger templates are
 * duplicated per active binding so cloned trigger instances can safely store runtime state.
 */
UCLASS(Abstract, EditInlineNew, Blueprintable)
class AUTOSETTINGS_API USettingBindingTrigger : public UObject
{
	GENERATED_BODY()

public:
	/** Activate this trigger for a specific live binding instance. */
	UFUNCTION(BlueprintNativeEvent)
	void Activate(const FSettingReference& Setting, USettingBinding* Binding);

	/** Deactivate this trigger for a specific live binding instance. */
	UFUNCTION(BlueprintNativeEvent)
	void Deactivate(const FSettingReference& Setting, USettingBinding* Binding);
};
