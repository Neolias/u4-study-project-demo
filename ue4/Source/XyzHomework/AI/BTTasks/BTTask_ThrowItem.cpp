// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "AI/BTTasks/BTTask_ThrowItem.h"

#include "AIController.h"
#include "XyzHomeworkTypes.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

UBTTask_ThrowItem::UBTTask_ThrowItem()
{
	NodeName = "ThrowItem";
}

EBTNodeResult::Type UBTTask_ThrowItem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AController* AIController = OwnerComp.GetAIOwner();
	if (!IsValid(AIController))
	{
		return EBTNodeResult::Failed;
	}

	AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(AIController->GetPawn());
	if (!IsValid(BaseCharacter))
	{
		return EBTNodeResult::Failed;
	}

	if (const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent())
	{
		const AActor* CurrentTarget = Cast<AActor>(Blackboard->GetValueAsObject(AICharacterBTCurrentTargetName));
		bool bCanSeeTarget = IsValid(CurrentTarget) && Blackboard->GetValueAsBool(AICharacterBTCanSeeTargetName);

		if (bCanSeeTarget)
		{
			if (!BaseCharacter->GetCharacterEquipmentComponent()->IsPrimaryItemEquipped())
			{
				BaseCharacter->TogglePrimaryItem();
			}

			BaseCharacter->StartItemThrow();
			if (BaseCharacter->IsThrowingItem())
			{
				return EBTNodeResult::Succeeded;
			}
		}
	}

	return EBTNodeResult::Failed;
}
