// Copyright Sam Bonifacio. All Rights Reserved.

#include "AutoSettingsConfigCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Misc/AutoSettingsConfig.h"
#include "Widgets/Layout/SBox.h"
#include "SWarningOrErrorBox.h"

TSharedRef<IDetailCustomization> FAutoSettingsConfigCustomization::MakeInstance()
{
	return MakeShareable(new FAutoSettingsConfigCustomization);
}

void FAutoSettingsConfigCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	IDetailCategoryBuilder& SettingsCategory = DetailLayout.EditCategory(TEXT("Settings"));

	TSharedPtr<IPropertyHandle> SettingsIniNameProperty = DetailLayout.GetProperty(
		GET_MEMBER_NAME_CHECKED(UAutoSettingsConfig, SettingsIniName));

	if (SettingsIniNameProperty.IsValid())
	{
		// Add the property normally
		SettingsCategory.AddProperty(SettingsIniNameProperty);

		// Add warning row below the property
		SettingsCategory.AddCustomRow(FText::FromString(TEXT("ConfigNameWarning")))
			.Visibility(TAttribute<EVisibility>::CreateLambda([SettingsIniNameProperty]()
				{
					FString ConfigName;
					if (SettingsIniNameProperty.IsValid() && SettingsIniNameProperty->GetValue(ConfigName) == FPropertyAccess::Success)
					{
						return UAutoSettingsConfig::IsConfigNameValid(ConfigName) ? EVisibility::Collapsed : EVisibility::Visible;
					}
					return EVisibility::Collapsed;
				}))
			.WholeRowContent()[SNew(SBox).Padding(FMargin(0.f, 4.f))[SNew(SWarningOrErrorBox).MessageStyle(EMessageStyle::Warning).Message_Lambda([SettingsIniNameProperty]() -> FText
				{
					return FText::FromString(TEXT("Config name may not be writable. Should contain 'user' (case-insensitive) or have bCanSaveAllSections=true"));
				})]];
	}
}
