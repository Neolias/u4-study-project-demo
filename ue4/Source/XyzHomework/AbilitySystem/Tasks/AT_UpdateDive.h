// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Tasks/AT_TickTaskBase.h"
#include "AT_UpdateDive.generated.h"

UCLASS()
class XYZHOMEWORK_API UAT_UpdateDive : public UAT_TickTaskBase
{
	GENERATED_BODY()

public:
	virtual void TickTask(float DeltaTime) override;
	static UAT_UpdateDive* NewUpdateDiveTask(UGameplayAbility* OwningAbility);
};
