// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "AbilitySystem/Tasks/AT_UpdateSlide.h"

#include "Characters/XyzBaseCharacter.h"
#include "GameFramework/PawnMovementComponent.h"

void UAT_UpdateSlide::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	const AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter))
	{
		BaseCharacter->GetMovementComponent()->AddInputVector(CachedBaseCharacter->GetActorForwardVector(), true);
	}
}

UAT_UpdateSlide* UAT_UpdateSlide::NewUpdateSlideTask(UGameplayAbility* OwningAbility)
{
	UAT_UpdateSlide* MyObj = NewAbilityTask<UAT_UpdateSlide>(OwningAbility);
	return MyObj;
}
