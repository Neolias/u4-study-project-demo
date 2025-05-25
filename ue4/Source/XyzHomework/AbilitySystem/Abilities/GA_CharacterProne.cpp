// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "AbilitySystem/Abilities/GA_CharacterProne.h"

#include "Characters/XyzBaseCharacter.h"
#include "Components/MovementComponents/XyzBaseCharMovementComponent.h"

UGA_CharacterProne::UGA_CharacterProne()
{
	AbilityType = EGameplayAbility::Prone;
}

bool UGA_CharacterProne::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(ActorInfo->AvatarActor.Get());
	return BaseCharacter && BaseCharacter->GetMovementComponent()->CanEverCrouch() && BaseCharacter->CanChangeProneState();
}

bool UGA_CharacterProne::CanBeCanceled() const
{
	if (!Super::CanBeCanceled())
	{
		return false;
	}

	const AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(GetAvatarActorFromActorInfo());
	return !IsValid(BaseCharacter) || BaseCharacter->IsDead() || (BaseCharacter->CanChangeProneState() && BaseCharacter->GetBaseCharacterMovementComponent()->CanUnProne());
}
