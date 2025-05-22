// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/GA_CharacterSlide.h"

#include "AbilitySystem/Tasks/AT_UpdateSlide.h"
#include "Characters/XyzBaseCharacter.h"

UGA_CharacterSlide::UGA_CharacterSlide()
{
	AbilityType = EGameplayAbility::Slide;
}

void UGA_CharacterSlide::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* OwnerInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, OwnerInfo, ActivationInfo, TriggerEventData);

	const AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(OwnerInfo->AvatarActor.Get());
	if (IsValid(BaseCharacter))
	{
		UAT_UpdateSlide* UpdateSlideTask = UAT_UpdateSlide::NewUpdateSlideTask(this);
		UpdateSlideTask->ReadyForActivation();
	}
}
