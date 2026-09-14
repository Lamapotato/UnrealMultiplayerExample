// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class IDetailCategoryBuilder;
class IDetailLayoutBuilder;
class IPropertyHandle;
class USettingBindingStrategy;
class USettingType;

/**
 * Interface for classes that provide custom detail panel UI for specific strategy types
 * in the AutoSettingWidget details panel
 */
class ISettingBindingDetailCustomizer
{
public:
	virtual ~ISettingBindingDetailCustomizer() = default;

	/**
	 * Check if this handler can provide UI for the given strategy
	 * @param Strategy The strategy to check
	 * @return true if this handler can provide UI for this strategy
	 */
	virtual bool CanHandle(const USettingBindingStrategy* BindingStrategy) const = 0;

	/**
	 * Add custom UI fields for the given strategy
	 * @param CategoryBuilder The category builder for adding strategy-specific fields
	 * @param Strategy The strategy to add UI for
	 * @param SubKeyHandle Handle to the SubKey property
	 * @param SettingType The setting type containing the strategy
	 */
	virtual void AddFields(
		IDetailCategoryBuilder& CategoryBuilder,
		const USettingBindingStrategy* BindingStrategy,
		TSharedRef<IPropertyHandle> SubKeyHandle,
		const USettingType* SettingType) const = 0;
};
