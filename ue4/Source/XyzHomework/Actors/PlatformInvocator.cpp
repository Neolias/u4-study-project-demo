// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformInvocator.h"

APlatformInvocator::APlatformInvocator()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APlatformInvocator::Invoke() const
{
	if (OnInvocatorActivated.IsBound())
	{
		OnInvocatorActivated.Broadcast();
	}
}

void APlatformInvocator::BeginPlay()
{
	Super::BeginPlay();
}

void APlatformInvocator::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

