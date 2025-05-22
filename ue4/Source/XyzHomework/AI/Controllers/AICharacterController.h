// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Controllers/XyzAIController.h"
#include "AICharacterController.generated.h"

/**
 * 
 */
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
	void TryMoveToNextWayPoint();

	TWeakObjectPtr<class AAICharacter> CachedAICharacter;
	bool bIsPatrolling = false;
};
