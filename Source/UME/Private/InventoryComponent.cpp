// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
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

	// Сохраняем временно элемент из первого инвентаря
	FSlotInfo Temp = CompA->Slots[IndexA];

	// Переносим второй в первый
	CompA->Slots[IndexA] = CompB->Slots[IndexB];
	// И первый во второй
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