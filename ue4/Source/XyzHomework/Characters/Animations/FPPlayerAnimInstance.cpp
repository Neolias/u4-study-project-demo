// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Animations/FPPlayerAnimInstance.h"

#include "Characters/FPPlayerCharacter.h"

void UFPPlayerAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	if (IsValid(TryGetPawnOwner()))
	{
		checkf(TryGetPawnOwner()->IsA<AFPPlayerCharacter>(), TEXT("UFPPlayerAnimInstance::NativeBeginPlay(): UFPPlayerAnimInstance can only be used with AFPPlayerCharacter."))
		CachedFPCharacter = StaticCast<AFPPlayerCharacter*>(TryGetPawnOwner());
	}
}
