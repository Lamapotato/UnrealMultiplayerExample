// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "ToggleSettingWidget.h"
#include "Components/CheckBox.h"
#include "CheckBoxSettingWidget.generated.h"

/**
 * AutoSetting for a native Unreal CheckBox
 */
UCLASS(abstract)
class AUTOSETTINGSUI_API UCheckBoxSettingWidget : public UToggleSettingWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Setting|Toggle", meta = (BindWidget))
	UCheckBox* CheckBox;

	virtual void NativeConstruct() override;

	virtual void UpdateToggleState_Implementation(bool State) override;

private:
	UFUNCTION()
	void CheckBoxStateChanged(bool IsChecked);
};