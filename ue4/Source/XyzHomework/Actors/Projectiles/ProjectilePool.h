// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "ProjectilePool.generated.h"

class AXyzProjectile;

/** Struct that holds instructions for spawning and managing projectiles. */
USTRUCT(BlueprintType)
struct FProjectilePool
{
	GENERATED_BODY()

public:
	void InstantiatePool(UWorld* World, AActor* PoolOwner_In);
	AXyzProjectile* GetNextAvailableProjectile();
	TArray<AXyzProjectile*> GetProjectiles() const { return Projectiles; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftClassPtr<AXyzProjectile> ProjectileClass;
	/** Number of projectiles that will be spawned. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 1, UIMin = 1))
	int32 PoolSize = 10;
	/** Reset location of all projectiles that are not used. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector PoolWorldLocation = FVector(0.f, 0.f, -1000.f);

private:
	UPROPERTY()
	AActor* PoolOwner = nullptr;
	UPROPERTY()
	TArray<AXyzProjectile*> Projectiles;
	/** Last projectile that was taken from the pool. */
	UPROPERTY()
	int32 CurrentProjectileIndex = -1;
};
