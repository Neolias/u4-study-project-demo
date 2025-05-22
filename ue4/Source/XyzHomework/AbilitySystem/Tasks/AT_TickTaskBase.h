// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AT_TickTaskBase.generated.h"

class AXyzBaseCharacter;
/**
 * 
 */
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
