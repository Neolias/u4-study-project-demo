// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Tasks/AT_TickTaskBase.h"

#include "Characters/XyzBaseCharacter.h"

UAT_TickTaskBase::UAT_TickTaskBase()
{
	bTickingTask = false;
	bSimulatedTask = true;
}

void UAT_TickTaskBase::Activate()
{
	Super::Activate();

	CachedBaseCharacter = Cast<AXyzBaseCharacter>(GetAvatarActor());	
	bTickingTask = true;
}
