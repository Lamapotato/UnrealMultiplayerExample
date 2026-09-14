// Copyright Sam Bonifacio. All Rights Reserved.

#include "Utility/SettingKeyFunctions.h"
#include "Framework/SettingType.h"

FSettingKey USettingKeyFunctions::ParseSettingKey(const FString& String)
{
	return FSettingKey(String);
}

FPartialSettingKey USettingKeyFunctions::ParsePartialSettingKey(const FString& String)
{
	// Parse directly into the semantic key type now that the wrapper exposes explicit conversion constructors.
	return FPartialSettingKey(String);
}

FFullSettingKey USettingKeyFunctions::ParseFullSettingKey(const FString& String)
{
	// Parse directly into the semantic key type now that the wrapper exposes explicit conversion constructors.
	return FFullSettingKey(String);
}

FFullSettingKey USettingKeyFunctions::ConvertToFullSettingKey(const FSettingKey& SettingKey)
{
	return FFullSettingKey(SettingKey);
}

FPartialSettingKey USettingKeyFunctions::ConvertToPartialSettingKey(const FSettingKey& SettingKey)
{
	return FPartialSettingKey(SettingKey);
}

FPartialSettingKey USettingKeyFunctions::MakeSettingKey(const TSubclassOf<USettingType>& SettingType, const FPartialSettingKey& SubKey)
{
	// Blueprint version delegates to C++ implementation
	return SB::AutoSettings::SettingKeys::MakeSettingKey(SettingType, SubKey);
}
