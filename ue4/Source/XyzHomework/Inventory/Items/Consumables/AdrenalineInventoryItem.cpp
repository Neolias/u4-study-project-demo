// Fill out your copyright notice in the Description page of Project Settings.


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
		if (BaseCharacter->GetCharacterInventoryComponent()->RemoveInventoryItem(Description.InventoryItemType, 1))
		{
			UXyzCharacterAttributeSet* AttributeSet = BaseCharacter->GetCharacterAttributes();
			AttributeSet->SetStamina(AttributeSet->GetMaxStamina());
			return true;
		}
	}

	return false;
}
