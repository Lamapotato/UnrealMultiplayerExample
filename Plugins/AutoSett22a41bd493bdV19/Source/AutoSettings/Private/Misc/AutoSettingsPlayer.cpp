// Copyright Sam Bonifacio. All Rights Reserved.

#include "Misc/AutoSettingsPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"

FName IAutoSettingsPlayer::GetUniquePlayerIdentifier(const APlayerController* PlayerController)
{
	check(PlayerController);
	return PlayerController->Implements<UAutoSettingsPlayer>()
		? Execute_GetUniquePlayerIdentifier(PlayerController)
		: GetUniquePlayerIdentifier_Default(PlayerController);
}

FName IAutoSettingsPlayer::GetUniquePlayerIdentifier_Default(const APlayerController* PlayerController)
{
	check(PlayerController);
	check(PlayerController->GetLocalPlayer());
	return FName(FString::FromInt(PlayerController->GetLocalPlayer()->GetControllerId()));
}

FName IAutoSettingsPlayer::GetUniquePlayerIdentifier_Implementation() const
{
	const APlayerController* ThisPC = Cast<APlayerController>(this);
	return ThisPC ? GetUniquePlayerIdentifier_Default(ThisPC) : NAME_None;
}