// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

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
