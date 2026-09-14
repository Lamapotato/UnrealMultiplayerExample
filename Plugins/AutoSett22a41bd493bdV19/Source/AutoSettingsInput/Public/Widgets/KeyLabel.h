// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "Display/KeyIconSource.h"
#include "KeyLabel.generated.h"

/**
 * Shows the icon for a specific key or falls back to text label
 */
UCLASS(abstract)
class AUTOSETTINGSINPUT_API UKeyLabel : public UUserWidget
{
	GENERATED_BODY()

public:
	UKeyLabel(const FObjectInitializer& ObjectInitializer);

	// Text to display when key is invalid or unbound
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Key Label")
	FText KeyInvalidText;

	// If not empty, text to display instead of key label
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Key Label")
	FText LabelOverride;

	// Key that this label is for
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Key Label")
	FKey Key;

	// Implement this to respond to data changes
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Key Label")
	void UpdateKeyLabel();

	UFUNCTION(BlueprintPure, Category = "Key Label")
	FKey GetKey() const
	{
		return Key;
	}

	UFUNCTION(BlueprintCallable, Category = "Key Label")
	void SetKey(const FKey& InKey);

protected:
	// Label text to display for the key
	UFUNCTION(BlueprintPure, Category = "Key Label")
	FText GetDisplayName() const;

	// True if the key has an icon defined
	UFUNCTION(BlueprintPure, Category = "Key Label")
	bool HasIcon() const;

	// True if the key is valid
	UFUNCTION(BlueprintPure, Category = "Key Label")
	bool HasValidKey() const;

	// Return to provide an icon brush for the key
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Key Label")
	FSlateBrush GetIconBrush() const;

	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Key Label")
	ESlateVisibility GetIconVisibility() const;

	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Key Label")
	ESlateVisibility GetDisplayNameVisibility() const;

	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnKeyIconsChanged();
};