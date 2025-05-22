// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/GA_CharacterSprint.h"

#include "Characters/XyzBaseCharacter.h"
#include "Components/MovementComponents/XyzBaseCharMovementComponent.h"

UGA_CharacterSprint::UGA_CharacterSprint()
{
	AbilityType = EGameplayAbility::Sprint;
}

bool UGA_CharacterSprint::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(ActorInfo->AvatarActor.Get());
	bool Result = true;
	if (BaseCharacter && BaseCharacter->IsCrouching())
	{
		if (BaseCharacter->IsProne())
		{
			Result = BaseCharacter->GetBaseCharacterMovementComponent()->CanUnProne();
		}
		if (Result)
		{
			Result = BaseCharacter->GetBaseCharacterMovementComponent()->CanUnCrouch();
		}
	}
	return Result;
}
