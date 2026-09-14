// Copyright Sam Bonifacio. All Rights Reserved.

#include "Framework/SettingBindingStrategy.h"
#include "AutoSettingsError.h"
#include "AutoSettingsLogs.h"
#include "Framework/SettingBinding.h"
#include "Framework/SettingRegistry.h"
#include "Framework/SettingTypes.h"
#include "Logging/StructuredLog.h"

bool USettingBindingStrategy::IsBindingReady_Implementation(const FSettingContext& Context)
{
	return true;
}

FString USettingBindingStrategy::GetTargetValue_Implementation(const FSettingReference& Setting)
{
	return TEXT("");
}

void USettingBindingStrategy::BeginBinding_Implementation(const FSettingReference& Setting, USettingBinding* Binding)
{
	if (!Binding)
	{
		return;
	}

	for (USettingBindingTrigger* Trigger : Binding->GetTriggers())
	{
		if (IsValid(Trigger))
		{
			// Log the exact runtime trigger object being activated so Blueprint debugging can verify the
			// live duplicate that is wired into the current binding rather than the template object.
			UE_LOGFMT(LogAutoSettings, Verbose,
				"Activating trigger '{0}' for setting '{1}' on binding '{2}'.",
				GetPathNameSafe(Trigger),
				Setting.Key.ToString(),
				GetPathNameSafe(Binding));
			Trigger->Activate(Setting, Binding);
		}
	}
}

void USettingBindingStrategy::EndBinding_Implementation(const FSettingReference& Setting, USettingBinding* Binding)
{
	if (!Binding)
	{
		return;
	}

	for (USettingBindingTrigger* Trigger : Binding->GetTriggers())
	{
		if (IsValid(Trigger))
		{
			// Match activation logging during teardown so trigger lifetimes are visible in the same log
			// stream when bindings invalidate and rebind.
			UE_LOGFMT(LogAutoSettings, Verbose,
				"Deactivating trigger '{0}' for setting '{1}' on binding '{2}'.",
				GetPathNameSafe(Trigger),
				Setting.Key.ToString(),
				GetPathNameSafe(Binding));
			Trigger->Deactivate(Setting, Binding);
		}
	}
}

void USettingBindingStrategy::SyncToTarget(const FSetSettingEvent& Event)
{
	if (!ensure(Event.GetRegistry()))
	{
		return;
	}

	if (!ValidateConfiguration(Event.Setting))
	{
		// Apply-time validation remains strict as a final guard for any path that reaches
		// application, including direct SetSetting calls after registration.
		SB::AutoSettings::Errors::LogError(FString::Printf(TEXT("Applying setting '%s': Configuration is invalid for binding strategy '%s'."),
			*Event.Setting.Key.ToString(), *GetClass()->GetName()));
		return;
	}

	if (!IsBindingReady(Event.Setting.Context))
	{
		UE_LOG(LogAutoSettings, Verbose, TEXT("Skipping apply for setting %s because target binding is not ready for strategy '%s'."),
			*Event.Setting.Key.ToString(), *GetClass()->GetName());
		return;
	}

	const FString Previous = GetTargetValue(Event.Setting);
	UE_LOG(LogAutoSettings, Log, TEXT("Synchronizing setting %s to target with value: '%s', previous: '%s'"), *Event.Setting.Key.ToString(),
		*Event.Data.Value, *Previous);

	// Design-time previews still need value reads, but should never push changes into live targets.
	if (!Event.GetRegistry()->bDesignTime)
	{
		OnSyncToTarget(Event);
	}
}

bool USettingBindingStrategy::ValidateConfiguration_Implementation(const FSettingReference& Setting)
{
	return true;
}

void USettingBindingStrategy::OnSyncToTarget_Implementation(const FSetSettingEvent& Event) {}
