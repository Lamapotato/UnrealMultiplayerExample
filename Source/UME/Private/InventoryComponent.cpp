// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Replicate Slots array
    DOREPLIFETIME_CONDITION(UInventoryComponent, Slots, COND_None);
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UInventoryComponent::OnRep_Slots()
{
    static TArray<FSlotInfo> OldSlots;
    TArray<FSlotInfo> NewSlots = Slots;

    OnSlotsChanged(OldSlots, NewSlots);

    OldSlots = NewSlots;
}

bool UInventoryComponent::SwapBetween(UInventoryComponent* CompA, int32 IndexA,
    UInventoryComponent* CompB, int32 IndexB)
{
    if (!CompA || !CompB) return false;

    const int32 SizeA = CompA->Slots.Num();
    const int32 SizeB = CompB->Slots.Num();

    if (!IsValidIndex(IndexA, SizeA) || !IsValidIndex(IndexB, SizeB))
    {
#if WITH_EDITOR
        UE_LOG(LogTemp, Warning,
            TEXT("UInventoryComponent::SwapBetween: invalid indices (%d,%d) for sizes %d/%d"),
            IndexA, IndexB, SizeA, SizeB);
#endif
        return false;
    }

    const FSlotInfo& SlotA = CompA->Slots[IndexA];
    const FSlotInfo& SlotB = CompB->Slots[IndexB];

    // Lambda для проверки совместимости
    auto IsCompatible = [](EInventorySlotType ItemType, EInventorySlotType SlotType, TSubclassOf<AActor> ObjectClass) -> bool
        {
            // Если класса нет - игнорируем тип предмета (слот считается пустым)
            if (!ObjectClass)
            {
                return true;
            }

            // Generic слот принимает любые предметы
            if (SlotType == EInventorySlotType::Generic)
            {
                return true;
            }

            // Иначе типы должны совпадать
            return ItemType == SlotType;
        };

    // Проверяем совместимость в обе стороны
    if (!IsCompatible(SlotB.ItemType, SlotA.SlotType, SlotB.ObjectClass) ||
        !IsCompatible(SlotA.ItemType, SlotB.SlotType, SlotA.ObjectClass))
    {
#if WITH_EDITOR
        UE_LOG(LogTemp, Warning,
            TEXT("UInventoryComponent::SwapBetween: incompatible types - SlotA(%s) vs ItemB(%s) | SlotB(%s) vs ItemA(%s)"),
            *UEnum::GetValueAsString(SlotA.SlotType),
            *UEnum::GetValueAsString(SlotB.ItemType),
            *UEnum::GetValueAsString(SlotB.SlotType),
            *UEnum::GetValueAsString(SlotA.ItemType));
#endif
        return false;
    }

    // Выполняем swap
    FSlotInfo Temp = CompA->Slots[IndexA];
    CompA->Slots[IndexA] = CompB->Slots[IndexB];
    CompB->Slots[IndexB] = Temp;

#if WITH_EDITOR
    UE_LOG(LogTemp, Log,
        TEXT("UInventoryComponent::SwapBetween: swapped [%s:%d] <-> [%s:%d]"),
        *CompA->GetName(), IndexA, *CompB->GetName(), IndexB);
#endif

    return true;
}

bool UInventoryComponent::IsValidIndex(int32 Idx, int32 Size)
{
	return (Idx >= 0 && Idx < Size);
}