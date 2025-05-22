// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Animations/Notifies/AnimNotify_EnableRagdoll.h"

#include "Characters/XyzBaseCharacter.h"

void UAnimNotify_EnableRagdoll::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	
	AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(MeshComp->GetOwner());
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->EnableRagdoll();
	}
}
