// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/GA_CharacterDive.h"

#include "AbilitySystem/Tasks/AT_UpdateDive.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/MovementComponents/XyzBaseCharMovementComponent.h"

UGA_CharacterDive::UGA_CharacterDive()
{
	AbilityType = EGameplayAbility::Dive;
}

bool UGA_CharacterDive::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!UGameplayAbility::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	checkf(ActorInfo->AvatarActor.Get() && ActorInfo->AvatarActor->IsA<AXyzBaseCharacter>(), TEXT("UGA_CharacterDive::CanActivateAbility(): UGA_CharacterDive can only be used with AXyzAXyzBaseCharacter."))
	const AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(ActorInfo->AvatarActor.Get());
	return IsValid(BaseCharacter) && !BaseCharacter->IsDead() && !BaseCharacter->GetRootComponent()->IsSimulatingPhysics() && BaseCharacter->GetMovementComponent()->IsSwimming();
}

void UGA_CharacterDive::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(ActorInfo->AvatarActor.Get());
	if (IsValid(BaseCharacter))
	{
		UAT_UpdateDive* TickTask = UAT_UpdateDive::NewUpdateDiveTask(this);
		TickTask->ReadyForActivation();
	}
}
