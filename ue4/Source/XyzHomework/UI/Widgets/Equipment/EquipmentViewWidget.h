// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Blueprint/UserWidget.h"
#include "EquipmentViewWidget.generated.h"

class AEquipmentItem;
class AXyzBaseCharacter;
class UGridPanel;
class UEquipmentSlotWidget;

/** Widget visualizing the character equipment. */
UCLASS()
class XYZHOMEWORK_API UEquipmentViewWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeWidget(AXyzBaseCharacter* BaseCharacter);
	void UpdateSlot(int32 SlotIndex) const;
	void UpdateAllSlots() const;
	UEquipmentSlotWidget* GetEquipmentSlotWidget(int32 SlotIndex) const;

protected:
	void AddSlotToView(AEquipmentItem* EquipmentItem, int32 SlotIndex);
	/** Instructs the character equipment component to equip 'Amount' items of 'ItemType' type into the 'SlotIndex' equipment slot. */
	void EquipItem(EInventoryItemType ItemType, int32 Amount, int32 SlotIndex);
	/** Instructs the character equipment component to unequip an item stored in the 'SlotIndex' equipment slot. */
	void UnequipItem(int32 SlotIndex);
	void OnSlotUpdated(int32 SlotIndex);

	/** All equipment slot widgets owned by this widget. */
	UPROPERTY(meta = (BindWidget))
	UGridPanel* ItemSlots;
	/** Widget class used to visualize a thumbnail when dragging items between slots. */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment View Widget")
	TSoftClassPtr<UEquipmentSlotWidget> DefaultSlotViewClass;
	
	TWeakObjectPtr<AXyzBaseCharacter> CachedBaseCharacter;
};
