// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

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
