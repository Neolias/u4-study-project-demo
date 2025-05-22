// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Tasks/AT_TickTaskBase.h"
#include "AT_UpdateDive.generated.h"

/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API UAT_UpdateDive : public UAT_TickTaskBase
{
	GENERATED_BODY()

public:
	virtual void TickTask(float DeltaTime) override;
	static UAT_UpdateDive* NewUpdateDiveTask(UGameplayAbility* OwningAbility);
};
