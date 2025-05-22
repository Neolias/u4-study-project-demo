// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Equipment/GA_CharEquipFromCurrentSlotFast.h"

#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "GameFramework/PawnMovementComponent.h"

UGA_CharEquipFromCurrentSlotFast::UGA_CharEquipFromCurrentSlotFast()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

bool UGA_CharEquipFromCurrentSlotFast::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	checkf(ActorInfo->AvatarActor.Get() && ActorInfo->AvatarActor->IsA<AXyzBaseCharacter>(), TEXT("UGA_CharEquipFromCurrentSlotFast::CanActivateAbility(): UGA_CharEquipFromCurrentSlotFast can only be used with AXyzAXyzBaseCharacter."))
	const AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(ActorInfo->AvatarActor.Get());
	return IsValid(BaseCharacter) && !BaseCharacter->IsDead() && !BaseCharacter->GetRootComponent()->IsSimulatingPhysics() && BaseCharacter->GetMovementComponent()->IsMovingOnGround();
}

void UGA_CharEquipFromCurrentSlotFast::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(ActorInfo->AvatarActor.Get());
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->GetCharacterEquipmentComponent()->EquipItemFromCurrentSlot(true);
	}
	CancelAbility(Handle, ActorInfo, ActivationInfo, true);
}
