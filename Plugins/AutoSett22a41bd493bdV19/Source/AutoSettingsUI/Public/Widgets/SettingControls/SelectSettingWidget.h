// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "Widgets/AutoSettingWidget.h"
#include "Misc/SettingOptionSource.h"
#include "SelectSettingWidget.generated.h"

/**
 * Base class for any AutoSetting containing multiple pre-defined options
 */
UCLASS(abstract)
class AUTOSETTINGSUI_API USelectSettingWidget : public UAutoSettingWidget
{
	GENERATED_BODY()

public:
	// Statically defined options
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Select")
	TArray<FSettingOption> Options;

	// Object to dynamically provide available options
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Select")
	USettingOptionSource* OptionSource;

	/**
	 * Call to trigger re-evaluation of Option Factory and reconstruction of available options
	 * Does not modify the selection even if the selected option is removed
	 */
	UFUNCTION(BlueprintCallable, Category = "Setting|Select")
	void RegenerateOptions();

protected:
	virtual void NativePreConstruct() override;

	UFUNCTION(BlueprintNativeEvent, Category = "Setting|Select")
	void UpdateOptions(const TArray<FSettingOption>& InOptions);

	bool HasOptionWithValue(FString Value) const;

	bool HasOptionWithLabel(FText Label) const;

	FSettingOption GetOptionByValue(FString Value) const;

	FSettingOption GetOptionByLabel(FText Label) const;

	bool IsUpdatingSettingOptions() const
	{
		return bUpdatingSettingOptions;
	}

	virtual void PostLoad() override;

	virtual void PostInitProperties() override;

private:
	UPROPERTY()
	bool bUpdatingSettingOptions;

	UPROPERTY()
	TSubclassOf<USettingOptionSource> OptionFactory_DEPRECATED;

	TArray<FSettingOption> GetCombinedOptions() const;

	void DispatchUpdateOptions(const TArray<FSettingOption>& InOptions);
};