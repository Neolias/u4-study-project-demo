// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Equipment/GA_CharEquipFromCurrentSlot.h"

#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "GameFramework/PawnMovementComponent.h"

UGA_CharEquipFromCurrentSlot::UGA_CharEquipFromCurrentSlot()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

bool UGA_CharEquipFromCurrentSlot::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	checkf(ActorInfo->AvatarActor.Get() && ActorInfo->AvatarActor->IsA<AXyzBaseCharacter>(), TEXT("UGA_CharEquipFromCurrentSlot::CanActivateAbility(): UGA_CharEquipFromCurrentSlot can only be used with AXyzAXyzBaseCharacter."))
	const AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(ActorInfo->AvatarActor.Get());
	return IsValid(BaseCharacter) && !BaseCharacter->IsDead() && !BaseCharacter->GetRootComponent()->IsSimulatingPhysics() && BaseCharacter->GetMovementComponent()->IsMovingOnGround();
}

void UGA_CharEquipFromCurrentSlot::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(ActorInfo->AvatarActor.Get());
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->GetCharacterEquipmentComponent()->EquipItemFromCurrentSlot();
	}
	CancelAbility(Handle, ActorInfo, ActivationInfo, true);
}
