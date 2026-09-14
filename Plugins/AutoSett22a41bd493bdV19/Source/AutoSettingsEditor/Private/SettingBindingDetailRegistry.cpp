// Copyright Sam Bonifacio. All Rights Reserved.

#include "SettingBindingDetailRegistry.h"
#include "Framework/SettingType.h"

FSettingBindingDetailRegistry& FSettingBindingDetailRegistry::Get()
{
	static FSettingBindingDetailRegistry Instance;
	return Instance;
}

void FSettingBindingDetailRegistry::RegisterCustomizer(TSharedPtr<ISettingBindingDetailCustomizer> Customizer)
{
	if (Customizer.IsValid())
	{
		Customizers.Add(Customizer);
	}
}

void FSettingBindingDetailRegistry::AddFieldsForBinding(
	IDetailCategoryBuilder& CategoryBuilder,
	const USettingBindingStrategy* BindingStrategy,
	TSharedRef<IPropertyHandle> SubKeyHandle,
	const USettingType* SettingType) const
{
	if (!BindingStrategy)
	{
		return;
	}

	// Find the first customizer that can handle this strategy
	for (const auto& Customizer : Customizers)
	{
		if (Customizer.IsValid() && Customizer->CanHandle(BindingStrategy))
		{
			Customizer->AddFields(CategoryBuilder, BindingStrategy, SubKeyHandle, SettingType);
			break; // Use first matching customizer
		}
	}
}
