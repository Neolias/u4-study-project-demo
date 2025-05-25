// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

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

/** Base class of all inventory items, such as consumables, weapons, ammo, etc., that can be stored in the character inventory. */
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
	/** Can this item be stacked with another item of the same type? */
	bool CanStackItems() const { return Description.bCanStackItems; }
	/** Returns the maximum size of a stack of items this object can represent. */
	int32 GetMaxCount() const { return Description.MaxCount; }
	bool IsEquipment() const { return bIsEquipment; }
	/** Returns the current size of a stack of items this object represents. */
	int32 GetCount() const { return Count; }
	/** Sets a new size of a stack of items this object represents. */
	void SetCount(int32 NewCount);
	/** Updates the size of a stack of items this object represents by adding 'Value'. */
	int32 AddCount(int32 Value);
	void OnCountUpdated() const;
	int32 GetAvailableSpaceInStack() const;
	/** Returns the latest inventory slot in which this item was stored in. */
	UInventorySlotWidget* GetPreviousInventorySlotWidget() const { return PreviousInventorySlotWidget.Get(); }
	/** Sets a new inventory slot in which this item was stored in. */
	void SetPreviousInventorySlotWidget(UInventorySlotWidget* SlotWidget);
	/** Updates the latest inventory slot in which this item was stored in. */
	bool UpdatePreviousSlotWidget(UInventoryItem* NewData) const;
	virtual bool Consume(APawn* Pawn) { return false; }

protected:
	UPROPERTY(Replicated)
	FInventoryItemDescription Description;
	UPROPERTY(Replicated)
	bool bIsEquipment = false;
	/** Size of a stack of items this object represents. */
	UPROPERTY(ReplicatedUsing=OnRep_Count)
	int32 Count = 0;
	UFUNCTION()
	void OnRep_Count();
	/** Latest inventory slot in which this item was stored in. */
	TWeakObjectPtr<UInventorySlotWidget> PreviousInventorySlotWidget;
};
