// Copyright Sam Bonifacio. All Rights Reserved.

#include "Misc/ConsoleVariableSettingBindingStrategy.h"

#include "AutoSettingsError.h"
#include "ConsoleUtilities.h"
#include "Framework/SettingBinding.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/SettingValueMask.h"
#include "Framework/SettingType.h"

UConsoleVariableSettingBindingStrategy::UConsoleVariableSettingBindingStrategy() {}

void UConsoleVariableSettingBindingStrategy::OnSyncToTarget_Implementation(const FSetSettingEvent& Event)
{
	const FName FinalCVarName = GetConsoleVariableName(Event.Setting.Key);

	FString FinalValue;
	if (ValueMask)
	{
		const FString CurrentCVarValue = UKismetSystemLibrary::GetConsoleVariableStringValue(FinalCVarName.ToString());
		FinalValue = ValueMask->RecombineValues(Event.Data.Value, CurrentCVarValue);
	}
	else
	{
		FinalValue = Event.Data.Value;
	}

	SB::AutoSettings::Console::SetValue<FString>(FinalCVarName, FinalValue);
}

FString UConsoleVariableSettingBindingStrategy::GetTargetValue_Implementation(const FSettingReference& Setting)
{
	const FName FinalCVarName = GetConsoleVariableName(Setting.Key);

	if (!SB::AutoSettings::Console::IsVariableRegistered(FinalCVarName))
	{
		const FString Context = FString::Printf(TEXT("Reading currently applied value for setting '%s'"), *Setting.Key.ToString());
		SB::AutoSettings::Errors::LogMissingCVar(Context, FinalCVarName);
		return FString();
	}

	// We know it exists, so GetConsoleVariable won't log an error
	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*FinalCVarName.ToString());
	if (!ensure(CVar))
	{
		return FString();
	}

	FString CVarValue = CVar->GetString();

	if (ValueMask)
	{
		return ValueMask->MaskValue(CVarValue);
	}

	return CVarValue;
}

bool UConsoleVariableSettingBindingStrategy::ValidateConfiguration_Implementation(const FSettingReference& Setting)
{
	const FName FinalCVarName = GetConsoleVariableName(Setting.Key);
	return SB::AutoSettings::Console::IsVariableRegistered(FinalCVarName);
}

void UConsoleVariableSettingBindingStrategy::BeginBinding_Implementation(const FSettingReference& Setting, USettingBinding* Binding)
{
	Super::BeginBinding_Implementation(Setting, Binding);

	if (!Binding)
	{
		return;
	}

	const FName Name = GetConsoleVariableName(Setting.Key);

	IConsoleVariable* ConsoleVariable = IConsoleManager::Get().FindConsoleVariable(*Name.ToString());

	if (!ConsoleVariable)
	{
		SB::AutoSettings::Errors::LogError(FString::Printf(TEXT("Tried to find console variable '%s' that doesn't exist"),
			*Name.ToString()));
		return;
	}

	ConsoleVariable->OnChangedDelegate().AddWeakLambda(Binding, [Binding](IConsoleVariable* ChangedConsoleVariable)
		{
			if (Binding)
			{
				Binding->NotifyTargetValueChanged();
			}
		});
}

void UConsoleVariableSettingBindingStrategy::EndBinding_Implementation(const FSettingReference& Setting, USettingBinding* Binding)
{
	const FName Name = GetConsoleVariableName(Setting.Key);

	IConsoleVariable* ConsoleVariable = IConsoleManager::Get().FindConsoleVariable(*Name.ToString());

	if (!ConsoleVariable)
	{
		SB::AutoSettings::Errors::LogError(FString::Printf(TEXT("Tried to find console variable '%s' that doesn't exist"),
			*Name.ToString()));
		return;
	}

	ConsoleVariable->OnChangedDelegate().RemoveAll(Binding);

	Super::EndBinding_Implementation(Setting, Binding);
}

FName UConsoleVariableSettingBindingStrategy::GetConsoleVariableName(const FFullSettingKey& Key) const
{
	if (!ConsoleVariableName.IsNone())
	{
		// If the CVar name is explicitly provided, use that
		return ConsoleVariableName;
	}

	// Use the setting key leaf fragment as the CVar name
	return Key.GetLastFragment().Name;
}
