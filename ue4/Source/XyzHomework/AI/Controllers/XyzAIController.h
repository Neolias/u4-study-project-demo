// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "XyzAIController.generated.h"

/** AI controller used with all AI pawns. */
UCLASS()
class XYZHOMEWORK_API AXyzAIController : public AAIController
{
	GENERATED_BODY()

public:
	AXyzAIController();
	virtual AActor* GetClosestSensedActor(TSubclassOf<class UAISense> SenseClass, const FAISenseAffiliationFilter& AffiliationFilter) const;

protected:
	UFUNCTION()
	virtual void OnPawnDamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
};
