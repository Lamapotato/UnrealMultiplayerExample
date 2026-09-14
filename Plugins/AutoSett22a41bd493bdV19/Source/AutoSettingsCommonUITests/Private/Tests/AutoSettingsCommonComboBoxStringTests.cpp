// Copyright Sam Bonifacio. All Rights Reserved.

#include "AutoSettingsCommonComboBoxStringTests.h"

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "CQTest.h"
#include "Framework/Application/SlateApplication.h"
#include "Layout/WidgetPath.h"
#include "Misc/ScopeExit.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

void UTestAutoSettingsCommonComboBoxString::OpenForTest()
{
	// Route through the production opening hook so the test does not duplicate popup-open setup.
	HandleOpening();
}

int32 UTestAutoSettingsCommonComboBoxString::GetCloseCount() const
{
	// Return the recorded close count so assertions stay focused on observable widget behavior.
	return CloseCount;
}

void UTestAutoSettingsCommonComboBoxString::HandleClosing()
{
	// Record the close event before running the real cleanup so a failing cleanup path is still observable.
	++CloseCount;

	Super::HandleClosing();
}

TEST_CLASS(CommonComboBoxString, "SB.AutoSettings.CommonUI")
{
	TEST_METHOD(RestoresCloseHandlingWhenReaddedToUi)
	{
		// The widget must keep its popup-close behavior across normal UMG remove and re-add lifecycles.
		if (!TestRunner->AddErrorIfFalse(FSlateApplication::IsInitialized(), TEXT("Slate must be initialized before testing combo box close detection.")))
		{
			return;
		}

		// Keep the transient combo box alive for the full test while exercising its normal UMG lifecycle.
		UTestAutoSettingsCommonComboBoxString* ComboBox = NewObject<UTestAutoSettingsCommonComboBoxString>(GetTransientPackage());
		TStrongObjectPtr<UTestAutoSettingsCommonComboBoxString> ComboBoxOwner(ComboBox);

		// Always remove any focus delegate left by the test so later Slate work cannot call back into this object.
		ON_SCOPE_EXIT
		{
			if (FSlateApplication::IsInitialized() && ComboBox)
			{
				FSlateApplication::Get().OnFocusChanging().RemoveAll(ComboBox);
			}
		};

		if (!TestRunner->AddErrorIfFalse(ComboBox != nullptr, TEXT("Failed to create the CommonUI combo box test widget.")))
		{
			return;
		}

		// Adding the widget to the UI builds the first Slate widget instance.
		ComboBox->TakeWidget();

		// Removing the widget from the UI releases the first Slate widget instance.
		ComboBox->ReleaseSlateResources(false);

		// Re-adding the same widget must restore close handling for the new Slate widget instance.
		const TSharedRef<SWidget> ReaddedWidget = ComboBox->TakeWidget();

		// Open the popup through the widget's normal opening hook before simulating the matching close signal.
		ComboBox->OpenForTest();

		// Focus returning to the combo box is the close signal this widget receives from the underlying Slate control.
		FSlateApplication::Get().OnFocusChanging().Broadcast(
			FFocusEvent(EFocusCause::SetDirectly, 0),
			FWeakWidgetPath(),
			nullptr,
			FWidgetPath(),
			ReaddedWidget);

		// Close handling is the behavior that must survive removing and re-adding the same widget instance.
		TestRunner->AddErrorIfFalse(ComboBox->GetCloseCount() == 1, TEXT("Combo box did not handle close when re-added to the UI."));
	}
};

#endif
