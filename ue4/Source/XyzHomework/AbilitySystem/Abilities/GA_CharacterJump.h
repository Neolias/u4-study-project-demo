// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GA_CharacterAbilityBase.h"
#include "GA_CharacterJump.generated.h"

/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API UGA_CharacterJump : public UGA_CharacterAbilityBase
{
	GENERATED_BODY()

public:
	UGA_CharacterJump();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;
};
