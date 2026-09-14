// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ISettingBindingDetailCustomizer.h"

class UConsoleVariableSettingBindingStrategy;

/**
 * Detail customizer for ConsoleVariableSettingBindingStrategy that provides
 * a console variable name field and related info panels
 */
class FConsoleVariableDetailCustomizer : public ISettingBindingDetailCustomizer
{
public:
	virtual bool CanHandle(const USettingBindingStrategy* BindingStrategy) const override;

	virtual void AddFields(
		IDetailCategoryBuilder& CategoryBuilder,
		const USettingBindingStrategy* BindingStrategy,
		TSharedRef<IPropertyHandle> SubKeyHandle,
		const USettingType* SettingType) const override;

private:
	/**
	 * Get the current console variable name for display
	 * @param CVarStrategy The console variable strategy
	 * @param SubKeyHandle Handle to the SubKey property
	 * @param SettingType The setting type containing the strategy
	 * @return The console variable name to display
	 */
	FString GetConsoleVariableName(
		const UConsoleVariableSettingBindingStrategy* CVarStrategy,
		TSharedRef<IPropertyHandle> SubKeyHandle,
		const USettingType* SettingType) const;

	/**
	 * Update SubKey when console variable name changes
	 * @param NewCVarName The new console variable name
	 * @param SubKeyHandle Handle to the SubKey property
	 */
	void UpdateSubKeyFromConsoleVariable(
		const FString& NewCVarName,
		TSharedRef<IPropertyHandle> SubKeyHandle) const;
};
