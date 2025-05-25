// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AT_TickTaskBase.generated.h"

class AXyzBaseCharacter;

/** Base class of ticking gameplay tasks. */
UCLASS(Abstract)
class XYZHOMEWORK_API UAT_TickTaskBase : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAT_TickTaskBase();
	virtual void Activate() override;

protected:
	TWeakObjectPtr<AXyzBaseCharacter> CachedBaseCharacter;
};
