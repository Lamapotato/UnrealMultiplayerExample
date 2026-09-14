// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BindingUIHandlers/ISettingBindingDetailCustomizer.h"

class IDetailCategoryBuilder;
class IPropertyHandle;
class USettingBindingStrategy;
class USettingType;

/**
 * Registry for setting binding detail customizers that manages and dispatches to appropriate customizers
 * based on binding type
 */
class FSettingBindingDetailRegistry
{
public:
	/**
	 * Get the singleton instance of the registry
	 */
	static FSettingBindingDetailRegistry& Get();

	/**
	 * Register a detail customizer for bindings
	 * @param Customizer The customizer to register
	 */
	void RegisterCustomizer(TSharedPtr<ISettingBindingDetailCustomizer> Customizer);

	/**
	 * Add UI fields for the given binding by finding and delegating to the appropriate handler
	 * @param CategoryBuilder The category builder for adding binding-specific fields
	 * @param BindingStrategy The binding strategy to add UI for
	 * @param SubKeyHandle Handle to the SubKey property
	 * @param SettingType The setting type containing the strategy
	 */
	void AddFieldsForBinding(
		IDetailCategoryBuilder& CategoryBuilder,
		const USettingBindingStrategy* BindingStrategy,
		TSharedRef<IPropertyHandle> SubKeyHandle,
		const USettingType* SettingType) const;

private:
	/** Registered detail customizers */
	TArray<TSharedPtr<ISettingBindingDetailCustomizer>> Customizers;
};
