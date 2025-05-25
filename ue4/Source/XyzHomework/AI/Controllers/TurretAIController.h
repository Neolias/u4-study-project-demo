// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "XyzAIController.h"
#include "TurretAIController.generated.h"

UCLASS()
class XYZHOMEWORK_API ATurretAIController : public AXyzAIController
{
	GENERATED_BODY()

protected:
	virtual void SetPawn(APawn* InPawn) override;
	virtual void ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors) override;	

private:
	TWeakObjectPtr<class ATurret> CachedTurret;
};
