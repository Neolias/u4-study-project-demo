// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Weapons/GA_CharacterPrimaryMeleeAttack.h"

#include "Actors/Equipment/Weapons/MeleeWeaponItem.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

UGA_CharacterPrimaryMeleeAttack::UGA_CharacterPrimaryMeleeAttack()
{
	AbilityType = EGameplayAbility::PrimaryMeleeAttack;
}

bool UGA_CharacterPrimaryMeleeAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(ActorInfo->AvatarActor.Get());
	return BaseCharacter && IsValid(BaseCharacter->GetCharacterEquipmentComponent()->GetCurrentMeleeWeapon());
}
