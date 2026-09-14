// Copyright Sam Bonifacio. All Rights Reserved.

#include "AutoSettingsControllerData.h"
#include "UObject/ObjectSaveContext.h"

void UAutoSettingsControllerData::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
	ApplyEditableGamepadName();
	Super::PreSave(ObjectSaveContext);
}

void UAutoSettingsControllerData::PostLoad()
{
	Super::PostLoad();
	ApplyEditableGamepadName();
}

void UAutoSettingsControllerData::ApplyEditableGamepadName()
{
	if (!EditableGamepadName.IsNone())
	{
		GamepadName = EditableGamepadName;
	}
}