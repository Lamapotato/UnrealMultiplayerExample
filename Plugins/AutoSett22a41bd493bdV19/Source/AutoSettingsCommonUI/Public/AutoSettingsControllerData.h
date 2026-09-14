// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonInputBaseTypes.h"
#include "AutoSettingsControllerData.generated.h"

/**
 * The base CommonInputControllerData class locks the allowed Gamepad Names to available platform names.
 * Therefore, it's not possible to have gamepad definitions that do not match the registered platforms.
 * This is an issue in cases where you may want to define multiple gamepad types for a single platform.
 * This subclass circumvents this issue by allowing direct editing of the Gamepad Name.
 */
UCLASS()
class AUTOSETTINGSCOMMONUI_API UAutoSettingsControllerData : public UCommonInputBaseControllerData
{
	GENERATED_BODY()

public:
	/**
	 * Gamepad Name field which is fully editable and not restricted to platform names
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoSettings")
	FName EditableGamepadName;

protected:
	virtual void PreSave(FObjectPreSaveContext ObjectSaveContext) override;
	virtual void PostLoad() override;

private:
	void ApplyEditableGamepadName();
};