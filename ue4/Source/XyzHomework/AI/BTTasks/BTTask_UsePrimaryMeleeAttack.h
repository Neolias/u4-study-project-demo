// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_UsePrimaryMeleeAttack.generated.h"

UCLASS()
class XYZHOMEWORK_API UBTTask_UsePrimaryMeleeAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_UsePrimaryMeleeAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;	
};
