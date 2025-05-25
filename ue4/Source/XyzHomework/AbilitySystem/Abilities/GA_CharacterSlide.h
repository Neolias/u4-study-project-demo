// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "GA_CharacterAbilityBase.h"
#include "GA_CharacterSlide.generated.h"

UCLASS()
class XYZHOMEWORK_API UGA_CharacterSlide : public UGA_CharacterAbilityBase
{
	GENERATED_BODY()

public:
	UGA_CharacterSlide();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* OwnerInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
