// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GA_CharacterAbilityBase.h"
#include "GA_CharacterPrimaryMeleeAttack.generated.h"

class AMeleeWeaponItem;
/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API UGA_CharacterPrimaryMeleeAttack : public UGA_CharacterAbilityBase
{
	GENERATED_BODY()

public:
	UGA_CharacterPrimaryMeleeAttack();
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
};
