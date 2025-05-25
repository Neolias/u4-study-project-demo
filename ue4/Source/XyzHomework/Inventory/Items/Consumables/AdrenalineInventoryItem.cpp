// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "AdrenalineInventoryItem.h"

#include "AbilitySystem/AttributeSets/XyzCharacterAttributeSet.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterInventoryComponent.h"

bool UAdrenalineInventoryItem::Consume(APawn* Pawn)
{
	Super::Consume(Pawn);

	const AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(Pawn);
	if (IsValid(BaseCharacter))
	{
		if (BaseCharacter->GetCharacterInventoryComponent()->RemoveInventoryItemByType(Description.InventoryItemType, 1))
		{
			UXyzCharacterAttributeSet* AttributeSet = BaseCharacter->GetCharacterAttributes();
			AttributeSet->SetStamina(AttributeSet->GetMaxStamina());
			return true;
		}
	}

	return false;
}
