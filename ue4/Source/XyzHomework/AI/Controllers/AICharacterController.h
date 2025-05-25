// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "AI/Controllers/XyzAIController.h"
#include "AICharacterController.generated.h"

/** AI controlled used with all humanoid AI characters. */
UCLASS()
class XYZHOMEWORK_API AAICharacterController : public AXyzAIController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "AIController")
	void ResetCurrentTarget();

protected:
	virtual void SetPawn(APawn* InPawn) override;
	virtual void ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors) override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

private:
	/** Finds a new waypoint and sets its location to a blackboard value. */
	void TryMoveToNextWayPoint();

	TWeakObjectPtr<class AAICharacter> CachedAICharacter;
	bool bIsPatrolling = false;
};
