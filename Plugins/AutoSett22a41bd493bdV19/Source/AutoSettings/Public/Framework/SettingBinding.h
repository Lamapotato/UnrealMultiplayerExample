// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/SettingTypes.h"
#include "SettingBinding.generated.h"

class USettingBindingStrategy;
class USettingBindingTrigger;
class USettingRegistry;

/**
 * Active runtime binding instance for a setting.
 *
 * The registry owns these objects and routes strategy/trigger notifications through them so both
 * native and Blueprint bindings share the same lifecycle and event model.
 */
UCLASS(BlueprintType)
class AUTOSETTINGS_API USettingBinding : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(USettingRegistry* InRegistry, const FSettingReference& InSetting, USettingBindingStrategy* InStrategy,
		const TArray<TObjectPtr<USettingBindingTrigger>>& TriggerTemplates);

	void Begin();
	void End();

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void NotifyTargetValueChanged();

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void NotifyBindingInvalidated();

	UFUNCTION(BlueprintPure, Category = "Settings")
	FSettingReference GetSetting() const
	{
		return Setting;
	}

	UFUNCTION(BlueprintPure, Category = "Settings")
	USettingRegistry* GetRegistry() const
	{
		return Registry;
	}

	UFUNCTION(BlueprintPure, Category = "Settings")
	bool IsActive() const
	{
		return bIsActive;
	}

	const TArray<TObjectPtr<USettingBindingTrigger>>& GetTriggers() const
	{
		return Triggers;
	}

private:
	UPROPERTY(Transient)
	TObjectPtr<USettingRegistry> Registry;

	UPROPERTY(Transient)
	TObjectPtr<USettingBindingStrategy> Strategy;

	UPROPERTY(BlueprintReadOnly, Category = "Settings", meta = (AllowPrivateAccess = "true"))
	FSettingReference Setting;

	UPROPERTY(Instanced, Transient)
	TArray<TObjectPtr<USettingBindingTrigger>> Triggers;

	UPROPERTY(Transient)
	bool bIsActive = false;
};
