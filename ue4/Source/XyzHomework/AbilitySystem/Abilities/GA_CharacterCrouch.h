// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "GA_CharacterAbilityBase.h"
#include "GA_CharacterCrouch.generated.h"

UCLASS()
class XYZHOMEWORK_API UGA_CharacterCrouch : public UGA_CharacterAbilityBase
{
	GENERATED_BODY()

public:
	UGA_CharacterCrouch();
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual bool CanBeCanceled() const override;
};
