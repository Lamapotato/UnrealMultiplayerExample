// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventorySlotType.h"
#include "InventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FSlotInfo
{
	GENERATED_BODY()

public:
	/** Class type for objects that can occupy this slot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	TSubclassOf<AActor> ObjectClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	EInventorySlotType ItemType = EInventorySlotType::Generic;

	/** Current number of objects in the slot (or max allowed). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	int32 ObjectCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	EInventorySlotType SlotType = EInventorySlotType::Generic;
};

USTRUCT(BlueprintType)
struct FSlotUIInfo : public FTableRowBase
{
	GENERATED_BODY()

public:

	/** Human‑readable name shown in UI. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FString DisplayName;

	/** Longer description or tooltip text. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FString Description;

	/** Icon that will be displayed next to the entry. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UTexture2D* Icon;
};



UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UME_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, ReplicatedUsing = OnRep_Slots)
	TArray<FSlotInfo> Slots;

	// OnRep function - C++ implementation
	UFUNCTION()
	void OnRep_Slots();

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnSlotsChanged(const TArray<FSlotInfo>& OldSlots, const TArray<FSlotInfo>& NewSlots);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Utility")
	bool SwapBetween(UInventoryComponent* CompA, int32 IndexA,
		UInventoryComponent* CompB, int32 IndexB);
private:
	bool IsValidIndex(int32 Idx, int32 Size);
};
