// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/GA_CharacterJump.h"

UGA_CharacterJump::UGA_CharacterJump()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;

	AbilityType = EGameplayAbility::Jump;
}

void UGA_CharacterJump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	CancelAbility(Handle, ActorInfo, ActivationInfo, true);
}

void UGA_CharacterJump::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	UGameplayAbility::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}
