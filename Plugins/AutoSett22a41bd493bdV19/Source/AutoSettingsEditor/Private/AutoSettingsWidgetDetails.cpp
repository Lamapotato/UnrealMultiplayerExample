// Copyright Sam Bonifacio. All Rights Reserved.

#include "AutoSettingsWidgetDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "Widgets/AutoSettingWidget.h"
#include "Framework/SettingType.h"
#include "Framework/SettingKey.h"
#include "SettingBindingDetailRegistry.h"

TSharedRef<IDetailCustomization> FAutoSettingsWidgetDetails::MakeInstance()
{
	return MakeShareable(new FAutoSettingsWidgetDetails);
}

void FAutoSettingsWidgetDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	// Keep "Setting" at the top; do not hard-code any Setting child categories.
	// Other categories are offset after it, preserving their existing relative order.
	DetailLayout.SortCategories([](const TMap<FName, IDetailCategoryBuilder*>& AllCategoryMap)
		{
			static const FName SettingCategory(TEXT("Setting"));
			static constexpr int32 NonSettingBase = 1000;

			for (const TPair<FName, IDetailCategoryBuilder*>& Pair : AllCategoryMap)
			{
				const FName& CategoryName = Pair.Key;
				int32 DesiredSortOrder = Pair.Value->GetSortOrder();

				if (CategoryName == SettingCategory)
				{
					DesiredSortOrder = 0;
				}
				else
				{
					DesiredSortOrder += NonSettingBase;
				}

				Pair.Value->SetSortOrder(DesiredSortOrder);
			}
		});
	// Get the object being customized
	TArray<TWeakObjectPtr<UObject>> CustomizedObjects;
	DetailLayout.GetObjectsBeingCustomized(CustomizedObjects);

	if (CustomizedObjects.Num() == 1)
	{
		if (UAutoSettingWidget* AutoSettingWidget = Cast<UAutoSettingWidget>(CustomizedObjects[0].Get()))
		{
			// Add binding-specific UI fields
			AddBindingFields(DetailLayout);
		}
	}
}

void FAutoSettingsWidgetDetails::AddBindingFields(IDetailLayoutBuilder& DetailLayout)
{
	// Get references to the properties we need
	TSharedRef<IPropertyHandle> SettingHandle = DetailLayout.GetProperty(TEXT("Setting"));
	TSharedPtr<IPropertyHandle> SettingTypeHandle = SettingHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSettingSpecifier, SettingType));
	TSharedPtr<IPropertyHandle> SubKeyHandle = SettingHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSettingSpecifier, SubKey));

	if (!SettingTypeHandle.IsValid() || !SubKeyHandle.IsValid())
	{
		return;
	}

	UObject* SettingTypeValue = nullptr;
	if (SettingTypeHandle->GetValue(SettingTypeValue) != FPropertyAccess::Success || !SettingTypeValue)
	{
		return;
	}

	UClass* SettingTypeClass = Cast<UClass>(SettingTypeValue);
	if (!SettingTypeClass)
	{
		return;
	}

	const USettingType* SettingTypeCDO = GetDefault<USettingType>(SettingTypeClass);
	if (!SettingTypeCDO)
	{
		return;
	}

	// Let the registry handle UI for the binding strategy
	if (SettingTypeCDO->BindingStrategy)
	{
		// Get the Strategy category for strategy-specific fields
		IDetailCategoryBuilder& StrategyCategory = DetailLayout.EditCategory("Setting|Strategy");

		FSettingBindingDetailRegistry::Get().AddFieldsForBinding(
			StrategyCategory,
			SettingTypeCDO->BindingStrategy,
			SubKeyHandle.ToSharedRef(),
			SettingTypeCDO);
	}

	// TODO: Add support for persistence strategy handlers in the future
	// (PersistenceStrategy is a different base class than BindingStrategy)
}
