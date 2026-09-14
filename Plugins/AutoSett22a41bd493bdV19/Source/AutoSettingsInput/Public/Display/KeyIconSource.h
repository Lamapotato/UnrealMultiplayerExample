// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "KeyIconSource.generated.h"

struct FSlateBrush;
struct FKey;
class ULocalPlayer;

DECLARE_DYNAMIC_DELEGATE(FOnKeyIconsChanged);

/**
 * Class responsible for looking up the icon for a specified key to show in UI
 * Can be subclassed to expose different icon sources
 */
UCLASS(Abstract, DefaultToInstanced, EditInlineNew, Blueprintable)
class AUTOSETTINGSINPUT_API UKeyIconSource : public UObject
{
	GENERATED_BODY()

public:
	virtual FSlateBrush GetIconBrushForKey(const FKey& Key, const ULocalPlayer* Player) const;

	virtual void RegisterIconsChangedCallback(ULocalPlayer* Player, const FOnKeyIconsChanged& Callback) const;

protected:
	UFUNCTION(BlueprintNativeEvent)
	FSlateBrush BP_GetIconBrushForKey(const FKey& Key, APlayerController* Player) const;

	UFUNCTION(BlueprintNativeEvent)
	void BP_RegisterIconsChangedCallback(APlayerController* Player, const FOnKeyIconsChanged& Callback) const;
};