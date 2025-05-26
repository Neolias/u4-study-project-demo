// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "AI/BTServices/BTService_AimAndFireWeapon.h"

#include "AIController.h"
#include "XyzHomeworkTypes.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

UBTService_AimAndFireWeapon::UBTService_AimAndFireWeapon()
{
	NodeName = "AimAndFireWeapon";
}

void UBTService_AimAndFireWeapon::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	const AController* AIController = OwnerComp.GetAIOwner();
	if (!IsValid(AIController))
	{
		return;
	}

	AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(AIController->GetPawn());
	if (!IsValid(BaseCharacter))
	{
		return;
	}

	if (UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent())
	{
		float DistanceToTarget = Blackboard->GetValueAsFloat(AICharacterBTDistanceToTargetName);
		bool bIsInRange = DistanceToTarget > MinFireRange && DistanceToTarget <= MaxFireRange;
		bool bCanSeeTarget = Blackboard->GetValueAsBool(AICharacterBTCanSeeTargetName);
		BaseCharacter->bUseControllerRotationYaw = bCanSeeTarget;
		bool bIsWeaponFireRequested = Blackboard->GetValueAsBool(AICharacterBTWeaponFireRequestedName);

		if (bIsInRange && bCanSeeTarget)
		{
			if (bIsWeaponFireRequested && !BaseCharacter->IsFiringWeapon())
			{
				BaseCharacter->EquipItemFromCurrentSlot();
			}
			if (!bIsWeaponFireRequested)
			{
				Blackboard->SetValueAsBool(AICharacterBTWeaponFireRequestedName, true);
				BaseCharacter->StartWeaponFire();
				BaseCharacter->StartAiming();
			}
		}
		else if (bIsWeaponFireRequested)
		{
			Blackboard->SetValueAsBool(AICharacterBTWeaponFireRequestedName, false);
			BaseCharacter->StopWeaponFire();
			BaseCharacter->StopAiming();
		}
	}
}
