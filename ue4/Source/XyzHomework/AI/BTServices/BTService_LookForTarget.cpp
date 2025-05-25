// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "AI/BTServices/BTService_LookForTarget.h"

#include "AIController.h"
#include "XyzHomeworkTypes.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/PlayerCharacter.h"

UBTService_LookForTarget::UBTService_LookForTarget()
{
	NodeName = "LookForTarget";
}

void UBTService_LookForTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!IsValid(AIController))
	{
		return;
	}

	if (UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent())
	{
		AActor* CurrentTarget = Cast<AActor>(Blackboard->GetValueAsObject(AICharacterBTCurrentTargetName));
		if (IsValid(CurrentTarget))
		{
			bool bCanSeeTarget = CanSeeTarget(AIController, CurrentTarget);
			Blackboard->SetValueAsBool(AICharacterBTCanSeeTargetName, bCanSeeTarget);

			float Distance = FVector::Dist(CurrentTarget->GetActorLocation(), AIController->GetPawn()->GetActorLocation());
			Blackboard->SetValueAsFloat(AICharacterBTDistanceToTargetName, Distance);
		}
		else
		{
			Blackboard->SetValueAsBool(AICharacterBTCanSeeTargetName, false);
		}
	}
}

bool UBTService_LookForTarget::CanSeeTarget(AAIController* AIController, AActor* CurrentTarget) const
{
	FVector ViewPointLocation;
	FRotator ViewPointRotation;
	AIController->GetActorEyesViewPoint(ViewPointLocation, ViewPointRotation);
	FHitResult HitResult;
	FCollisionQueryParams CollisionQueryParams;
	AActor* AIPawn = AIController->GetPawn();
	CollisionQueryParams.AddIgnoredActor(AIPawn);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, ViewPointLocation, CurrentTarget->GetActorLocation(), ECC_Pawn, CollisionQueryParams))
	{
		if (IsValid(HitResult.GetActor()) && HitResult.GetActor() == CurrentTarget)
		{
			FVector DirectionToTarget = CurrentTarget->GetActorLocation() - AIPawn->GetActorLocation();
			bool bIsLocatedAhead = FVector::DotProduct(AIPawn->GetActorForwardVector(), DirectionToTarget) > 0.0f;
			return bIsLocatedAhead;
		}
	}

	return false;
}
