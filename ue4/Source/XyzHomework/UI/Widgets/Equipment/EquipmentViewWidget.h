// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Blueprint/UserWidget.h"
#include "EquipmentViewWidget.generated.h"

class AXyzBaseCharacter;
class UGridPanel;
class UEquipmentSlotWidget;
class AEquipmentItem;

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
	void EquipItem(EInventoryItemType ItemType, int32 Amount, int32 SenderIndex);
	void UnequipItem(int32 SlotIndex);
	void OnSlotUpdated(int32 SlotIndex);

	UPROPERTY(meta = (BindWidget))
	UGridPanel* ItemSlots;
	UPROPERTY(EditDefaultsOnly, Category = "ItemContainer View Settings")
	TSoftClassPtr<UEquipmentSlotWidget> DefaultSlotViewClass;
	
	TWeakObjectPtr<AXyzBaseCharacter> CachedBaseCharacter;
};
