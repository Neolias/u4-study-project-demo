// Fill out your copyright notice in the Description page of Project Settings.

#include "EquipmentSlotWidget.h"

#include "Actors/Equipment/EquipmentItem.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "Inventory/Items/InventoryItem.h"

void UEquipmentSlotWidget::InitializeSlot(AEquipmentItem* EquipmentItem, int32 SlotIndex)
{
	if (IsValid(EquipmentItem))
	{
		LinkedEquipmentItem = EquipmentItem;
		CachedInventoryItem = EquipmentItem->GetLinkedInventoryItem();
	}
	else
	{
		CachedInventoryItem.Reset();
		LinkedEquipmentItem.Reset();
	}

	if (SlotIndex >= 0 && SlotIndex < (uint32)EEquipmentItemSlot::Max)
	{
		SlotIndexInComponent = SlotIndex;
		SlotName->SetText(UEnum::GetDisplayValueAsText<EEquipmentItemSlot>((EEquipmentItemSlot)SlotIndex));
	}
	else
	{
		SlotIndexInComponent = 0;
		SlotName->SetText(FText::FromName(NAME_None));
	}
}

void UEquipmentSlotWidget::UpdateView() const
{
	const UInventoryItem* InventoryItem = CachedInventoryItem.Get();
	if (IsValid(InventoryItem))
	{
		ItemIcon->SetBrushFromTexture(InventoryItem->GetItemDescription().Icon);
		ItemName->SetText(InventoryItem->GetItemDescription().Name);
	}
	else
	{
		ItemIcon->SetBrushFromTexture(nullptr);
		ItemName->SetText(FText::FromName(NAME_None));
	}

	OnSlotUpdated.ExecuteIfBound(SlotIndexInComponent);
}

void UEquipmentSlotWidget::SetLinkedSlotItem(UInventoryItem* NewItem) const
{
	if (IsValid(NewItem))
	{
		OnEquipmentDroppedInSlot.ExecuteIfBound(NewItem->GetInventoryItemType(), NewItem->GetCount(), SlotIndexInComponent);
	}
	else
	{
		OnEquipmentRemovedFromSlot.ExecuteIfBound(SlotIndexInComponent);
	}
}

FReply UEquipmentSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	const UInventoryItem* InventoryItem = CachedInventoryItem.Get();
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

		const AEquipmentItem* EquipmentItem = Cast<AEquipmentItem>(InventoryItem->GetOuter());
		if (IsValid(EquipmentItem))
		{
			AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(EquipmentItem->GetOwner());
			if (IsValid(BaseCharacter))
			{
				BaseCharacter->Server_RemoveEquipmentItem(SlotIndexInComponent);
			}
		}
	}

	return FReply::Handled();
}

bool UEquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UInventoryItem* PayloadItem = Cast<UInventoryItem>(InOperation->Payload);
	if (!IsValid(PayloadItem))
	{
		return false;
	}

	if (!PayloadItem->IsEquipment())
	{
		return PayloadItem->UpdatePreviousSlotWidget(PayloadItem);
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
		SetLinkedSlotItem(PayloadItem);
		return true;
	}

	if (InventoryItem->GetAvailableSpaceInStack() == 0)
	{
		return PayloadItem->UpdatePreviousSlotWidget(PayloadItem);
	}

	const AEquipmentItem* EquipmentItem = Cast<AEquipmentItem>(InventoryItem->GetOuter());
	if (IsValid(EquipmentItem))
	{
		const AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(EquipmentItem->GetOwner());
		if (IsValid(BaseCharacter))
		{
			BaseCharacter->GetCharacterEquipmentComponent()->Server_StackEquipmentItems(SlotIndexInComponent, InventoryItem->GetInventoryItemType(), PayloadItem->GetCount());
			return true;
		}
	}

	return false;
}
