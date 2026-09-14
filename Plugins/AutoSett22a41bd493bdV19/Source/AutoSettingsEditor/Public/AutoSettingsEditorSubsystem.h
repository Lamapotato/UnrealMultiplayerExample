// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "AutoSettingsEditorSubsystem.generated.h"

class USettingRegistry;

/**
 * Editor subsystem providing design-time setting registries for widget preview.
 * Allows setting widgets to display proper values in the editor before runtime.
 */
UCLASS()
class AUTOSETTINGSEDITOR_API UAutoSettingsEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	/**
	 * The design time registries are used to show setting values in the widget editor at design time, rather than the registries
	 * operating at runtime that are held in other locations
	 */

	/**
	 * Design time registry for global settings
	 */
	UPROPERTY()
	USettingRegistry* DesignTimeGlobalRegistry;

	/**
	 * Design time registry for player-specific settings
	 */
	UPROPERTY()
	USettingRegistry* DesignTimePlayerRegistry;
};