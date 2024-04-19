// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Equipment/EquipmentItem.h"

#include "Characters/XyzBaseCharacter.h"

void AEquipmentItem::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);

	if (IsValid(NewOwner))
	{
		checkf(GetOwner()->IsA<AXyzBaseCharacter>(), TEXT("EquipmentItem object should be owned by AXyzBaseCharacter."))
			CachedBaseCharacterOwner = StaticCast<AXyzBaseCharacter*>(GetOwner());
	}
	else
	{
		CachedBaseCharacterOwner = nullptr;
	}
}
