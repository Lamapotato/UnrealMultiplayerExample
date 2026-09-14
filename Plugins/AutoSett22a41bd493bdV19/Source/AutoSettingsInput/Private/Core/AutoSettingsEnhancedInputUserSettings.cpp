// Copyright Sam Bonifacio. All Rights Reserved.

#include "Core/AutoSettingsEnhancedInputUserSettings.h"

#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Framework/AutoSettingsPlayerSubsystem.h"
#include "Utility/InputMappingUtils.h"

UAutoSettingsEnhancedInputUserSettings* UAutoSettingsEnhancedInputUserSettings::Get(const ULocalPlayer* LocalPlayer)
{
	const SB::AutoSettings::Input::FEnhancedInputSubsystemData Data = SB::AutoSettings::Input::GetEnhancedInputSubsystemData(LocalPlayer);
	return Data.bIsValid ? Data.UserSettings : nullptr;
}

void UAutoSettingsEnhancedInputUserSettings::OnKeyMappingUpdated(FPlayerKeyMapping* ChangedMapping,
	const FMapPlayerKeyArgs& InArgs, const bool bIsBeingUnmapped)
{
	Super::OnKeyMappingUpdated(ChangedMapping, InArgs, bIsBeingUnmapped);
	OnKeyMappingUpdatedEvent.Broadcast(*ChangedMapping);
}

void UAutoSettingsEnhancedInputUserSettings::SetMappingMetadata(const FName& MappingName, const FMappingMetadata& Metadata)
{
	MappingDataMap.Add(MappingName, Metadata);
}

FMappingMetadata UAutoSettingsEnhancedInputUserSettings::GetMappingMetadata(const FName& MappingName) const
{
	if (const FMappingMetadata* Found = MappingDataMap.Find(MappingName))
	{
		return *Found;
	}
	return FMappingMetadata(); // Return default metadata
}

bool UAutoSettingsEnhancedInputUserSettings::GetMappingNegateFlag(const FName& MappingName) const
{
	return GetMappingMetadata(MappingName).bNegate;
}

void UAutoSettingsEnhancedInputUserSettings::ClearMappingMetadata(const FName& MappingName)
{
	MappingDataMap.Remove(MappingName);
}