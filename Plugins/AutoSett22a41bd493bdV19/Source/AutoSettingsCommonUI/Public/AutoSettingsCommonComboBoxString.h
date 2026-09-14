// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ComboBoxString.h"
#include "Input/UIActionBindingHandle.h"
#include "AutoSettingsCommonComboBoxString.generated.h"

/**
 * A ComboBox with some extra glue to register its own CommonUI back action when the ComboBox is open
 * This is required for the ComboBox to consume the back action when it's closed so that it doesn't fall through
 * to parent menus
 */
UCLASS()
class AUTOSETTINGSCOMMONUI_API UAutoSettingsCommonComboBoxString : public UComboBoxString
{
	GENERATED_BODY()

public:
	virtual void OnWidgetRebuilt() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

protected:
	virtual void HandleOpening() override;

	// Restore navigation and back-action state when focus returning to the combo box indicates the popup closed.
	virtual void HandleClosing();

private:
	// Tracks the Slate focus-change listener that detects when the combo box popup closes.
	FDelegateHandle FocusChangingHandle;

	// Tracks the temporary CommonUI back action binding that consumes back input while the popup is open.
	FUIActionBindingHandle BackActionBindingHandle;

	// Flag to track pending async unregister and prevent race conditions during rapid open/close cycles
	bool bPendingBackHandlerUnregister = false;

	// Register the Slate close-detection listener for the current Slate widget instance.
	void RegisterCloseDetection();

	// Unregister the Slate close-detection listener and clear the stored delegate handle.
	void UnregisterCloseDetection();

	// Register the CommonUI back action that prevents popup back input from bubbling to parent widgets.
	void RegisterBackHandler();

	// Unregister the temporary CommonUI back action after the popup has had a chance to consume input.
	void UnregisterBackHandler();

	// Block directional navigation while the combo box popup owns navigation focus.
	UFUNCTION()
	UWidget* HandleMenuNavigation(EUINavigation NavDirection);
};
