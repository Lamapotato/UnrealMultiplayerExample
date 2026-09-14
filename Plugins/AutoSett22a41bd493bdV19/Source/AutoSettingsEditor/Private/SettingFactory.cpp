// Copyright Sam Bonifacio. All Rights Reserved.

#include "SettingFactory.h"
#include "AssetToolsModule.h"
#include "Framework/SettingType.h"

USettingFactory::USettingFactory()
{
	SupportedClass = USettingType::StaticClass();

	bCreateNew = true;
	bEditAfterNew = true;
	bEditorImport = false;
}

UObject* USettingFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<USettingType>(InParent, Class, Name, Flags);
}

FText USettingFactory::GetDisplayName() const
{
	// Keep the class name as an FString because NameToDisplayString accepts an FString reference.
	const FString SettingTypeClassName = USettingType::StaticClass()->GetName();

	return FText::FromString(FName::NameToDisplayString(SettingTypeClassName, false));
}
