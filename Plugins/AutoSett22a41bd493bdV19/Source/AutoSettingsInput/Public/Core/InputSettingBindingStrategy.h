// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/SettingBindingStrategy.h"
#include "UObject/Object.h"
#include "Utility/InputMappingUtils.h"
#include "InputSettingBindingStrategy.generated.h"

class ULocalPlayer;
struct FPlayerMappableKeySlot;

/**
 *
 */
UCLASS()
class AUTOSETTINGSINPUT_API UInputSettingBindingStrategy : public USettingBindingStrategy
{
	GENERATED_BODY()

public:
	virtual bool IsBindingReady_Implementation(const FSettingContext& Context) override;
	virtual void OnSyncToTarget_Implementation(const FSetSettingEvent& Event) override;
	virtual FString GetTargetValue_Implementation(const FSettingReference& Setting) override;
	virtual void BeginBinding_Implementation(const FSettingReference& Setting, USettingBinding* Binding) override;
	virtual void EndBinding_Implementation(const FSettingReference& Setting, USettingBinding* Binding) override;
	virtual bool ValidateConfiguration_Implementation(const FSettingReference& Setting) override;

private:
	bool IsInputMappingKey(const FSettingReference& Setting) const;
	/** Represents an input mapping value with an optional axis scale for directional inputs */
	struct FInputMappingValue
	{
		/** The input key (e.g., W, Mouse X, Gamepad Left Thumbstick X) */
		FKey Key = EKeys::Invalid;

		/** Scale factor for axis inputs (-1.0 for negative direction, 1.0 for positive), only set if explicitly specified */
		TOptional<float> AxisScale;

		FInputMappingValue() = default;

		/** Constructs from a string in format "KeyName" or "KeyName:Scale" */
		explicit FInputMappingValue(const FString& InputValue);

		/** Converts to string, including scale for axis inputs when available */
		FString ToString() const;
	};

	ULocalPlayer* GetLocalPlayer(const FSettingContext& Context) const;
	class UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem(const FSettingContext& Context) const;
	class UAutoSettingsEnhancedInputUserSettings* GetEnhancedInputUserSettings(const FSettingContext& Context) const;
	bool EnsureValidInputMappingKey(const FSettingReference& Setting) const;
	FName GetMappingName(const FSettingReference& Setting) const;
	EPlayerMappableKeySlot GetKeySlot(const FSettingReference& Setting) const;

	FInputMappingValue ParseInputValue(const FString& InputValue) const;
	void ApplyAxisMetadata(class UAutoSettingsEnhancedInputUserSettings* UserSettings, const FInputMappingValue& MappingValue, const FName& MappingName) const;
	bool EnsureMappingContextRegistered(class UAutoSettingsEnhancedInputUserSettings* UserSettings, const FName& MappingName, const FSettingContext& Context) const;
	void ApplyKeyMapping(class UAutoSettingsEnhancedInputUserSettings* UserSettings, const FKey& NewKey, const FName& MappingName, EPlayerMappableKeySlot KeySlot) const;

	FString GetDesignTimeDefaultKey(const FSettingReference& Setting) const;
	FString FormatAxisKeyWithScale(const FKey& AxisKey, const FSettingReference& Setting) const;
};
