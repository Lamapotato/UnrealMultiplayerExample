// Copyright Sam Bonifacio. All Rights Reserved.

#include "Framework/SettingDefaultProvider.h"

#include "AutoSettingsLogs.h"
#include "Utility/SettingKeyFunctions.h"

USettingDefaultProvider::USettingDefaultProvider()
{
	bEnabled = true;
}

TMap<FPartialSettingKey, FString> USettingDefaultProvider::GetDefaultValues_Implementation()
{
	// Default C++ implementation - Blueprint can override this
	return TMap<FPartialSettingKey, FString>();
}
