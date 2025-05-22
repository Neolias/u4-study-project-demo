// Fill out your copyright notice in the Description page of Project Settings.

#include "PickupItem.h"

#include "Characters/XyzBaseCharacter.h"

APickupItem::APickupItem()
{
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	SetRootComponent(ItemMesh);

	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

FName APickupItem::GetActionName()
{
	return ActionName;
}

void APickupItem::Interact(APawn* InteractingPawn)
{
	AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(InteractingPawn);
	if (IsValid(BaseCharacter) && BaseCharacter->GetLocalRole() == ROLE_Authority)
	{
		if (BaseCharacter->PickupItem(ItemType, Amount))
		{
			Destroy();
		}
	}
}
