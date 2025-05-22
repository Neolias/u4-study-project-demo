// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Abilities/GameplayAbility.h"
#include "GA_CharacterAbilityBase.generated.h"

class AXyzBaseCharacter;
/**
 * 
 */
UCLASS(Abstract)
class XYZHOMEWORK_API UGA_CharacterAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_CharacterAbilityBase();
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="GAS")
	EGameplayAbility AbilityType = EGameplayAbility::None;
};
