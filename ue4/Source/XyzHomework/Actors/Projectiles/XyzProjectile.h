// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "XyzProjectile.generated.h"

/** Base class of all projectiles. */
UCLASS()
class XYZHOMEWORK_API AXyzProjectile : public AActor
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_FourParams(FOnCollisionComponentHit, AXyzProjectile*, FVector, const FHitResult&, FVector);
	FOnCollisionComponentHit OnCollisionComponentHitEvent;

	AXyzProjectile();
	virtual void Tick(float DeltaSeconds) override;
	/** Makes the projectile visible and enables its movement. */
	void SetProjectileActive(bool bIsActive);
	virtual void Launch(FVector Direction, FVector ProjectileResetLocation = FVector::ZeroVector);

protected:
	virtual void BeginPlay() override;
	APawn* GetOwningPawn() const;
	virtual void OnProjectileLaunched() {}
	UFUNCTION()
	virtual void OnCollisionComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UProjectileMovementComponent* ProjectileMovementComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	class USphereComponent* CollisionComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	UStaticMeshComponent* StaticMeshComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (ClampMin = 0.f, UIMin = 0.f))
	float LaunchSpeed = 1500.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (ClampMin = 0.f, UIMin = 0.f))
	float MaxSpeed = 3000.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (ClampMin = 0.f, UIMin = 0.f))
	float Bounciness = 0.1f;

	/** Location where the projectile will idle while it is not used (location of the projectile pool). */
	FVector ResetLocation = FVector::ZeroVector;
};
