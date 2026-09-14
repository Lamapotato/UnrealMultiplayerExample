// Copyright Sam Bonifacio. All Rights Reserved.

#include "Utility/InputMappingFunctions.h"

#include "Utility/InputMappingUtils.h"

FName UInputMappingFunctions::GetKeyName(const FKey& Key)
{
	return Key.GetFName();
}

FText UInputMappingFunctions::GetMappingDisplayName(const FEnhancedActionKeyMapping& Mapping)
{
	return SB::AutoSettings::Input::Mapping::GetDisplayName(Mapping);
}
