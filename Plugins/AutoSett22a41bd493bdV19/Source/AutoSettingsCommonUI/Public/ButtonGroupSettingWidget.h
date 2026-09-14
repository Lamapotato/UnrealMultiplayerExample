// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SettingControls/SelectSettingWidget.h"

#include "ButtonGroupSettingWidget.generated.h"

class UCommonButtonGroupBase;

/**
 * Setting widget that uses the CommonUI ButtonGroup to provide a collection of buttons for setting options
 */
UCLASS(abstract)
class AUTOSETTINGSCOMMONUI_API UButtonGroupSettingWidget : public USelectSettingWidget
{
	GENERATED_BODY()

public:
	// Button class to create for each option
	UPROPERTY(EditAnywhere, BlueprintReadWrite, NoClear, Category = "Setting|Radio Select")
	TSubclassOf<UCommonButtonBase> ButtonClass;

	UFUNCTION(BlueprintPure, Category = "Setting|Radio Select")
	UCommonButtonGroupBase* GetButtonGroup() const
	{
		return ButtonGroup;
	}

protected:
	virtual void NativePreConstruct() override;

	virtual void UpdateOptions_Implementation(const TArray<FSettingOption>& InOptions) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Setting|Radio Select")
	void HandleButtonCreation(UCommonButtonBase* NewButton, const FSettingOption& Option);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Setting|Radio Select")
	void HandleButtonRemoval(UCommonButtonBase* NewButton);

	virtual void UpdateSelection_Implementation(const FString& Value) override;

	virtual void NativeDestruct() override;

private:
	UPROPERTY()
	TObjectPtr<UCommonButtonGroupBase> ButtonGroup;

	UPROPERTY()
	TMap<UCommonButtonBase*, FString> ButtonValueMap;

	void RemoveAllButtons();
};