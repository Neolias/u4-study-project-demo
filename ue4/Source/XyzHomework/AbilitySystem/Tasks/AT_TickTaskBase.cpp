// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

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
