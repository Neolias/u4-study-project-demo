// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/GA_CharacterOutOfStamina.h"

#include "Characters/XyzBaseCharacter.h"

UGA_CharacterOutOfStamina::UGA_CharacterOutOfStamina()
{
	AbilityType = EGameplayAbility::OutOfStamina;
}

bool UGA_CharacterOutOfStamina::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!UGameplayAbility::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	checkf(ActorInfo->AvatarActor.Get() && ActorInfo->AvatarActor->IsA<AXyzBaseCharacter>(), TEXT("UGA_CharacterOutOfStamina::CanActivateAbility(): UGA_CharacterOutOfStamina can only be used with AXyzAXyzBaseCharacter."))
	const AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(ActorInfo->AvatarActor.Get());
	return IsValid(BaseCharacter);
}
