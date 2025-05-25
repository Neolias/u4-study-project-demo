// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "InventoryItem.h"

#include "Actors/Equipment/EquipmentItem.h"
#include "Net/UnrealNetwork.h"
#include "UI/Widgets/Inventory/InventorySlotWidget.h"

bool UInventoryItem::IsSupportedForNetworking() const
{
	return true;
}

void UInventoryItem::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryItem, Description);
	DOREPLIFETIME(UInventoryItem, bIsEquipment);
	DOREPLIFETIME(UInventoryItem, Count);
}

void UInventoryItem::InitializeItem(const FInventoryItemDescription& InventoryItemDescription)
{
	Description = InventoryItemDescription;
	bIsEquipment = Description.EquipmentItemClass.LoadSynchronous() != nullptr;
}

void UInventoryItem::SetCount(int32 NewCount)
{
	Count = FMath::Clamp(NewCount, 0, Description.MaxCount);
	OnCountUpdated();
}

int32 UInventoryItem::AddCount(int32 Value)
{
	int32 OldCount = Count;
	SetCount(Count + Value);
	return Count - OldCount;
}

void UInventoryItem::OnCountUpdated() const
{
	if (OnCountChangedEvent.IsBound())
	{
		OnCountChangedEvent.Broadcast();
	}
}

int32 UInventoryItem::GetAvailableSpaceInStack() const
{
	return CanStackItems() ? Description.MaxCount - Count : 0;
}

void UInventoryItem::SetPreviousInventorySlotWidget(UInventorySlotWidget* SlotWidget)
{
	PreviousInventorySlotWidget = SlotWidget;
}

bool UInventoryItem::UpdatePreviousSlotWidget(UInventoryItem* NewData) const
{
	UInventorySlotWidget* PreviousSlotWidget = GetPreviousInventorySlotWidget();
	if (IsValid(PreviousSlotWidget))
	{
		PreviousSlotWidget->SetLinkedSlotItem(NewData);
		return true;
	}
	return false;
}

void UInventoryItem::OnRep_Count()
{
	OnCountUpdated();
}
