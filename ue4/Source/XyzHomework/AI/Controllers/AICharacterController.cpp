// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "AI/Controllers/AICharacterController.h"

#include "XyzHomeworkTypes.h"
#include "AI/Characters/AICharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CharacterComponents/AIPatrollingComponent.h"
#include "Perception/AISense.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Sight.h"

void AAICharacterController::ResetCurrentTarget()
{
	Blackboard->SetValueAsObject(AICharacterBTCurrentTargetName, nullptr);
	ClearFocus(EAIFocusPriority::Gameplay);
}

void AAICharacterController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	AAICharacter* AICharacter = CachedAICharacter.Get();
	if (IsValid(AICharacter))
	{
		AICharacter->OnTakeAnyDamage.RemoveDynamic(this, &AAICharacterController::OnPawnDamageTaken);
	}

	if (IsValid(InPawn))
	{
		checkf(InPawn->IsA<AAICharacter>(), TEXT("AAICharacterController::SetPawn(): AAICharacterController can posses only AAICharacters."));
		AICharacter = StaticCast<AAICharacter*>(InPawn);
		AICharacter->OnTakeAnyDamage.AddDynamic(this, &AAICharacterController::OnPawnDamageTaken);
		CachedAICharacter = AICharacter;
		RunBehaviorTree(AICharacter->GetBehaviorTree());
		TryMoveToNextWayPoint();
	}
	else
	{
		CachedAICharacter.Reset();
	}
}

void AAICharacterController::ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	AAICharacter* AICharacter = CachedAICharacter.Get();
	if (!IsValid(AICharacter) || !AICharacter->ShouldFollowEnemies() || !Blackboard)
	{
		return;
	}

	FAISenseAffiliationFilter AISenseAffiliationFilter = AICharacter->GetAISenseAffiliationFilter();
	AActor* ClosestSensedActor = GetClosestSensedActor(UAISense_Damage::StaticClass(), AISenseAffiliationFilter);
	if (!IsValid(ClosestSensedActor))
	{
		ClosestSensedActor = GetClosestSensedActor(UAISense_Sight::StaticClass(), AISenseAffiliationFilter);
	}

	if (IsValid(ClosestSensedActor))
	{
		Blackboard->SetValueAsObject(AICharacterBTCurrentTargetName, ClosestSensedActor);
		SetFocus(ClosestSensedActor);
		bIsPatrolling = false;
	}
	else
	{
		Blackboard->SetValueAsObject(AICharacterBTCurrentTargetName, nullptr);
		ClearFocus(EAIFocusPriority::Gameplay);
		TryMoveToNextWayPoint();
	}
}

void AAICharacterController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);
	if (!Result.IsSuccess())
	{
		bIsPatrolling = false;
	}

	TryMoveToNextWayPoint();
}

void AAICharacterController::TryMoveToNextWayPoint()
{
	AAICharacter* AICharacter = CachedAICharacter.Get();
	if (!IsValid(AICharacter) || !Blackboard)
	{
		return;
	}

	UAIPatrollingComponent* PatrollingComponent = AICharacter->GetPatrollingComponent();
	if (bIsPatrolling)
	{
		FVector NextWayPoint;
		if (PatrollingComponent->GetNextWayPoint(NextWayPoint))
		{
			Blackboard->SetValueAsVector(AICharacterBTNextLocationName, NextWayPoint);
		}
	}
	else
	{
		FVector ClosestWayPoint;
		if (PatrollingComponent->GetClosestWayPoint(ClosestWayPoint, CachedAICharacter->GetActorLocation()))
		{
			Blackboard->SetValueAsVector(AICharacterBTNextLocationName, ClosestWayPoint);
			bIsPatrolling = true;
		}
	}
}
