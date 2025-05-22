// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Tasks/AT_UpdateDive.h"

#include "Characters/XyzBaseCharacter.h"

void UAT_UpdateDive::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->SwimUp(-1);
	}
}

UAT_UpdateDive* UAT_UpdateDive::NewUpdateDiveTask(UGameplayAbility* OwningAbility)
{
	UAT_UpdateDive* MyObj = NewAbilityTask<UAT_UpdateDive>(OwningAbility);
	return MyObj;
}
