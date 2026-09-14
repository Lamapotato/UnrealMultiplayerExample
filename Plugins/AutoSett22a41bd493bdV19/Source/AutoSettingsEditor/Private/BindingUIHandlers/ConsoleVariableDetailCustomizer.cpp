// Copyright Sam Bonifacio. All Rights Reserved.

#include "ConsoleVariableDetailCustomizer.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "PropertyHandle.h"
#include "Framework/SettingType.h"
#include "Misc/ConsoleVariableSettingBindingStrategy.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"

bool FConsoleVariableDetailCustomizer::CanHandle(const USettingBindingStrategy* BindingStrategy) const
{
	return BindingStrategy && BindingStrategy->IsA<UConsoleVariableSettingBindingStrategy>();
}

void FConsoleVariableDetailCustomizer::AddFields(
	IDetailCategoryBuilder& CategoryBuilder,
	const USettingBindingStrategy* BindingStrategy,
	TSharedRef<IPropertyHandle> SubKeyHandle,
	const USettingType* SettingType) const
{
	const auto* CVarStrategy = Cast<UConsoleVariableSettingBindingStrategy>(BindingStrategy);
	if (!CVarStrategy)
	{
		return;
	}

	const bool bHasExplicitCVar = !CVarStrategy->ConsoleVariableName.IsNone();

	// Add Console Variable Source as a name-value field
	CategoryBuilder.AddCustomRow(FText::FromString("Console Variable Source"))
		.NameContent()
			[SNew(STextBlock)
					.Text(FText::FromString("Console Variable Source"))
					.ToolTipText(FText::FromString("Indicates where the console variable name is defined"))
					.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))]
		.ValueContent()
		.MinDesiredWidth(150.0f)
			[SNew(STextBlock)
					.Text(bHasExplicitCVar
							? FText::FromString("Setting Type")
							: FText::FromString("Sub Key"))
					.ToolTipText(bHasExplicitCVar
							? FText::FromString("The console variable name is defined directly in the linked Setting Type")
							: FText::FromString("The console variable name comes from the Sub Key property above"))
					.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))];

	// Add Console Variable Name field in the Strategy category
	CategoryBuilder.AddCustomRow(FText::FromString("Console Variable"))
		.NameContent()
			[SNew(STextBlock)
					.Text(FText::FromString("Console Variable Name"))
					.ToolTipText(FText::FromString("The name of the console variable that this setting controls"))
					.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))]
		.ValueContent()
		.MinDesiredWidth(250.0f)
			[SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
					  .FillWidth(1.0f)
						  [SNew(SEditableTextBox)
								  .Text_Lambda([this, CVarStrategy, SubKeyHandle, SettingType]() -> FText
									  {
										  return FText::FromString(GetConsoleVariableName(CVarStrategy, SubKeyHandle, SettingType));
									  })
								  .OnTextCommitted_Lambda([this, bHasExplicitCVar, SubKeyHandle](const FText& NewText, ETextCommit::Type CommitType)
									  {
										  if (!bHasExplicitCVar)
										  {
											  UpdateSubKeyFromConsoleVariable(NewText.ToString(), SubKeyHandle);
										  }
									  })
								  .IsReadOnly(bHasExplicitCVar)
								  .HintText(FText::FromString("Enter console variable name..."))
								  .Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))]];
}

FString FConsoleVariableDetailCustomizer::GetConsoleVariableName(
	const UConsoleVariableSettingBindingStrategy* CVarStrategy,
	TSharedRef<IPropertyHandle> SubKeyHandle,
	const USettingType* SettingType) const
{
	if (!CVarStrategy)
	{
		return FString();
	}

	// Check if explicit console variable name is set
	if (!CVarStrategy->ConsoleVariableName.IsNone())
	{
		return CVarStrategy->ConsoleVariableName.ToString();
	}

	// Get from SubKey
	TSharedPtr<IPropertyHandle> FragmentsHandle = SubKeyHandle->GetChildHandle("Fragments");
	if (!FragmentsHandle.IsValid())
	{
		return FString();
	}

	uint32 NumFragments = 0;
	FragmentsHandle->GetNumChildren(NumFragments);

	if (NumFragments > 0)
	{
		// Get the last fragment
		TSharedPtr<IPropertyHandle> LastFragmentHandle = FragmentsHandle->GetChildHandle(NumFragments - 1);
		if (LastFragmentHandle.IsValid())
		{
			TSharedPtr<IPropertyHandle> NameHandle = LastFragmentHandle->GetChildHandle("Name");
			if (NameHandle.IsValid())
			{
				FName FragmentName;
				if (NameHandle->GetValue(FragmentName) == FPropertyAccess::Success)
				{
					return FragmentName.ToString();
				}
			}
		}
	}
	else if (SettingType && SettingType->KeyFragment.Name != NAME_None)
	{
		// If SubKey is empty, use the SettingType's KeyFragment
		return SettingType->KeyFragment.Name.ToString();
	}

	return FString();
}

void FConsoleVariableDetailCustomizer::UpdateSubKeyFromConsoleVariable(
	const FString& NewCVarName,
	TSharedRef<IPropertyHandle> SubKeyHandle) const
{
	// Don't update if empty
	if (NewCVarName.IsEmpty())
	{
		return;
	}

	// Get the Fragments array handle
	TSharedPtr<IPropertyHandle> FragmentsHandle = SubKeyHandle->GetChildHandle("Fragments");
	if (!FragmentsHandle.IsValid())
	{
		return;
	}

	// Get current number of fragments
	uint32 NumFragments = 0;
	FragmentsHandle->GetNumChildren(NumFragments);

	if (NumFragments > 0)
	{
		// Update the last fragment
		TSharedPtr<IPropertyHandle> LastFragmentHandle = FragmentsHandle->GetChildHandle(NumFragments - 1);
		if (LastFragmentHandle.IsValid())
		{
			TSharedPtr<IPropertyHandle> NameHandle = LastFragmentHandle->GetChildHandle("Name");
			if (NameHandle.IsValid())
			{
				NameHandle->SetValue(FName(*NewCVarName));
			}
		}
	}
	else
	{
		// Add a new fragment to the array
		FragmentsHandle->AsArray()->AddItem();

		// Now set the value of the newly added item
		TSharedPtr<IPropertyHandle> NewFragmentHandle = FragmentsHandle->GetChildHandle(0);
		if (NewFragmentHandle.IsValid())
		{
			TSharedPtr<IPropertyHandle> NameHandle = NewFragmentHandle->GetChildHandle("Name");
			if (NameHandle.IsValid())
			{
				NameHandle->SetValue(FName(*NewCVarName));
			}
		}
	}
}
