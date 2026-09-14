// Copyright Sam Bonifacio. All Rights Reserved.

#include "SettingTypeDetails.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Framework/SettingType.h"
#include "PropertyHandle.h"
#include "SettingKeyDisplayUtils.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<IDetailCustomization> FSettingTypeDetails::MakeInstance()
{
	return MakeShared<FSettingTypeDetails>();
}

void FSettingTypeDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	TArray<TWeakObjectPtr<UObject>> CustomizedObjects;
	DetailLayout.GetObjectsBeingCustomized(CustomizedObjects);

	if (CustomizedObjects.Num() == 1)
	{
		SettingType = Cast<USettingType>(CustomizedObjects[0].Get());
	}

	if (!SettingType)
	{
		return;
	}

	DetailLayout.EditCategory(TEXT("General")).SetSortOrder(0);
	IDetailCategoryBuilder& KeyCategory = DetailLayout.EditCategory(TEXT("Key"));
	KeyCategory.SetSortOrder(1);
	IDetailCategoryBuilder& HierarchyCategory = DetailLayout.EditCategory(TEXT("Hierarchy"));
	HierarchyCategory.SetSortOrder(2);
	DetailLayout.EditCategory(TEXT("Strategies")).SetSortOrder(3);
	DetailLayout.EditCategory(TEXT("Defaults")).SetSortOrder(4);
	DetailLayout.EditCategory(TEXT("Relationships")).SetSortOrder(5);
	DetailLayout.EditCategory(TEXT("Metadata")).SetSortOrder(6);

	TSharedRef<IPropertyHandle> KeyFragmentHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(USettingType, KeyFragment));
	TSharedRef<IPropertyHandle> AllowedChildKeyFragmentsHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(USettingType, AllowedChildKeyFragments));
	DetailLayout.HideProperty(KeyFragmentHandle);
	DetailLayout.HideProperty(AllowedChildKeyFragmentsHandle);
	KeyCategory.AddProperty(KeyFragmentHandle);

	KeyCategory.AddCustomRow(FText::FromString(TEXT("Key Preview")))
		.NameContent()[SNew(STextBlock).Text(FText::FromString(TEXT("Key Preview"))).Font(IDetailLayoutBuilder::GetDetailFont())]
		.ValueContent()
		.MinDesiredWidth(250.f)
			[SNew(SEditableTextBox)
					.IsReadOnly(true)
					.SelectAllTextWhenFocused(true)
					.RevertTextOnEscape(true)
					.Font(FAppStyle::GetFontStyle(TEXT("PropertyWindow.NormalFont")))
					.Text_Lambda([WeakSettingType = TWeakObjectPtr<USettingType>(const_cast<USettingType*>(SettingType))]()
						{
							return SB::AutoSettings::Editor::SettingKeyDisplay::GetPreviewText(WeakSettingType.Get(), FPartialSettingKey());
						})
					.ToolTipText_Lambda([WeakSettingType = TWeakObjectPtr<USettingType>(const_cast<USettingType*>(SettingType))]()
						{
							return SB::AutoSettings::Editor::SettingKeyDisplay::GetPreviewText(WeakSettingType.Get(), FPartialSettingKey());
						})];

	HierarchyCategory.AddProperty(AllowedChildKeyFragmentsHandle);
}
