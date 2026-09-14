// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "AutoSettingsCommonComboBoxString.h"

#include "CoreMinimal.h"

#include "AutoSettingsCommonComboBoxStringTests.generated.h"

// Test double that exposes the combo box open/close lifecycle without changing the production widget API.
UCLASS(HideDropdown, NotBlueprintable)
class AUTOSETTINGSCOMMONUITESTS_API UTestAutoSettingsCommonComboBoxString : public UAutoSettingsCommonComboBoxString
{
	GENERATED_BODY()

public:
	// Exercise the same opening logic that the underlying Slate combo box calls when its popup opens.
	void OpenForTest();

	// Report how many times the widget observed its popup-close path during the current test.
	int32 GetCloseCount() const;

protected:
	// Count close handling while preserving the production cleanup performed by the base widget.
	virtual void HandleClosing() override;

private:
	// Number of observed close-handling calls for the current transient test widget instance.
	int32 CloseCount = 0;
};
