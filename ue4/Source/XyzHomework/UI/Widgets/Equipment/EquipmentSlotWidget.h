// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Blueprint/UserWidget.h"
#include "EquipmentSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UInventorySlotWidget;
class AEquipmentItem;
class UInventoryItem;

/** Widget associated with a slot in the character equipment. */
UCLASS()
class XYZHOMEWORK_API UEquipmentSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DELEGATE_ThreeParams(FOnEquipmentDroppedInSlot, EInventoryItemType, int32, int32);
	DECLARE_DELEGATE_OneParam(FOnEquipmentRemovedFromSlot, int32);
	DECLARE_DELEGATE_OneParam(FOnSlotUpdated, int32)
	FOnEquipmentDroppedInSlot OnEquipmentDroppedInSlot;
	FOnEquipmentRemovedFromSlot OnEquipmentRemovedFromSlot;
	FOnSlotUpdated OnSlotUpdated;

	void InitializeSlot(AEquipmentItem* EquipmentItem, int32 SlotIndex);
	void UpdateView() const;
	/**
	 * Links this slot widget with a new inventory item. If 'NewItem' != nullptr, a new 'LinkedEquipmentItem' will be added to the character equipment.
	 * Otherwise, an item stored in the 'SlotIndexInComponent' equipment slot will be removed from the character equipment.
	 */
	void SetLinkedSlotItem(UInventoryItem* NewItem) const;

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemName;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SlotName;

private:
	/** Equipment item that is associated with this slot widget. */
	TWeakObjectPtr<AEquipmentItem> LinkedEquipmentItem;
	/** Inventory item that is associated with the equipment item stored in the 'SlotIndexInComponent' equipment slot. */
	TWeakObjectPtr<UInventoryItem> CachedInventoryItem;
	/** Equipment slot that is associated with this slot widget. */
	int32 SlotIndexInComponent = 0;
};
