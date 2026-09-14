// Copyright Sam Bonifacio. All Rights Reserved.

#include "../Public/AutoSettingsConsoleVariable.h"
#include "AutoSettingsLogs.h"
#include "ConsoleUtilities.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "Logging/StructuredLog.h"

IConsoleVariable* UAutoSettingsConsoleVariable::CreateConsoleVariable()
{
	UE_LOGFMT(LogAutoSettings, VeryVerbose, "{0}: Creating console variable '{1}'", *GetName(), *Name.ToString());

	if (!ensure(!SB::AutoSettings::Console::IsVariableRegistered(Name)))
	{
		UE_LOGFMT(LogAutoSettings, Error, "Console variable '{0}' is already registered", *Name.ToString());
		return nullptr;
	}

	return RegisterConsoleVariable();
}

void UAutoSettingsConsoleVariable::RegisterCallback()
{
	UE_LOGFMT(LogAutoSettings, VeryVerbose, "{0}: Registering callback for console variable '{1}'", *GetName(), *Name.ToString());

	if (!SB::AutoSettings::Console::IsVariableRegistered(Name))
	{
		UE_LOGFMT(LogAutoSettings, Warning, "Console variable '{0}' is not registered, callback registration may fail", *Name.ToString());
	}

	RegisterChangedCallback();
}

UWorld* UAutoSettingsConsoleVariable::GetWorld() const
{
	if (GetOuter()->IsA<UPackage>())
	{
		return nullptr;
	}
	return Super::GetWorld();
}

FPrimaryAssetId UAutoSettingsConsoleVariable::GetPrimaryAssetId() const
{
	FName AssetName;

	if (!Name.IsNone())
	{
		// Use the CVar name if it exists
		AssetName = Name;
	}
	else
	{
		// Otherwise use the class name, so that abstract base classes with no name still work
		AssetName = FPackageName::GetShortFName(GetOutermost()->GetName());
	}
	// if(GetClass()->HasAllClassFlags(CLASS_Abstract))
	// {
	// 	// Todo: Test we can have multiple abstract cvars without issue
	// 	AssetName = TEXT("ABSTRACT_CVAR");
	// }

	return FPrimaryAssetId(ConsoleVariablePrimaryAssetTypeName, AssetName);
}

// Template implementations to replace macros
template <typename TValue>
IConsoleVariable* RegisterConsoleVariableImpl(const FName& Name, const TValue& DefaultValue, const FString& Help)
{
	return SB::AutoSettings::Console::RegisterVariable<TValue>(Name, DefaultValue, Help);
}

// Single templated callback registration using SB::AutoSettings::Console namespace
template <typename TValue, typename TThis>
void RegisterCallbackImpl(TThis* This, const FName& Name, bool bCallOnChangedWhenRegistered)
{
	// Use weak object pointer to prevent crashes if the UObject is garbage collected
	TWeakObjectPtr<TThis> WeakThis(This);

	// Create a callback using TFunction with weak lambda capture
	TFunction<void(TValue)> CppCallback = [WeakThis](TValue NewValue)
	{
		// Check if the object is still valid before calling
		if (TThis* StrongThis = WeakThis.Get())
		{
			// Call the public wrapper which accesses the event
			StrongThis->CallOnChanged(NewValue);
		}
	};
	SB::AutoSettings::Console::RegisterVariableChangedCallback<TValue>(Name, CppCallback, bCallOnChangedWhenRegistered);
}

IConsoleVariable* UConsoleVariable_Boolean::RegisterConsoleVariable()
{
	return RegisterConsoleVariableImpl(Name, DefaultValue, Help);
}

void UConsoleVariable_Boolean::RegisterChangedCallback()
{
	RegisterCallbackImpl<bool>(this, Name, bCallOnChangedWhenRegistered);
}

IConsoleVariable* UConsoleVariable_Integer::RegisterConsoleVariable()
{
	return RegisterConsoleVariableImpl(Name, DefaultValue, Help);
}

void UConsoleVariable_Integer::RegisterChangedCallback()
{
	RegisterCallbackImpl<int32>(this, Name, bCallOnChangedWhenRegistered);
}

IConsoleVariable* UConsoleVariable_Float::RegisterConsoleVariable()
{
	return RegisterConsoleVariableImpl(Name, DefaultValue, Help);
}

void UConsoleVariable_Float::RegisterChangedCallback()
{
	RegisterCallbackImpl<float>(this, Name, bCallOnChangedWhenRegistered);
}

IConsoleVariable* UConsoleVariable_String::RegisterConsoleVariable()
{
	return RegisterConsoleVariableImpl(Name, DefaultValue, Help);
}

void UConsoleVariable_String::RegisterChangedCallback()
{
	RegisterCallbackImpl<FString>(this, Name, bCallOnChangedWhenRegistered);
}