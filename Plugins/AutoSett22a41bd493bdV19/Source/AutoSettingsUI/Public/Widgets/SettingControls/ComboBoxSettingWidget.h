// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "SelectSettingWidget.h"
#include "Components/ComboBoxString.h"
#include "ComboBoxSettingWidget.generated.h"

/**
 * AutoSetting for a native Unreal ComboBox
 */
UCLASS(abstract)
class AUTOSETTINGSUI_API UComboBoxSettingWidget : public USelectSettingWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Setting|Select", meta = (BindWidget))
	UComboBoxString* ComboBox;

	virtual void NativeConstruct() override;

	virtual void UpdateSelection_Implementation(const FString& Value) override;

	virtual void UpdateOptions_Implementation(const TArray<FSettingOption>& InOptions) override;

private:
	UFUNCTION()
	void ComboBoxSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
};