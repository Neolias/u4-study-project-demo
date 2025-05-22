// Fill out your copyright notice in the Description page of Project Settings.

#include "InventorySlotWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CharacterComponents/CharacterInventoryComponent.h"
#include "Inventory/Items/InventoryItem.h"

void UInventorySlotWidget::InitializeSlot(UInventorySlot* InventorySlot)
{
	if (IsValid(InventorySlot))
	{
		LinkedSlot = InventorySlot;
		CachedInventoryItem = InventorySlot->GetLinkedInventoryItem();
	}
	else
	{
		CachedInventoryItem.Reset();
		LinkedSlot.Reset();
	}

	UInventorySlot* LinkedInventorySlot = LinkedSlot.Get();
	if (IsValid(LinkedInventorySlot))
	{
		LinkedInventorySlot->OnSlotUpdatedEvent.BindUObject(this, &UInventorySlotWidget::UpdateView);
	}
}

void UInventorySlotWidget::UpdateView()
{
	if (!ItemIcon || !ItemCount)
	{
		return;
	}

	const UInventorySlot* InventorySlot = LinkedSlot.Get();
	if (!InventorySlot)
	{
		ItemIcon->SetBrushFromTexture(nullptr);
		ItemCount->SetText(FText::GetEmpty());
		return;
	}

	UInventoryItem* InventoryItem = InventorySlot->GetLinkedInventoryItem();
	CachedInventoryItem = InventoryItem;
	if (IsValid(InventoryItem))
	{
		ItemIcon->SetBrushFromTexture(InventoryItem->GetItemDescription().Icon);
		if (InventoryItem->CanStackItems())
		{
			FText CountText = FText::FromString(FString::FromInt(InventoryItem->GetCount()));
			ItemCount->SetText(CountText);
		}
		else
		{
			ItemCount->SetText(FText::GetEmpty());
		}
	}
	else
	{
		ItemIcon->SetBrushFromTexture(nullptr);
		ItemCount->SetText(FText::GetEmpty());
	}
}

void UInventorySlotWidget::UpdateItemIconAndCount(UInventoryItem* NewItemData)
{
	if (!IsValid(NewItemData))
	{
		UpdateView();
		return;
	}

	if (!ItemCount)
	{
		return;
	}

	ItemIcon->SetBrushFromTexture(NewItemData->GetItemDescription().Icon);
	if (NewItemData->CanStackItems())
	{
		FText CountText = FText::FromString(FString::FromInt(NewItemData->GetCount()));
		ItemCount->SetText(CountText);
	}
	else
	{
		ItemCount->SetText(FText::GetEmpty());
	}
}

void UInventorySlotWidget::SetItemIcon(UTexture2D* Icon) const
{
	if (ItemIcon)
	{
		ItemIcon->SetBrushFromTexture(Icon);
	}
}

void UInventorySlotWidget::SetLinkedSlotItem(UInventoryItem* NewItem)
{
	NewItem = IsValid(NewItem) && NewItem->GetCount() > 0 ? NewItem : nullptr;
	if (NewItem)
	{
		NewItem->SetPreviousInventorySlotWidget(this);
	}

	UInventorySlot* InventorySlot = LinkedSlot.Get();
	if (InventorySlot)
	{
		InventorySlot->SetLinkedInventoryItem(NewItem);
	}
}

FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	UInventoryItem* InventoryItem = CachedInventoryItem.Get();
	if (!IsValid(InventoryItem))
	{
		return FReply::Handled();
	}

	FKey MouseBtn = InMouseEvent.GetEffectingButton();
	if (MouseBtn == EKeys::RightMouseButton)
	{
		/* Some simplification, so as not to complicate the architecture
		 * - on instancing item, we use the current pawn as an outer one.
		 * In real practice we need use callback for inform item holder what action was do in UI */

		InventoryItem->SetPreviousInventorySlotWidget(this);
		AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(InventoryItem->GetOuter());
		if (IsValid(BaseCharacter))
		{
			BaseCharacter->Server_UseItem(InventoryItem, LinkedSlot.Get());
		}

		return FReply::Handled();
	}

	FEventReply Reply = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton);
	return Reply.NativeReply;
}

void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	UDragDropOperation* DragOperation = Cast<UDragDropOperation>(UWidgetBlueprintLibrary::CreateDragDropOperation(UDragDropOperation::StaticClass()));

	/* Some simplification for not define new widget for drag and drop operation  */
	UInventorySlotWidget* DragWidget = CreateWidget<UInventorySlotWidget>(GetOwningPlayer(), GetClass());
	UInventoryItem* InventoryItem = CachedInventoryItem.Get();
	DragWidget->UpdateItemIconAndCount(InventoryItem);

	DragOperation->DefaultDragVisual = DragWidget;
	DragOperation->Pivot = EDragPivot::MouseDown;
	if (IsValid(InventoryItem))
	{
		InventoryItem->SetPreviousInventorySlotWidget(this);
		DragOperation->Payload = InventoryItem;
	}
	OutOperation = DragOperation;

	UInventorySlot* InventorySlot = LinkedSlot.Get();
	if (InventorySlot)
	{
		InventorySlot->ClearSlot();
	}
}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UInventoryItem* PayloadItem = Cast<UInventoryItem>(InOperation->Payload);
	if (!IsValid(PayloadItem))
	{
		return false;
	}

	const UInventoryItem* InventoryItem = CachedInventoryItem.Get();
	if (!IsValid(InventoryItem))
	{
		SetLinkedSlotItem(PayloadItem);
		return true;
	}

	bool bCanStackItems = InventoryItem->CanStackItems() && InventoryItem->GetInventoryItemType() == PayloadItem->GetInventoryItemType();
	if (!bCanStackItems)
	{
		return SwapSlotItems(PayloadItem);
	}

	if (InventoryItem->GetAvailableSpaceInStack() == 0)
	{
		return PayloadItem->UpdatePreviousSlotWidget(PayloadItem);
	}

	const AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(InventoryItem->GetOuter());
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->GetCharacterInventoryComponent()->Server_StackItems(LinkedSlot.Get(), PayloadItem->GetInventoryItemType(), PayloadItem->GetCount());
		return true;
	}

	return false;
}

void UInventorySlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	const UInventoryItem* PayloadItem = Cast<UInventoryItem>(InOperation->Payload);
	if (!IsValid(PayloadItem))
	{
		return;
	}

	AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(PayloadItem->GetOuter());
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->Server_DropItem(PayloadItem->GetInventoryItemType(), PayloadItem->GetCount());
	}
}

bool UInventorySlotWidget::SwapSlotItems(UInventoryItem* OtherSlotItem)
{
	if (!IsValid(OtherSlotItem))
	{
		return false;
	}

	if (OtherSlotItem->UpdatePreviousSlotWidget(CachedInventoryItem.Get()))
	{
		SetLinkedSlotItem(OtherSlotItem);
		return true;
	}

	OtherSlotItem->UpdatePreviousSlotWidget(OtherSlotItem);
	return false;
}
