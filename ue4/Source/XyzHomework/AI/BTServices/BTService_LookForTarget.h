// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_LookForTarget.generated.h"

UCLASS()
class XYZHOMEWORK_API UBTService_LookForTarget : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_LookForTarget();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	bool CanSeeTarget(AAIController* AIController, AActor* CurrentTarget) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI parameters")
	float DetectionRange = 5000.f;
};
