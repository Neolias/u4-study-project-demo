// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Weapons/GA_CharacterFireWeapon.h"

#include "AbilitySystem/Tasks/AT_AutoFireWeapon.h"
#include "Actors/Equipment/Weapons/RangedWeaponItem.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

UGA_CharacterFireWeapon::UGA_CharacterFireWeapon()
{
	AbilityType = EGameplayAbility::FireWeapon;
}

bool UGA_CharacterFireWeapon::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	const AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(ActorInfo->AvatarActor.Get());
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) || (BaseCharacter && BaseCharacter->GetCharacterEquipmentComponent()->IsEquipAnimMontagePlaying()))
	{
		return false;
	}

	const ARangedWeaponItem* CurrentRangedWeapon = BaseCharacter->GetCharacterEquipmentComponent()->GetCurrentRangedWeapon();
	return IsValid(CurrentRangedWeapon) && CurrentRangedWeapon->GetWeaponModeParameters();
}

void UGA_CharacterFireWeapon::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(ActorInfo->AvatarActor.Get());
	if (IsValid(BaseCharacter))
	{
		const ARangedWeaponItem* CurrentRangedWeapon = BaseCharacter->GetCharacterEquipmentComponent()->GetCurrentRangedWeapon();
		if (IsValid(CurrentRangedWeapon) && CurrentRangedWeapon->GetWeaponModeParameters()->FireMode == EWeaponFireMode::FullAuto)
		{
			if (UAT_AutoFireWeapon* AutoFireWeaponTask = UAT_AutoFireWeapon::NewAutoFireWeaponTask(this))
			{
				AutoFireWeaponTask->ReadyForActivation();
			}
		}
	}
}
