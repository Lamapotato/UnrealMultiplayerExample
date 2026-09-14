// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AutoSettingsInput/Public/Display/KeyIconSource.h"
#include "CommonKeyIconSource.generated.h"

/**
 * KeyIconSource which uses CommonInput to look up key icons
 */
UCLASS()
class AUTOSETTINGSCOMMONUI_API UCommonKeyIconSource : public UKeyIconSource
{
	GENERATED_BODY()

public:
	virtual FSlateBrush GetIconBrushForKey(const FKey& Key, const ULocalPlayer* Player) const override;

	virtual void RegisterIconsChangedCallback(ULocalPlayer* Player, const FOnKeyIconsChanged& Callback) const override;
};