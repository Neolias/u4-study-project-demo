// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GA_CharacterAbilityBase.h"
#include "GA_CharacterDive.generated.h"

class AXyzBaseCharacter;
/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API UGA_CharacterDive : public UGA_CharacterAbilityBase
{
	GENERATED_BODY()

public:
	UGA_CharacterDive();
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
