// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

namespace SB::AutoSettings::Errors
{
	AUTOSETTINGSCORE_API void LogError(const FString& Message);
	AUTOSETTINGSCORE_API void LogError(const UObject* SourceObject, const FString& Message);

	AUTOSETTINGSCORE_API void LogInvalidClass(const FString& SourceObjectName, const FString& ClassPropertyName);
	AUTOSETTINGSCORE_API void LogInvalidClass(const UClass* SourceClass, const FString& ClassPropertyName);

	AUTOSETTINGSCORE_API void LogWidgetCreationFailed(const UClass* SourceClass, const FString& WidgetName);
	AUTOSETTINGSCORE_API void LogMissingBindWidget(const UClass* SourceClass, FName PropertyName);

	AUTOSETTINGSCORE_API void LogMissingCVar(const FString& Context, FName CVar);

	template <typename T>
	void ValidateSettingsProperty(const FString& CategoryName, const FString& PropertyName, const T& ActualValue, const T& ExpectedValue)
	{
		if (ActualValue == ExpectedValue)
		{
			return;
		}

		LogError(FString::Format(TEXT("'{0}' project settings property '{1}' must be '{2}'. Currently '{3}'."),
			{ CategoryName,
				PropertyName,
				LexToString(ExpectedValue),
				LexToString(ActualValue) }));
	}

	template <>
	AUTOSETTINGSCORE_API void ValidateSettingsProperty<UClass*>(const FString& CategoryName, const FString& PropertyName, UClass* const& ActualClass, UClass* const& ExpectedClass);

	AUTOSETTINGSCORE_API FString GetDisplayName(const UClass* Class);
} // namespace SB::AutoSettings::Errors