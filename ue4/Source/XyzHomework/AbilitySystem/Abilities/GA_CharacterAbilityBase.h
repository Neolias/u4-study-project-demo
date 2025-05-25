// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Abilities/GameplayAbility.h"
#include "GA_CharacterAbilityBase.generated.h"

class AXyzBaseCharacter;

/** Base class of all character gameplay abilities. Capable of executing the actual game logic of abilities via AXyzBaseCharacter::ExecuteGameplayAbilityCallbackInternal(). */
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
