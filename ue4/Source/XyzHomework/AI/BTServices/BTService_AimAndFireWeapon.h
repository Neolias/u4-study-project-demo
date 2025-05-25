// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_AimAndFireWeapon.generated.h"

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
