// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Engine/DataTable.h"
#include "UObject/NoExportTypes.h"
#include "InventoryItem.generated.h"

class UEquipmentSlotWidget;
class UInventorySlotWidget;
class UInventoryItem;
class AEquipmentItem;
class APickupItem;

USTRUCT(BlueprintType)
struct FInventoryItemDescription : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EInventoryItemType InventoryItemType = EInventoryItemType::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<APickupItem> PickUpItemClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<AEquipmentItem> EquipmentItemClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanStackItems = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bCanStackItems"))
	int32 MaxCount = 1;
};

USTRUCT(BlueprintType)
struct FInventoryTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<UInventoryItem> InventoryItemClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FInventoryItemDescription InventoryItemDescription;
};

UCLASS(NotBlueprintable)
class XYZHOMEWORK_API UInventoryItem : public UObject
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE(FOnCountChangedEvent);
	FOnCountChangedEvent OnCountChangedEvent;

	virtual bool IsSupportedForNetworking() const override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	void InitializeItem(const FInventoryItemDescription& InventoryItemDescription);
	const FInventoryItemDescription& GetItemDescription() const { return Description; }
	EInventoryItemType GetInventoryItemType() const { return Description.InventoryItemType; }
	TSoftClassPtr<AEquipmentItem> GetEquipmentItemClass() const { return Description.EquipmentItemClass; }
	bool CanStackItems() const { return Description.bCanStackItems; }
	int32 GetMaxCount() const { return Description.MaxCount; }
	bool IsEquipment() const { return bIsEquipment; }
	int32 GetCount() const { return Count; }
	void SetCount(int32 NewCount);
	int32 AddCount(int32 Value);
	void OnCountUpdated() const;
	int32 GetAvailableSpaceInStack() const;
	UInventorySlotWidget* GetPreviousInventorySlotWidget() const { return PreviousInventorySlotWidget.Get(); }
	void SetPreviousInventorySlotWidget(UInventorySlotWidget* SlotWidget);
	bool UpdatePreviousSlotWidget(UInventoryItem* NewData) const;
	virtual bool Consume(APawn* Pawn) { return false; }

protected:
	UPROPERTY(Replicated)
	FInventoryItemDescription Description;
	UPROPERTY(Replicated)
	bool bIsEquipment = false;
	UPROPERTY(ReplicatedUsing=OnRep_Count)
	int32 Count = 0;
	UFUNCTION()
	void OnRep_Count();
	TWeakObjectPtr<UInventorySlotWidget> PreviousInventorySlotWidget;
};
