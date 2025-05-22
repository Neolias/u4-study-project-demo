// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryViewWidget.h"

#include "InventorySlotWidget.h"
#include "Components/GridPanel.h"

void UInventoryViewWidget::InitializeWidget(const TArray<UInventorySlot*>& InventorySlots)
{
	for (UInventorySlot* SlotToAdd : InventorySlots)
	{
		AddSlotToView(SlotToAdd);
	}

	bIsInitialized = true;
}

void UInventoryViewWidget::UpdateSlotWidgets(const TArray<UInventorySlot*>& InventorySlots) const
{
	if (!bIsInitialized || !ItemSlots)
	{
		return;
	}

	int32 i = 0;
	for (const UInventorySlot* InventorySlot : InventorySlots)
	{
		if (UInventorySlotWidget* SlotWidget = StaticCast<UInventorySlotWidget*>(ItemSlots->GetChildAt(i)))
		{
			UInventoryItem* LinkedItem = IsValid(InventorySlot) ? InventorySlot->GetLinkedInventoryItem() : nullptr;
			SlotWidget->SetLinkedSlotItem(LinkedItem);
		}
		i++;
	}
}

void UInventoryViewWidget::AddSlotToView(UInventorySlot* SlotToAdd)
{
	checkf(InventorySlotWidgetClass.LoadSynchronous() != nullptr, TEXT("UItemContainerWidget::AddSlotToView(): InventorySlotWidgetClass is undefined."));
	if (UInventorySlotWidget* SlotWidget = CreateWidget<UInventorySlotWidget>(this, InventorySlotWidgetClass.LoadSynchronous()))
	{
		SlotWidget->InitializeSlot(SlotToAdd);

		int32 CurrentSlotCount = ItemSlots->GetChildrenCount();
		int32 CurrentSlotRow = CurrentSlotCount / ColumnCount;
		int32 CurrentSlotColumn = CurrentSlotCount % ColumnCount;
		ItemSlots->AddChildToGrid(SlotWidget, CurrentSlotRow, CurrentSlotColumn);

		SlotWidget->UpdateView();
	}
}
