// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/SettingBindingTrigger.h"
#include "SettingTypes.h"
#include "SettingBindingStrategy.generated.h"

class USettingBinding;

/**
 * Base class for implementing how settings are applied to the game.
 *
 * A binding strategy is an optional adapter that knows how to synchronize a setting value with
 * some external target system (console variables, input mappings, custom game logic, etc.) and
 * report target-side changes back to the registry.
 *
 * Inherit from this class and implement OnSyncToTarget to create custom binding strategies.
 */
UCLASS(Abstract, EditInlineNew, Blueprintable)
class AUTOSETTINGS_API USettingBindingStrategy : public UObject
{
	GENERATED_BODY()

public:
	/** Trigger templates for reusable binding invalidation. */
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "Binding")
	TArray<TObjectPtr<USettingBindingTrigger>> Triggers;

	/** Determine if this strategy's external target is currently ready to bind for the given context. */
	UFUNCTION(BlueprintNativeEvent)
	bool IsBindingReady(const FSettingContext& Context);

	/**
	 * Get the current value for this setting from the underlying target external system.
	 * This is distinct from the registry's authoritative applied value.
	 */
	UFUNCTION(BlueprintNativeEvent)
	FString GetTargetValue(const FSettingReference& Setting);

	/** Begin a new active runtime binding. Override to register callbacks or perform other setup. */
	UFUNCTION(BlueprintNativeEvent)
	void BeginBinding(const FSettingReference& Setting, USettingBinding* Binding);

	/** End an active runtime binding. Override to remove callbacks or perform other teardown. */
	UFUNCTION(BlueprintNativeEvent)
	void EndBinding(const FSettingReference& Setting, USettingBinding* Binding);

	/** Validate that this specific setting key/configuration is valid for this binding strategy. */
	UFUNCTION(BlueprintNativeEvent)
	bool ValidateConfiguration(const FSettingReference& Setting);

protected:
	friend class USettingType;

	/** Synchronize a setting value to the external target - only USettingType should call this. */
	UFUNCTION()
	void SyncToTarget(const FSetSettingEvent& Event);

	UFUNCTION(BlueprintNativeEvent)
	void OnSyncToTarget(const FSetSettingEvent& Event);
};
