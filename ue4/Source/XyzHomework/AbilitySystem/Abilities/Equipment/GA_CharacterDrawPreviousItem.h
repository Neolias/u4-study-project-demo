// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_CharacterDrawPreviousItem.generated.h"

/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API UGA_CharacterDrawPreviousItem : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_CharacterDrawPreviousItem();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
