// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_AimAndFireWeapon.generated.h"

/**
 * 
 */
UCLASS()
class XYZHOMEWORK_API UBTService_AimAndFireWeapon : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_AimAndFireWeapon();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI parameters")
	float MaxFireRange = 1800.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI parameters")
	float MinFireRange = 800.f;
};
