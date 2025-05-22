// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GA_CharacterAbilityBase.h"
#include "GA_CharacterMantle.generated.h"

class AXyzBaseCharacter;
/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API UGA_CharacterMantle : public UGA_CharacterAbilityBase
{
	GENERATED_BODY()

public:
	UGA_CharacterMantle();
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
};
