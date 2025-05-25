// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "AbilitySystem/Abilities/GA_CharacterAbilityBase.h"

#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "GameFramework/PawnMovementComponent.h"

UGA_CharacterAbilityBase::UGA_CharacterAbilityBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UGA_CharacterAbilityBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	checkf(ActorInfo->AvatarActor.Get() && ActorInfo->AvatarActor->IsA<AXyzBaseCharacter>(), TEXT("UGA_CharacterAbilityBase::CanActivateAbility(): UGA_CharacterAbilityBase can only be used with AXyzAXyzBaseCharacter."))
	const AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(ActorInfo->AvatarActor.Get());
	return IsValid(BaseCharacter) && !BaseCharacter->IsDead() && !BaseCharacter->GetRootComponent()->IsSimulatingPhysics() && BaseCharacter->GetMovementComponent()->IsMovingOnGround() && (BaseCharacter->GetCharacterEquipmentComponent()->IsEquipAnimMontagePlaying() || BaseCharacter->IsReloadingWeapon() || !BaseCharacter->IsAnimMontagePlaying());
}

void UGA_CharacterAbilityBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(ActorInfo->AvatarActor.Get());
	if (IsValid(BaseCharacter))
	{
		if (BaseCharacter->GetLocalRole() == ROLE_Authority)
		{
			BaseCharacter->Multicast_ExecuteGameplayAbilityCallback(AbilityType, true);
		}
		else
		{
			BaseCharacter->ExecuteGameplayAbilityCallbackInternal(AbilityType, true, false);
		}
	}
}

void UGA_CharacterAbilityBase::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	if (!CanBeCanceled())
	{
		return;
	}

	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);

	AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(ActorInfo->AvatarActor.Get());
	if (IsValid(BaseCharacter))
	{
		if (BaseCharacter->GetLocalRole() == ROLE_Authority)
		{
			BaseCharacter->Multicast_ExecuteGameplayAbilityCallback(AbilityType, false);
		}
		else
		{
			BaseCharacter->ExecuteGameplayAbilityCallbackInternal(AbilityType, false, false);
		}
	}
}
