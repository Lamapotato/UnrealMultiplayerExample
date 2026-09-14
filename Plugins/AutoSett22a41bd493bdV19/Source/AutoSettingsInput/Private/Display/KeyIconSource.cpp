// Copyright Sam Bonifacio. All Rights Reserved.

#include "Display/KeyIconSource.h"

#include "Engine/LocalPlayer.h"
#include "Styling/SlateBrush.h"

FSlateBrush UKeyIconSource::GetIconBrushForKey(const FKey& Key, const ULocalPlayer* Player) const
{
	return BP_GetIconBrushForKey(Key, Player->GetPlayerController(nullptr));
}

void UKeyIconSource::RegisterIconsChangedCallback(ULocalPlayer* Player, const FOnKeyIconsChanged& Callback) const
{
	return BP_RegisterIconsChangedCallback(Player->GetPlayerController(nullptr), Callback);
}

FSlateBrush UKeyIconSource::BP_GetIconBrushForKey_Implementation(const FKey& Key, APlayerController* Player) const
{
	return {};
}

void UKeyIconSource::BP_RegisterIconsChangedCallback_Implementation(APlayerController* Player,
	const FOnKeyIconsChanged& Callback) const
{
}