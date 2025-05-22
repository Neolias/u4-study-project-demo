// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "XyzProjectile.generated.h"

UCLASS()
class XYZHOMEWORK_API AXyzProjectile : public AActor
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_FourParams(FOnCollisionComponentHit, AXyzProjectile*, FVector, const FHitResult&, FVector);
	FOnCollisionComponentHit OnCollisionComponentHitEvent;

	AXyzProjectile();
	virtual void Tick(float DeltaSeconds) override;
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

	FVector ResetLocation = FVector::ZeroVector;
};
