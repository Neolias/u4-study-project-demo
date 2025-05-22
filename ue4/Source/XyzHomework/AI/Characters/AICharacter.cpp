// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Characters/AICharacter.h"

#include "AIController.h"
#include "AbilitySystem/AttributeSets/XyzCharacterAttributeSet.h"
#include "Components/CharacterComponents/AIPatrollingComponent.h"

AAICharacter::AAICharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PatrollingComponent = CreateDefaultSubobject<UAIPatrollingComponent>(TEXT("AIPatrollingComponent"));
	AttributeSet->Defence = 0.f;
}

//@ SaveSubsystemInterface
void AAICharacter::OnLevelDeserialized_Implementation()
{
	Super::OnLevelDeserialized_Implementation();

	if (!IsValid(GetController()))
	{
		SpawnDefaultController();
	}
}

//~ SaveSubsystemInterface
