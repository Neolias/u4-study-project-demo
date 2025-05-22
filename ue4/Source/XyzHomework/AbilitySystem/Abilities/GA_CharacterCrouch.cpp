// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/GA_CharacterCrouch.h"

#include "Characters/XyzBaseCharacter.h"
#include "Components/MovementComponents/XyzBaseCharMovementComponent.h"

UGA_CharacterCrouch::UGA_CharacterCrouch()
{
	AbilityType = EGameplayAbility::Crouch;
}

bool UGA_CharacterCrouch::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(ActorInfo->AvatarActor.Get());
	return BaseCharacter && BaseCharacter->GetMovementComponent()->CanEverCrouch() && BaseCharacter->CanChangeCrouchState();
}

bool UGA_CharacterCrouch::CanBeCanceled() const
{
	if (!Super::CanBeCanceled())
	{
		return false;
	}

	const AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(GetAvatarActorFromActorInfo());
	return !IsValid(BaseCharacter) || BaseCharacter->IsDead() || (!BaseCharacter->IsProne() && BaseCharacter->CanChangeCrouchState() && BaseCharacter->GetBaseCharacterMovementComponent()->CanUnCrouch());
}
