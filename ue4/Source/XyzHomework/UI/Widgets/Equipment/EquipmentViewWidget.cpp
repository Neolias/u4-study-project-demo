// Fill out your copyright notice in the Description page of Project Settings.

#include "EquipmentViewWidget.h"

#include "EquipmentSlotWidget.h"
#include "Actors/Equipment/EquipmentItem.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/GridPanel.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

void UEquipmentViewWidget::InitializeWidget(AXyzBaseCharacter* BaseCharacter)
{
	if (IsValid(BaseCharacter))
	{
		CachedBaseCharacter = BaseCharacter;
	}
	else
	{
		CachedBaseCharacter.Reset();
		return;
	}
	const TArray<AEquipmentItem*>& Items = BaseCharacter->GetCharacterEquipmentComponent()->GetEquippedItemsArray();
	/* We skip "none" slot*/
	for (int32 Index = 1; Index < Items.Num(); ++Index)
	{
		AddSlotToView(Items[Index], Index);
	}
}

void UEquipmentViewWidget::UpdateSlot(int32 SlotIndex) const
{
	UEquipmentSlotWidget* WidgetToUpdate = GetEquipmentSlotWidget(SlotIndex);
	if (IsValid(WidgetToUpdate))
	{
		const AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
		if (!IsValid(BaseCharacter))
		{
			return;
		}

		WidgetToUpdate->InitializeSlot(BaseCharacter->GetCharacterEquipmentComponent()->GetEquippedItem(SlotIndex), SlotIndex);
		WidgetToUpdate->UpdateView();
	}
}

void UEquipmentViewWidget::UpdateAllSlots() const
{
	/* We skip "none" slot*/
	for (int32 Index = 1; Index < (int32)EEquipmentItemSlot::Max; ++Index)
	{
		UpdateSlot(Index);
	}
}

UEquipmentSlotWidget* UEquipmentViewWidget::GetEquipmentSlotWidget(int32 SlotIndex) const
{
	if (!ItemSlots || SlotIndex < 1 || SlotIndex - 1 >= ItemSlots->GetChildrenCount())
	{
		return nullptr;
	}

	return StaticCast<UEquipmentSlotWidget*>(ItemSlots->GetChildAt(SlotIndex - 1));
}

void UEquipmentViewWidget::AddSlotToView(AEquipmentItem* EquipmentItem, int32 SlotIndex)
{
	checkf(DefaultSlotViewClass.LoadSynchronous() != nullptr, TEXT("UEquipmentViewWidget::AddSlotToView(): DefaultSlotViewClass is undefined."));

	if (!ItemSlots)
	{
		return;
	}
	
	if (UEquipmentSlotWidget* SlotWidget = CreateWidget<UEquipmentSlotWidget>(this, DefaultSlotViewClass.LoadSynchronous()))
	{
		SlotWidget->InitializeSlot(EquipmentItem, SlotIndex);
		SlotWidget->OnEquipmentDroppedInSlot.BindUObject(this, &UEquipmentViewWidget::EquipItem);
		SlotWidget->OnEquipmentRemovedFromSlot.BindUObject(this, &UEquipmentViewWidget::UnequipItem);
		SlotWidget->OnSlotUpdated.BindUObject(this, &UEquipmentViewWidget::OnSlotUpdated);		
		ItemSlots->AddChildToGrid(SlotWidget, ItemSlots->GetChildrenCount(), 1);
		SlotWidget->UpdateView();
	}
}

void UEquipmentViewWidget::EquipItem(EInventoryItemType ItemType, int32 Amount, int32 SlotIndex)
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->Server_AddEquipmentItem(ItemType, Amount, SlotIndex);
	}
}

void UEquipmentViewWidget::UnequipItem(int32 SlotIndex)
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->Server_RemoveEquipmentItem(SlotIndex);
	}
}

void UEquipmentViewWidget::OnSlotUpdated(int32 SlotIndex)
{
	const AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->GetCharacterEquipmentComponent()->OnEquipmentSlotUpdated(SlotIndex);
	}
}
