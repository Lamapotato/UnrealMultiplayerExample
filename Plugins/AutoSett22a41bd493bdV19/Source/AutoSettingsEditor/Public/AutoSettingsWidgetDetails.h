// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IDetailCategoryBuilder;
class IPropertyHandle;
class UAutoSettingWidget;

/**
 *
 */
class AUTOSETTINGSEDITOR_API FAutoSettingsWidgetDetails : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

private:
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;

	/** Add binding-specific UI fields for the given binding strategies */
	void AddBindingFields(IDetailLayoutBuilder& DetailLayout);
};
