// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Interactive/PickupItems/AmmoPickupItem.h"

#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

void AAmmoPickupItem::Interact(APawn* InteractingPawn)
{
	const AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(InteractingPawn);
	if (IsValid(BaseCharacter) && BaseCharacter->GetLocalRole() == ROLE_Authority)
	{
		if (BaseCharacter->GetCharacterEquipmentComponent()->AddAmmo(AmmoType, Amount))
		{
			Destroy();
		}
	}
}
