// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "SelectSettingWidget.h"
#include "Widgets/GenericControls/Spinner.h"
#include "SpinnerSettingWidget.generated.h"

/**
 * AutoSetting for a Spinner
 */
UCLASS(abstract)
class AUTOSETTINGSUI_API USpinnerSettingWidget : public USelectSettingWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Setting|Spinner", meta = (BindWidget))
	USpinner* Spinner;

	virtual void NativeConstruct() override;

	virtual void UpdateSelection_Implementation(const FString& Value) override;

	virtual void UpdateOptions_Implementation(const TArray<FSettingOption>& InOptions) override;

private:
	UFUNCTION()
	void SpinnerSelectionChanged(FString Value);
};