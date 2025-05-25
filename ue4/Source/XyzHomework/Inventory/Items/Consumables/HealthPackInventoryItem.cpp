// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "HealthPackInventoryItem.h"

#include "AbilitySystem/AttributeSets/XyzCharacterAttributeSet.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterInventoryComponent.h"

bool UHealthPackInventoryItem::Consume(APawn* Pawn)
{
	Super::Consume(Pawn);

	const AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(Pawn);
	if (IsValid(BaseCharacter))
	{
		if (BaseCharacter->GetCharacterInventoryComponent()->RemoveInventoryItemByType(Description.InventoryItemType, 1))
		{
			BaseCharacter->GetCharacterAttributes()->AddHealth(HealthRestoreAmount);
			return true;
		}
	}

	return false;
}
