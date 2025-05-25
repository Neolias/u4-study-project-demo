// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "Actors/Projectiles/XyzProjectile.h"
#include "ExplosiveProjectile.generated.h"

/** Base class of all projectiles that own the explosive component. */
UCLASS()
class XYZHOMEWORK_API AExplosiveProjectile : public AXyzProjectile
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnProjectileExplosionEvent, AExplosiveProjectile*, FVector);
	FOnProjectileExplosionEvent OnProjectileExplosionEvent;

	AExplosiveProjectile();

protected:
	void Explode();
	virtual void OnProjectileLaunched() override;
	void OnDetonationTimerElapsed();
	virtual void OnCollisionComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UExplosionComponent* ExplosionComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Explosive")
	bool bShouldDetonateOnCollision = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Explosive", meta = (ClampMin = 0.f, UIMin = 0.f))
	float DetonationTime = 2.f;

	FTimerHandle DetonationTimer;
};
