#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EInventorySlotType : uint8
{
    Generic   UMETA(DisplayName = "Generic"),
    Weapon    UMETA(DisplayName = "Weapon"),
    Armor     UMETA(DisplayName = "Armor"),

};