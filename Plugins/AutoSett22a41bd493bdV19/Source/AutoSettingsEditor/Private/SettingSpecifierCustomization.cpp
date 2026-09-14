// Copyright Sam Bonifacio. All Rights Reserved.

#include "SettingSpecifierCustomization.h"

#include "DetailWidgetRow.h"
#include "Framework/SettingKey.h"
#include "Framework/SettingType.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyHandle.h"
#include "SettingKeyDisplayUtils.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<IPropertyTypeCustomization> FSettingSpecifierCustomization::MakeInstance()
{
	return MakeShared<FSettingSpecifierCustomization>();
}

void FSettingSpecifierCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	HeaderRow.NameContent()[StructPropertyHandle->CreatePropertyNameWidget()]
		.ValueContent()[StructPropertyHandle->CreatePropertyValueWidget()];
}

void FSettingSpecifierCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle,
	IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	SettingTypeHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSettingSpecifier, SettingType));
	SubKeyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSettingSpecifier, SubKey));

	if (SettingTypeHandle.IsValid())
	{
		StructBuilder.AddProperty(SettingTypeHandle.ToSharedRef());
	}

	if (SubKeyHandle.IsValid())
	{
		StructBuilder.AddProperty(SubKeyHandle.ToSharedRef());
	}

	const TSharedPtr<IPropertyHandle> LocalSettingTypeHandle = SettingTypeHandle;
	const TSharedPtr<IPropertyHandle> LocalSubKeyHandle = SubKeyHandle;

	StructBuilder.AddCustomRow(FText::FromString(TEXT("Key Preview")))
		.NameContent()[SNew(STextBlock).Text(FText::FromString(TEXT("Key Preview"))).Font(FAppStyle::GetFontStyle(TEXT("PropertyWindow.NormalFont")))]
		.ValueContent()
		.MinDesiredWidth(250.f)
			[SNew(SEditableTextBox)
					.IsReadOnly(true)
					.SelectAllTextWhenFocused(true)
					.RevertTextOnEscape(true)
					.Font(FAppStyle::GetFontStyle(TEXT("PropertyWindow.NormalFont")))
					.Text_Lambda([LocalSettingTypeHandle, LocalSubKeyHandle]()
						{
							return SB::AutoSettings::Editor::SettingKeyDisplay::GetPreviewText(
								ResolveSettingType(LocalSettingTypeHandle),
								ResolveSubKey(LocalSubKeyHandle));
						})
					.ToolTipText_Lambda([LocalSettingTypeHandle, LocalSubKeyHandle]()
						{
							return SB::AutoSettings::Editor::SettingKeyDisplay::GetPreviewText(
								ResolveSettingType(LocalSettingTypeHandle),
								ResolveSubKey(LocalSubKeyHandle));
						})];
}

const USettingType* FSettingSpecifierCustomization::ResolveSettingType(TSharedPtr<IPropertyHandle> SettingTypeHandle)
{
	if (!SettingTypeHandle.IsValid())
	{
		return nullptr;
	}

	UObject* SettingTypeValue = nullptr;
	if (SettingTypeHandle->GetValue(SettingTypeValue) != FPropertyAccess::Success)
	{
		return nullptr;
	}

	if (UClass* SettingTypeClass = Cast<UClass>(SettingTypeValue))
	{
		return GetDefault<USettingType>(SettingTypeClass);
	}

	return nullptr;
}

FPartialSettingKey FSettingSpecifierCustomization::ResolveSubKey(TSharedPtr<IPropertyHandle> SubKeyHandle)
{
	FPartialSettingKey SubKey;
	if (!SubKeyHandle.IsValid())
	{
		return SubKey;
	}

	TArray<void*> RawData;
	SubKeyHandle->AccessRawData(RawData);
	if (RawData.Num() == 1 && RawData[0])
	{
		SubKey = *static_cast<FPartialSettingKey*>(RawData[0]);
	}

	return SubKey;
}
