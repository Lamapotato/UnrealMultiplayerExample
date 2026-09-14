// Copyright Sam Bonifacio. All Rights Reserved.

#include "CommonKeyIconSource.h"

#include "CommonInputSubsystem.h"
#include "CommonInputBaseTypes.h"

ECommonInputType GetInputType(const FKey& Key)
{
	if (Key.IsGamepadKey())
	{
		return ECommonInputType::Gamepad;
	}

	return ECommonInputType::MouseAndKeyboard;
}

FName GetGamepadName(const UCommonInputSubsystem* Subsystem)
{
	// Get the currently applied GamepadName from the subsystem
	const FName SubsystemCurrentGamepadName = Subsystem->GetCurrentGamepadName();

	// It's possible the subsystem GamepadName isn't actually a gamepad, CommonInput does not validate the state unfortunately
	// So we'll check here that it's actually a gamepad, and pick a different one from available ControllerData if it's not

	TArray<FName> GamepadNames;
	for (const auto& ControllerDataClass : UCommonInputPlatformSettings::Get()->GetControllerData())
	{
		TSubclassOf<UCommonInputBaseControllerData> ControllerData = ControllerDataClass.LoadSynchronous();
		if (!IsValid(ControllerData))
		{
			continue;
		}
		const UCommonInputBaseControllerData* CDO = GetDefault<UCommonInputBaseControllerData>(ControllerData);
		if (CDO->InputType == ECommonInputType::Gamepad)
		{
			FName ThisGamepadName = CDO->GamepadName;
			if (ThisGamepadName == SubsystemCurrentGamepadName)
			{
				// We found it, break and return
				return SubsystemCurrentGamepadName;
			}
			// Remember the name so that we can use it if we don't find the Subsystem gamepad name
			GamepadNames.Add(ThisGamepadName);
		}
	}

	if (GamepadNames.Num() > 0)
	{
		// We didn't find the subsystem gamepad name, so we'll assume that it's not actually a gamepad and pick something else
		return GamepadNames[0];
	}

	// Go with the subsystem gamepad name after all as we didn't find anything better
	return SubsystemCurrentGamepadName;
}

FName GetGamepadName(const ULocalPlayer* LocalPlayer)
{
	if (LocalPlayer)
	{
		// If there is a local player, get the gamepad name they are using
		return GetGamepadName(UCommonInputSubsystem::Get(LocalPlayer));
	}

	// Use the default gamepad name
	return UCommonInputPlatformSettings::Get()->GetDefaultGamepadName();
}

FSlateBrush UCommonKeyIconSource::GetIconBrushForKey(const FKey& Key, const ULocalPlayer* Player) const
{
	FSlateBrush SlateBrush;
	if (Key.IsValid() && UCommonInputPlatformSettings::Get()->TryGetInputBrush(SlateBrush, Key, GetInputType(Key), GetGamepadName(Player)))
	{
		return SlateBrush;
	}

	return {};
}

void UCommonKeyIconSource::RegisterIconsChangedCallback(ULocalPlayer* Player, const FOnKeyIconsChanged& Callback) const
{
	UCommonInputSubsystem::Get(Player)->OnInputMethodChangedNative.AddWeakLambda(Callback.GetUObject(), [=](ECommonInputType InputType)
		{
			bool bSuccess = Callback.ExecuteIfBound();
		});
}
