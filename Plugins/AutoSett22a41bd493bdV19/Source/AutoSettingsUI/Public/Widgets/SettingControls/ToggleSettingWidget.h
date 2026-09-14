// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "Widgets/AutoSettingWidget.h"
#include "ToggleSettingWidget.generated.h"

/**
 * Base AutoSetting with generic toggle functionality (Such as a checkbox or switch)
 */
UCLASS(abstract)
class AUTOSETTINGSUI_API UToggleSettingWidget : public UAutoSettingWidget
{
	GENERATED_BODY()

protected:
	virtual void UpdateSelection_Implementation(const FString& Value) override;

	// Called to update the widget with a new state
	UFUNCTION(BlueprintNativeEvent, Category = "Setting|Toggle")
	void UpdateToggleState(bool State);

	// Takes a boolean state and applies the setting
	UFUNCTION(BlueprintCallable, Category = "Setting|Toggle")
	void ToggleStateUpdated(bool State);
};