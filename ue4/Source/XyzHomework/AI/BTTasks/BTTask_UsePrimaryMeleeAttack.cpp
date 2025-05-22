// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTTasks/BTTask_UsePrimaryMeleeAttack.h"

#include "AIController.h"
#include "XyzHomeworkTypes.h"
#include "Actors/Equipment/Weapons/MeleeWeaponItem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

UBTTask_UsePrimaryMeleeAttack::UBTTask_UsePrimaryMeleeAttack()
{
	NodeName = "UsePrimaryMeleeAttack";
}

EBTNodeResult::Type UBTTask_UsePrimaryMeleeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
			if (BaseCharacter->GetCharacterEquipmentComponent()->EquipItemBySlot(EEquipmentItemSlot::MeleeWeapon))
			{
				BaseCharacter->StartPrimaryMeleeAttack();
				return EBTNodeResult::Succeeded;
			}
		}
	}

	return EBTNodeResult::Failed;
}
