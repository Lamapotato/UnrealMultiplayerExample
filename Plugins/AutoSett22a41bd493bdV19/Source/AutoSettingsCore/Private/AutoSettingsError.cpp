// Copyright Sam Bonifacio. All Rights Reserved.

#include "AutoSettingsError.h"
#include "Logging/MessageLog.h"
#include "AutoSettingsLogs.h"
#include "UObject/Class.h"

void SB::AutoSettings::Errors::LogError(const FString& Message)
{
	// PIE message log
	FMessageLog("PIE").Error(FText::FromString("Auto Settings: " + Message));
	// Output log
	UE_LOG(LogAutoSettings, Error, TEXT("%s"), *Message);
}

void SB::AutoSettings::Errors::LogError(const UObject* SourceObject, const FString& Message)
{
	LogError(FString::Printf(TEXT("%s: %s"), *GetNameSafe(SourceObject), *Message));
}

void SB::AutoSettings::Errors::LogInvalidClass(const FString& SourceObjectName, const FString& ClassPropertyName)
{
	LogError(FString::Printf(TEXT("%s has an invalid %s. Please update the property to a valid class."), *SourceObjectName, *ClassPropertyName));
}

void SB::AutoSettings::Errors::LogInvalidClass(const UClass* SourceClass, const FString& ClassPropertyName)
{
	LogInvalidClass(GetNameSafe(SourceClass), ClassPropertyName);
}

void SB::AutoSettings::Errors::LogWidgetCreationFailed(const UClass* SourceClass, const FString& WidgetName)
{
	LogError(FString::Printf(TEXT("%s: Failed to create %s widget"), *GetNameSafe(SourceClass), *WidgetName));
}

void SB::AutoSettings::Errors::LogMissingBindWidget(const UClass* SourceClass, FName PropertyName)
{
	LogError(FString::Printf(TEXT("%s: Missing or invalid BindWidget for property %s. Please make a widget of the correct type and name exists."), *GetNameSafe(SourceClass), *PropertyName.ToString()));
}

void SB::AutoSettings::Errors::LogMissingCVar(const FString& Context, FName CVar)
{
	LogError(FString::Printf(TEXT("%s: Missing or unregistered CVar: '%s'. Please ensure the CVar is registered correctly."), *Context, *CVar.ToString()));
}

template <>
void SB::AutoSettings::Errors::ValidateSettingsProperty<UClass*>(const FString& CategoryName, const FString& PropertyName, UClass* const& ActualClass, UClass* const& ExpectedClass)
{
	if (ActualClass->IsChildOf(ExpectedClass))
	{
		return;
	}

	FString ExpectedClassName = GetDisplayName(ExpectedClass);
	FString CurrentClassName = GetDisplayName(ActualClass);
	LogError(FString::Format(TEXT("'{0}' project settings property '{1}' must be '{2}' or a subclass. Currently '{3}'."),
		{ CategoryName,
			PropertyName,
			ExpectedClassName,
			CurrentClassName }));
}

FString SB::AutoSettings::Errors::GetDisplayName(const UClass* Class)
{
#if WITH_EDITORONLY_DATA
	return Class->GetDisplayNameText().ToString();
#else
	return GetNameSafe(Class);
#endif
}