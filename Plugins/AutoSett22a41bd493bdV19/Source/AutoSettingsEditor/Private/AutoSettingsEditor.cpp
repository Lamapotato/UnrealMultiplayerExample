// Copyright Sam Bonifacio. All Rights Reserved.

#include "AutoSettingsEditor.h"
#include "AutoSettingsConfigCustomization.h"
#include "AutoSettingsWidgetDetails.h"
#include "SettingSpecifierCustomization.h"
#include "SettingTypeDetails.h"
#include "Framework/SettingKey.h"
#include "Framework/SettingType.h"
#include "PropertyEditorModule.h"
#include "AutoSettingsInput/Public/Widgets/InputMappingSettingWidget.h"
#include "Widgets/AutoSettingWidget.h"
#include "Misc/AutoSettingsConfig.h"
#include "SettingBindingDetailRegistry.h"
#include "BindingUIHandlers/ConsoleVariableDetailCustomizer.h"

#define LOCTEXT_NAMESPACE "FAutoSettingsEditorModule"

void FAutoSettingsEditorModule::StartupModule()
{
	// Register setting widget details
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(UAutoSettingWidget::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FAutoSettingsWidgetDetails::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(UInputMappingSettingWidget::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FAutoSettingsWidgetDetails::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(USettingType::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FSettingTypeDetails::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(UAutoSettingsConfig::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FAutoSettingsConfigCustomization::MakeInstance));
	PropertyModule.RegisterCustomPropertyTypeLayout(FSettingSpecifier::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSettingSpecifierCustomization::MakeInstance));
	PropertyModule.NotifyCustomizationModuleChanged();

	// Register binding detail customizers
	FSettingBindingDetailRegistry& Registry = FSettingBindingDetailRegistry::Get();
	Registry.RegisterCustomizer(MakeShareable(new FConsoleVariableDetailCustomizer()));
}

void FAutoSettingsEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout(UAutoSettingWidget::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UInputMappingSettingWidget::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(USettingType::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UAutoSettingsConfig::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout(FSettingSpecifier::StaticStruct()->GetFName());
		PropertyModule.NotifyCustomizationModuleChanged();
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAutoSettingsEditorModule, AutoSettingsEditor)
