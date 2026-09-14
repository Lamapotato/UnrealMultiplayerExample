// Copyright Sam Bonifacio. All Rights Reserved.

#include "Framework/SettingPersistenceStrategy.h"
#include "AutoSettingsLogs.h"
#include "Framework/SettingTypes.h"
#include "Framework/SettingRegistry.h"

void USettingPersistenceStrategy::Save(const FSetSettingEvent& Event)
{
	const FString Previous = GetSavedValue(Event.Setting).Value;
	UE_LOG(LogAutoSettings, Log, TEXT("Saving setting %s with value: '%s', previous: '%s'"), *Event.Setting.Key.ToString(),
		*Event.Data.Value, *Previous);

	// Skip actual saving at design time to prevent side effects
	if (!Event.GetRegistry() || !Event.GetRegistry()->bDesignTime)
	{
		OnSave(Event);
	}

	if (Event.GetRegistry())
	{
		Event.GetRegistry()->OnSettingSaved.Broadcast(Event);
	}
}

FSettingData USettingPersistenceStrategy::GetSavedValue_Implementation(const FSettingReference& Setting) const
{
	return FSettingData();
}

bool USettingPersistenceStrategy::HasSavedValue(const FSettingReference& Setting) const
{
	return FindKeys().Contains(Setting.Key);
}

TArray<FFullSettingKey> USettingPersistenceStrategy::FindKeys_Implementation() const
{
	return {};
}

TArray<FFullSettingKey> USettingPersistenceStrategy::FindChildKeys(const FFullSettingKey& ParentKey)
{
	return FindKeys().FilterByPredicate([=](const FFullSettingKey& Key)
		{
			return Key.IsChildOf(ParentKey);
		});
}

void USettingPersistenceStrategy::OnSave_Implementation(const FSetSettingEvent& Event) {}