// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Equipment/EquipmentItem.h"

#include "Characters/XyzBaseCharacter.h"
#include "Engine/ActorChannel.h"
#include "Inventory/Items/InventoryItem.h"
#include "Net/UnrealNetwork.h"

AEquipmentItem::AEquipmentItem()
{
	bReplicates = true;
}

void AEquipmentItem::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);

	if (IsValid(NewOwner))
	{
		checkf(GetOwner()->IsA<AXyzBaseCharacter>(), TEXT("AEquipmentItem::SetOwner(): AEquipmentItem can only be used with AXyzBaseCharacter."))
		CachedBaseCharacter = StaticCast<AXyzBaseCharacter*>(GetOwner());
		if (GetLocalRole() == ROLE_Authority)
		{
			SetAutonomousProxy(true);
		}
	}
	else
	{
		CachedBaseCharacter.Reset();
	}
}

void AEquipmentItem::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEquipmentItem, LinkedInventoryItem)
}

bool AEquipmentItem::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bResult = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	bResult |= Channel->ReplicateSubobject(LinkedInventoryItem, *Bunch, *RepFlags);
	return bResult;
}

void AEquipmentItem::SetLinkedInventoryItem(UInventoryItem* InventoryItem)
{
	LinkedInventoryItem = InventoryItem;
}

bool AEquipmentItem::IsEquipmentSlotCompatible(EEquipmentItemSlot EquipmentSlot) const
{
	return CompatibleEquipmentSlots.Contains(EquipmentSlot);
}

//@ SaveSubsystemInterface
void AEquipmentItem::OnLevelDeserialized_Implementation()
{
	if (!IsValid(Cast<ACharacter>(GetOwner())))
	{
		Destroy();
		return;
	}

	SetActorRelativeTransform(FTransform(FRotator::ZeroRotator, FVector::ZeroVector));
}

//~ SaveSubsystemInterface
