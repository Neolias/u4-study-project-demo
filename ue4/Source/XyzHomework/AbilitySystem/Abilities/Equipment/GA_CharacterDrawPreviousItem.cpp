// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Equipment/GA_CharacterDrawPreviousItem.h"

#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

UGA_CharacterDrawPreviousItem::UGA_CharacterDrawPreviousItem()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UGA_CharacterDrawPreviousItem::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(ActorInfo->AvatarActor.Get());
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->GetCharacterEquipmentComponent()->DrawPreviousItem();
	}
	CancelAbility(Handle, ActorInfo, ActivationInfo, true);
}
