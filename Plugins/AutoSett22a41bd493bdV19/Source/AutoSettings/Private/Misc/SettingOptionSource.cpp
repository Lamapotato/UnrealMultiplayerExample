// Copyright Sam Bonifacio. All Rights Reserved.

#include "Misc/SettingOptionSource.h"
#include "AutoSettingsLogs.h"

TArray<FSettingOption> USettingOptionSource::GetOptions_Implementation() const
{
	UE_LOG(LogAutoSettings, Error, TEXT("Default ConstructOptions called, no options constructed"));
	return TArray<FSettingOption>();
}