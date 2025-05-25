// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "AbilitySystem/Abilities/GA_CharacterUseEnvironmentActor.h"

#include "Characters/XyzBaseCharacter.h"

UGA_CharacterUseEnvironmentActor::UGA_CharacterUseEnvironmentActor()
{
	AbilityType = EGameplayAbility::UseEnvironmentActor;
}

bool UGA_CharacterUseEnvironmentActor::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(ActorInfo->AvatarActor.Get());
	return BaseCharacter && BaseCharacter->CanUseEnvironmentActors();
}
