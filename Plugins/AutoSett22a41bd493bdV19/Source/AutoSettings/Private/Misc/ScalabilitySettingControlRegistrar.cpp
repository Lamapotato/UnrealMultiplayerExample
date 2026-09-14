// Copyright Sam Bonifacio. All Rights Reserved.

#include "Misc/ScalabilitySettingControlRegistrar.h"

#include "Misc/ConsoleVariableSettingBindingStrategy.h"
#include "Framework/SettingRegistry.h"
#include "HAL/IConsoleManager.h"
#include "Misc/ConfigCacheIni.h"

FName GetCVarName(const FRegisteredSetting& Setting)
{
	UConsoleVariableSettingBindingStrategy* CVarStrategy = Cast<UConsoleVariableSettingBindingStrategy>(Setting.SettingType->BindingStrategy);
	if (!CVarStrategy)
	{
		return NAME_None;
	}

	return CVarStrategy->GetConsoleVariableName(Setting.Key);
}

void UScalabilitySettingControlRegistrar::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	USettingRegistry::OnCheckControlRelationship.AddWeakLambda(this, [this](const FRegisteredSetting& Setting, const FRegisteredSetting& ControlledSetting, USettingRegistry::FSettingControlResult& Result)
		{
			// Parse scalability control relationships from config if required
			if (!bHasParsedScalabilityControlGroups)
			{
				ParseScalabilityControlGroups();
			}

			FName SettingCVarName = GetCVarName(Setting);
			if (SettingCVarName.IsNone())
			{
				return;
			}

			auto FoundControlGroup = ScalabilityControlGroups.Find(SettingCVarName);
			if (!FoundControlGroup || FoundControlGroup->ControlledCVars.Num() == 0)
			{
				return;
			}

			FName ControlledCVarName = GetCVarName(ControlledSetting);
			if (ControlledCVarName.IsNone())
			{
				return;
			}

			if (FoundControlGroup->ControlledCVars.Contains(ControlledCVarName))
			{
				// Mark it as a controlled setting
				Result.SetControlled();
			}
		});
}

bool IsConsoleVariable(const FName Name)
{
	return IConsoleManager::Get().FindConsoleVariable(*Name.ToString()) != nullptr;
}

void UScalabilitySettingControlRegistrar::ParseScalabilityControlGroups()
{
	// Parse Unreal's scalability config to find which CVars are controlled by each scalability group
	// For example: sg.ShadowQuality controls r.ShadowQuality, r.Shadow.MaxResolution, etc.
	TArray<FString> Sections;
	GConfig->GetSectionNames(*GScalabilityIni, Sections);

	for (auto& SectionName : Sections)
	{
		TArray<FString> SectionNameSplit;
		SectionName.ParseIntoArray(SectionNameSplit, TEXT("@"), true);
		const FName SectionCVar = FName("sg." + SectionNameSplit[0]);

		if (!IsConsoleVariable(SectionCVar))
		{
			// Section doesn't correspond to a scalability group CVar (sg.*)
			continue;
		}

		const FConfigSection* Section = GConfig->GetSection(*SectionName, true, *GScalabilityIni);
		for (const auto& Value : *Section)
		{
			if (!IsConsoleVariable(Value.Key))
			{
				// Config entry is not a CVar
				continue;
			}
			// This scalability group controls this CVar
			ScalabilityControlGroups.FindOrAdd(SectionCVar).ControlledCVars.AddUnique(Value.Key);
		}
	}

	bHasParsedScalabilityControlGroups = true;
}
