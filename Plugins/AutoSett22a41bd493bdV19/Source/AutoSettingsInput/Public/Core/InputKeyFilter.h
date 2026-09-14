// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InputKeyFilter.generated.h"

/**
 *
 */
UCLASS(Abstract, DefaultToInstanced, EditInlineNew)
class AUTOSETTINGSINPUT_API UInputKeyFilter : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	bool MatchesKey(const FKey& Key);
};

UCLASS()
class AUTOSETTINGSINPUT_API UInputKeyFilter_Gamepad : public UInputKeyFilter
{
	GENERATED_BODY()

protected:
	virtual bool MatchesKey_Implementation(const FKey& Key) override;
};

UCLASS()
class AUTOSETTINGSINPUT_API UInputKeyFilter_NonGamepad : public UInputKeyFilter
{
	GENERATED_BODY()

protected:
	virtual bool MatchesKey_Implementation(const FKey& Key) override;
};
