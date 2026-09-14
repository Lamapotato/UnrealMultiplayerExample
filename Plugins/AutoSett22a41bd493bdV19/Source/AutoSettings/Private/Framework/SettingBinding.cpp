// Copyright Sam Bonifacio. All Rights Reserved.

#include "Framework/SettingBinding.h"

#include "AutoSettingsLogs.h"
#include "CoreGlobals.h"
#include "Framework/SettingBindingStrategy.h"
#include "Framework/SettingBindingTrigger.h"
#include "Framework/SettingRegistry.h"
#include "Logging/StructuredLog.h"

void USettingBinding::Initialize(USettingRegistry* InRegistry, const FSettingReference& InSetting, USettingBindingStrategy* InStrategy,
	const TArray<TObjectPtr<USettingBindingTrigger>>& TriggerTemplates)
{
	Registry = InRegistry;
	Setting = InSetting;
	Strategy = InStrategy;
	bIsActive = true;
	Triggers.Empty();

	for (USettingBindingTrigger* TriggerTemplate : TriggerTemplates)
	{
		if (!IsValid(TriggerTemplate))
		{
			continue;
		}

		USettingBindingTrigger* RuntimeTrigger = DuplicateObject<USettingBindingTrigger>(TriggerTemplate, this);
		if (ensure(RuntimeTrigger))
		{
			// Log the trigger clone so runtime debugging can distinguish the live trigger instance from
			// the design-time template owned by the strategy asset or setting CDO.
			UE_LOGFMT(LogAutoSettings, Verbose,
				"Created runtime trigger '{0}' from template '{1}' for setting '{2}'.",
				GetPathNameSafe(RuntimeTrigger),
				GetPathNameSafe(TriggerTemplate),
				Setting.Key.ToString());
			Triggers.Add(RuntimeTrigger);
		}
	}
}

void USettingBinding::Begin()
{
	if (!bIsActive || !Strategy)
	{
		return;
	}

	Strategy->BeginBinding(Setting, this);
}

void USettingBinding::End()
{
	if (!bIsActive)
	{
		return;
	}

	bIsActive = false;

	// Shutdown can deinitialize registries after transient Blueprint trigger objects have already been
	// marked unreachable, so avoid Blueprint teardown dispatch once the engine is exiting.
	if (Strategy && !IsEngineExitRequested() && !GExitPurge)
	{
		Strategy->EndBinding(Setting, this);
	}

	Triggers.Empty();
}

void USettingBinding::NotifyTargetValueChanged()
{
	if (!bIsActive || !Registry)
	{
		return;
	}

	Registry->HandleBindingTargetValueChanged(this);
}

void USettingBinding::NotifyBindingInvalidated()
{
	if (!bIsActive || !Registry)
	{
		return;
	}

	Registry->HandleBindingInvalidated(this);
}
