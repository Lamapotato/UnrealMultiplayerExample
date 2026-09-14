// Copyright Sam Bonifacio. All Rights Reserved.

#include "Core/InputKeyFilter.h"

#include "Processing/InputModifierAxisDirection.h"

bool UInputKeyFilter::MatchesKey_Implementation(const FKey& Key)
{
	return true;
}

bool UInputKeyFilter_Gamepad::MatchesKey_Implementation(const FKey& Key)
{
	return Key.IsGamepadKey();
}

bool UInputKeyFilter_NonGamepad::MatchesKey_Implementation(const FKey& Key)
{
	return !Key.IsGamepadKey();
}
