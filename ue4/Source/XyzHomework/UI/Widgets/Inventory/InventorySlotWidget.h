// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySlotWidget.generated.h"

class UInventorySlot;
class UTextBlock;
class UInventoryItem;
class UImage;

UCLASS()
class XYZHOMEWORK_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeSlot(UInventorySlot* InventorySlot);
	void UpdateView();
	void UpdateItemIconAndCount(UInventoryItem* NewItemData);
	void SetItemIcon(UTexture2D* Icon) const;
	void SetLinkedSlotItem(UInventoryItem* NewItem);
	UInventorySlot* GetLinkedSlot() const { return LinkedSlot.Get(); }

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemCount;

private:
	bool SwapSlotItems(UInventoryItem* OtherSlotItem);
	
	TWeakObjectPtr<UInventorySlot> LinkedSlot;
	TWeakObjectPtr<UInventoryItem> CachedInventoryItem;
};
