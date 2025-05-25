// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "CharacterInventoryComponent.h"

#include "XyzGenericEnums.h"
#include "Characters/XyzBaseCharacter.h"
#include "Engine/ActorChannel.h"
#include "Inventory/Items/InventoryItem.h"
#include "Net/UnrealNetwork.h"
#include "UI/Widgets/Inventory/InventoryViewWidget.h"

bool UInventorySlot::IsSupportedForNetworking() const
{
	return true;
}

void UInventorySlot::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventorySlot, Item)
}

void UInventorySlot::OnSlotUpdated() const
{
	OnSlotUpdatedEvent.ExecuteIfBound();
}

void UInventorySlot::SetLinkedInventoryItem(UInventoryItem* InventoryItem)
{
	const AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(GetOuter());
	if (IsValid(BaseCharacter))
	{
		if (BaseCharacter->GetLocalRole() == ROLE_Authority)
		{
			Item = InventoryItem;
			OnLinkedInventoryItemChanged();
			if (!IsValid(Item))
			{
				BaseCharacter->GetCharacterInventoryComponent()->AddUsedSlotCount(-1);
			}
		}
		else
		{
			BaseCharacter->GetCharacterInventoryComponent()->Server_SetLinkedInventoryItem(this, InventoryItem);
		}
	}
}

void UInventorySlot::OnLinkedInventoryItemChanged()
{
	if (IsValid(Item))
	{
		Item->OnCountChangedEvent.Clear();
		Item->OnCountChangedEvent.AddUObject(this, &UInventorySlot::OnSlotUpdated);
	}
	OnSlotUpdated();
}

void UInventorySlot::ClearSlot()
{
	SetLinkedInventoryItem(nullptr);
}

void UInventorySlot::OnRep_Item()
{
	OnLinkedInventoryItemChanged();
}

UCharacterInventoryComponent::UCharacterInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UCharacterInventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCharacterInventoryComponent, ItemSlots)
	DOREPLIFETIME(UCharacterInventoryComponent, UsedSlotCount)
}

bool UCharacterInventoryComponent::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bResult = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	bResult |= Channel->ReplicateSubobjectList(ItemSlots, *Bunch, *RepFlags);

	for (const UInventorySlot* Slot : ItemSlots)
	{
		if (IsValid(Slot))
		{
			bResult |= Channel->ReplicateSubobject(Slot->GetLinkedInventoryItem(), *Bunch, *RepFlags);
		}
	}

	return bResult;
}

void UCharacterInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(GetOwner()->IsA<AXyzBaseCharacter>(), TEXT("UCharacterInventoryComponent::BeginPlay(): UCharacterInventoryComponent can only be used with AXyzBaseCharacter."))

	if (GetOwner()->GetLocalRole() == ROLE_Authority)
	{
		ItemSlots.Reserve(Capacity);
		for (int32 i = 0; i < Capacity; i++)
		{
			ItemSlots.Emplace(NewObject<UInventorySlot>(GetOwner()));
		}
	}
}

FInventoryTableRow* UCharacterInventoryComponent::LoadItemDataFromDataTable(EInventoryItemType ItemType) const
{
	if (const UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *InventoryItemDataTable.GetUniqueID().GetAssetPathString()))
	{
		FString RowID = UEnum::GetDisplayValueAsText<EInventoryItemType>(ItemType).ToString();
		return DataTable->FindRow<FInventoryTableRow>(FName(RowID), TEXT("Find item data"));
	}

	return nullptr;
}

FInventoryTableRow* UCharacterInventoryComponent::LoadItemDataFromDataTable(EWeaponAmmoType AmmoType) const
{
	if (const UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *InventoryItemDataTable.GetUniqueID().GetAssetPathString()))
	{
		FString RowID = UEnum::GetDisplayValueAsText<EWeaponAmmoType>(AmmoType).ToString() + FString("Ammo");
		return DataTable->FindRow<FInventoryTableRow>(FName(RowID), TEXT("Find item data"));
	}
	return nullptr;
}

void UCharacterInventoryComponent::AddUsedSlotCount(int32 Amount)
{
	UsedSlotCount = FMath::Clamp(UsedSlotCount + Amount, 0, Capacity);
}

void UCharacterInventoryComponent::CreateViewWidget(APlayerController* PlayerController)
{
	if (IsValid(InventoryViewWidget))
	{
		return;
	}

	if (!IsValid(PlayerController) || !InventoryViewWidgetClass.LoadSynchronous())
	{
		return;
	}

	InventoryViewWidget = CreateWidget<UInventoryViewWidget>(PlayerController, InventoryViewWidgetClass.LoadSynchronous());
	InventoryViewWidget->InitializeWidget(ItemSlots);
}

void UCharacterInventoryComponent::OpenViewInventory(APlayerController* PlayerController)
{
	if (!IsValid(InventoryViewWidget))
	{
		CreateViewWidget(PlayerController);
	}

	if (IsValid(InventoryViewWidget) && !InventoryViewWidget->IsVisible())
	{
		InventoryViewWidget->AddToViewport();
	}
}

void UCharacterInventoryComponent::CloseViewInventory()
{
	if (InventoryViewWidget->IsVisible())
	{
		InventoryViewWidget->RemoveFromParent();
	}
}

bool UCharacterInventoryComponent::IsViewInventoryVisible() const
{
	if (IsValid(InventoryViewWidget))
	{
		return InventoryViewWidget->IsVisible();
	}
	return false;
}

bool UCharacterInventoryComponent::AddInventoryItem(EInventoryItemType ItemType, int32 Amount)
{
	if (Amount < 1)
	{
		return false;
	}

	int32 Remainder = StackItems(ItemType, Amount);
	if (Remainder == -1) // indicates an error
	{
		return false;
	}

	return FillEmptySlots(ItemType, Remainder);
}

int32 UCharacterInventoryComponent::StackItems(EInventoryItemType ItemType, int32 Amount)
{
	if (Amount < 1)
	{
		return 0;
	}

	TArray<UInventorySlot*> CompatibleItemSlots;
	int32 MaxCountPerSlot = 0;
	int32 AvailableSpaceInSlots = 0;
	for (UInventorySlot* Slot : ItemSlots)
	{
		if (IsValid(Slot))
		{
			const UInventoryItem* LinkedSlotItem = Slot->GetLinkedInventoryItem();
			if (IsValid(LinkedSlotItem) && LinkedSlotItem->GetInventoryItemType() == ItemType && LinkedSlotItem->CanStackItems())
			{
				CompatibleItemSlots.Add(Slot);
				MaxCountPerSlot = LinkedSlotItem->GetMaxCount();
				AvailableSpaceInSlots += MaxCountPerSlot - LinkedSlotItem->GetCount();
			}
		}
	}

	if (CompatibleItemSlots.Num() > 0 && Amount > AvailableSpaceInSlots + (Capacity - UsedSlotCount) * MaxCountPerSlot)
	{
		return -1; // indicates an error
	}

	int32 Remainder = Amount;
	for (const UInventorySlot* Slot : CompatibleItemSlots)
	{
		Remainder -= Slot->GetLinkedInventoryItem()->AddCount(Remainder);
		if (Remainder == 0)
		{
			break;
		}
	}
	return Remainder;
}

void UCharacterInventoryComponent::Server_StackItems_Implementation(UInventorySlot* InventorySlot, EInventoryItemType ItemType, int32 AmountToAdd)
{
	int32 Remainder = AmountToAdd;

	// Try filling the provided slot
	if (IsValid(InventorySlot))
	{
		UInventoryItem* LinkedSlotItem = InventorySlot->GetLinkedInventoryItem();
		if (IsValid(LinkedSlotItem) && LinkedSlotItem->GetInventoryItemType() == ItemType && LinkedSlotItem->CanStackItems())
		{
			Remainder -= LinkedSlotItem->AddCount(AmountToAdd);
		}
	}

	// Since stacking is forced by clients, remaining items should be handled if they cannot fit
	if (Remainder > 0)
	{
		AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(GetOwner());
		if (IsValid(BaseCharacter))
		{
			Remainder = StackItems(ItemType, Remainder);
			if (Remainder > 0 && !BaseCharacter->PickupItem(ItemType, Remainder))
			{
				BaseCharacter->DropItem(ItemType, Remainder);
			}
		}
	}
}

bool UCharacterInventoryComponent::FillEmptySlots(EInventoryItemType ItemType, int32 Amount)
{
	if (Amount < 1)
	{
		return true;
	}

	int32 Remainder = Amount;
	int32 FreeSlotCount = Capacity - UsedSlotCount;
	if (Remainder && !FreeSlotCount)
	{
		return false;
	}

	const FInventoryTableRow* ItemData = LoadItemDataFromDataTable(ItemType);
	if (ItemData && ItemData->InventoryItemClass.LoadSynchronous())
	{
		int32 RequiredSlotCount = FMath::CeilToInt((float)Remainder / ItemData->InventoryItemDescription.MaxCount);
		if (RequiredSlotCount > FreeSlotCount)
		{
			return false;
		}

		for (int32 i = 0; i < RequiredSlotCount; ++i)
		{
			UInventorySlot** FreeSlotPtr = ItemSlots.FindByPredicate([=](const UInventorySlot* Slot) { return Slot && !IsValid(Slot->GetLinkedInventoryItem()); });
			if (FreeSlotPtr && *FreeSlotPtr)
			{
				UInventorySlot* FreeSlot = *FreeSlotPtr;
				UInventoryItem* NewItem = NewObject<UInventoryItem>(GetOwner(), ItemData->InventoryItemClass.LoadSynchronous());
				NewItem->InitializeItem(ItemData->InventoryItemDescription);
				Remainder -= NewItem->AddCount(Remainder);
				FreeSlot->SetLinkedInventoryItem(NewItem);
				UsedSlotCount++;
			}
		}
	}

	return Remainder == 0;
}

int32 UCharacterInventoryComponent::RemoveInventoryItemByType(EInventoryItemType ItemType, int32 Amount)
{
	if (Amount < 1)
	{
		return 0;
	}

	TArray<UInventorySlot*> CompatibleItemSlots;
	for (UInventorySlot* Slot : ItemSlots)
	{
		if (IsValid(Slot))
		{
			const UInventoryItem* LinkedSlotItem = Slot->GetLinkedInventoryItem();
			if (IsValid(LinkedSlotItem) && LinkedSlotItem->GetInventoryItemType() == ItemType)
			{
				CompatibleItemSlots.Add(Slot);
			}
		}
	}
	CompatibleItemSlots.Sort([=](const UInventorySlot& SlotA, const UInventorySlot& SlotB)
	{
		return SlotA.GetLinkedInventoryItem()->GetCount() < SlotB.GetLinkedInventoryItem()->GetCount();
	});

	int32 Remainder = Amount;
	for (UInventorySlot* Slot : CompatibleItemSlots)
	{
		Remainder -= RemoveInventoryItemBySlot(Slot, Remainder);
		if (Remainder < 1)
		{
			break;
		}
	}

	return FMath::Clamp(Amount - Remainder, 0, Amount);
}

int32 UCharacterInventoryComponent::RemoveInventoryItemByIndex(int32 SlotIndex, int32 Amount)
{
	if (SlotIndex >= 0 && SlotIndex < ItemSlots.Num())
	{
		return RemoveInventoryItemBySlot(ItemSlots[SlotIndex], Amount);
	}

	return 0;
}

int32 UCharacterInventoryComponent::RemoveInventoryItemBySlot(UInventorySlot* Slot, int32 Amount)
{
	if (!Slot)
	{
		return 0;
	}

	int32 Result = 0;
	UInventoryItem* LinkedSlotItem = Slot->GetLinkedInventoryItem();
	if (IsValid(LinkedSlotItem))
	{
		int32 ItemCount = LinkedSlotItem->GetCount();
		if (ItemCount > Amount)
		{
			LinkedSlotItem->SetCount(ItemCount - Amount);
			Result = Amount;
		}
		else
		{
			Slot->ClearSlot();
			UsedSlotCount--;
			Result = ItemCount;
		}
	}

	return Result;
}

bool UCharacterInventoryComponent::AddAmmoItem(EWeaponAmmoType AmmoType, int32 Amount)
{
	if (Amount < 1)
	{
		return true;
	}

	if (const FInventoryTableRow* ItemData = LoadItemDataFromDataTable(AmmoType))
	{
		return AddInventoryItem(ItemData->InventoryItemDescription.InventoryItemType, Amount);
	}

	return true; // If ammo data is not found, returns true to let the equipment component fill EquipmentAmmoArray
}

int32 UCharacterInventoryComponent::RemoveAmmoItem(EWeaponAmmoType AmmoType, int32 Amount)
{
	if (Amount < 1)
	{
		return 0;
	}

	if (const FInventoryTableRow* ItemData = LoadItemDataFromDataTable(AmmoType))
	{
		return RemoveInventoryItemByType(ItemData->InventoryItemDescription.InventoryItemType, Amount);
	}

	return Amount; // If ammo data is not found, returns requested amount to let ammo reloading proceed
}

void UCharacterInventoryComponent::Server_SetLinkedInventoryItem_Implementation(UInventorySlot* InventorySlot, UInventoryItem* InventoryItem)
{
	InventorySlot->SetLinkedInventoryItem(InventoryItem);
}
