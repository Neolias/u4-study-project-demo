// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/XyzBaseCharacter.h"
#include "Perception/AIPerceptionTypes.h"
#include "AICharacter.generated.h"

class UAIPatrollingComponent;
class UBehaviorTree;
/**
 * 
 */
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
