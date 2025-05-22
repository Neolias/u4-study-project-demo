// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GA_CharacterAbilityBase.h"
#include "GA_CharacterSprint.generated.h"

/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API UGA_CharacterSprint : public UGA_CharacterAbilityBase
{
	GENERATED_BODY()

public:
	UGA_CharacterSprint();
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
};
