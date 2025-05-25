// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ThrowItem.generated.h"

UCLASS()
class XYZHOMEWORK_API UBTTask_ThrowItem : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ThrowItem();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;	
};
