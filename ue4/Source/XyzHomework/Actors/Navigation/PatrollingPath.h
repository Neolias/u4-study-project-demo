// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PatrollingPath.generated.h"

UCLASS()
class XYZHOMEWORK_API APatrollingPath : public AActor
{
	GENERATED_BODY()

public:
	const TArray<FVector>& GetWayPoints() const { return WayPoints; }
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Path, meta = (MakeEditWidget))
	TArray<FVector> WayPoints;

};
