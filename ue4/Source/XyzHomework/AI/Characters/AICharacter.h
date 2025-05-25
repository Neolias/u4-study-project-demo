// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "Characters/XyzBaseCharacter.h"
#include "Perception/AIPerceptionTypes.h"
#include "AICharacter.generated.h"

class UAIPatrollingComponent;
class UBehaviorTree;

/** Base class of all humanoid AI characters. Owns the patrolling component. */
UCLASS(Blueprintable)
class XYZHOMEWORK_API AAICharacter : public AXyzBaseCharacter
{
	GENERATED_BODY()

public:
	AAICharacter(const FObjectInitializer& ObjectInitializer);
	UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }
	FAISenseAffiliationFilter GetAISenseAffiliationFilter() const { return AISenseAffiliationFilter; }
	UAIPatrollingComponent* GetPatrollingComponent() const { return PatrollingComponent; }
	bool ShouldFollowEnemies() const { return bShouldFollowEnemies; }

	//@ SaveSubsystemInterface
	virtual void OnLevelDeserialized_Implementation() override;
	//~ SaveSubsystemInterface

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UAIPatrollingComponent* PatrollingComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|AI")
	UBehaviorTree* BehaviorTree;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|AI")
	FAISenseAffiliationFilter AISenseAffiliationFilter;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|AI")
	bool bShouldFollowEnemies = true;
};
