// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "AbilitySystem/Abilities/GA_CharacterMantle.h"

#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "Components/MovementComponents/XyzBaseCharMovementComponent.h"

UGA_CharacterMantle::UGA_CharacterMantle()
{
	AbilityType = EGameplayAbility::Mantle;
}

bool UGA_CharacterMantle::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!UGameplayAbility::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	checkf(ActorInfo->AvatarActor.Get() && ActorInfo->AvatarActor->IsA<AXyzBaseCharacter>(), TEXT("UGA_CharacterMantle::CanActivateAbility(): UGA_CharacterMantle can only be used with AXyzAXyzBaseCharacter."))
	const AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(ActorInfo->AvatarActor.Get());
	FLedgeDescription LedgeDescription;
	return IsValid(BaseCharacter) && !BaseCharacter->IsDead() && !BaseCharacter->GetRootComponent()->IsSimulatingPhysics() && (BaseCharacter->GetCharacterEquipmentComponent()->IsEquipAnimMontagePlaying() || BaseCharacter->IsReloadingWeapon() || !BaseCharacter->IsAnimMontagePlaying())
		&& (BaseCharacter->GetMovementComponent()->IsMovingOnGround() || BaseCharacter->GetMovementComponent()->IsSwimming() || BaseCharacter->GetBaseCharacterMovementComponent()->IsOnLadder())
		&& BaseCharacter->DetectLedge(LedgeDescription);
}
