// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySlotWidget.generated.h"

class UInventorySlot;
class UTextBlock;
class UInventoryItem;
class UImage;

/** Widget associated with a slot in the character inventory. */
UCLASS()
class XYZHOMEWORK_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeSlot(UInventorySlot* InventorySlot);
	void UpdateView();
	void UpdateItemIconAndCount(UInventoryItem* NewItemData);
	void SetItemIcon(UTexture2D* Icon) const;
	/** Links this slot widget with a new inventory item and updates the previous inventory slot if needed. */
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

	/** Inventory slot that is associated with this slot widget. */
	TWeakObjectPtr<UInventorySlot> LinkedSlot;
	TWeakObjectPtr<UInventoryItem> CachedInventoryItem;
};
