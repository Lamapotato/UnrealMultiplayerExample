// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/SettingKey.h"
#include "IPropertyTypeCustomization.h"

class FDetailWidgetRow;
class IDetailChildrenBuilder;
class IPropertyHandle;
class IPropertyTypeCustomizationUtils;
class USettingType;

class FSettingSpecifierCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder,
		IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:
	static const USettingType* ResolveSettingType(TSharedPtr<IPropertyHandle> SettingTypeHandle);
	static FPartialSettingKey ResolveSubKey(TSharedPtr<IPropertyHandle> SubKeyHandle);

	TSharedPtr<IPropertyHandle> SettingTypeHandle;
	TSharedPtr<IPropertyHandle> SubKeyHandle;
};
