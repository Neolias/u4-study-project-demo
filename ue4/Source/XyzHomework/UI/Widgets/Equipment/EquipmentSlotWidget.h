// Fill out your copyright notice in the Description page of Project Settings.

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
	TWeakObjectPtr<AEquipmentItem> LinkedEquipmentItem;
	TWeakObjectPtr<UInventoryItem> CachedInventoryItem;
	int32 SlotIndexInComponent = 0;
};
