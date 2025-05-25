// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GA_CharacterAbilityBase.h"
#include "GA_CharacterUseEnvironmentActor.generated.h"

UCLASS()
class XYZHOMEWORK_API UGA_CharacterUseEnvironmentActor : public UGA_CharacterAbilityBase
{
	GENERATED_BODY()

public:
	UGA_CharacterUseEnvironmentActor();
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
};
