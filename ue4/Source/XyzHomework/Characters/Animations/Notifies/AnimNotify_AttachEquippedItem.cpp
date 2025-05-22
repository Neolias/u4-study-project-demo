// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Animations/Notifies/AnimNotify_AttachEquippedItem.h"

#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

void UAnimNotify_AttachEquippedItem::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	const AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(MeshComp->GetOwner());
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->GetCharacterEquipmentComponent()->AttachCurrentEquipmentItemToCharacterMesh();
	}	
}
