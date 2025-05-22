// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericStructs.h"
#include "Actors/Projectiles/ProjectilePool.h"
#include "Components/SceneComponent.h"
#include "WeaponMuzzleComponent.generated.h"

class UGameplayEffect;
class AExplosiveProjectile;
class UNiagaraSystem;
class AXyzProjectile;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class XYZHOMEWORK_API UWeaponMuzzleComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UWeaponMuzzleComponent();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void Shoot(FWeaponModeParameters* WeaponModeParameters, FVector ViewPointLocation, FRotator ViewPointRotation);

protected:
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment Item|Muzzle Component")
	TArray<FProjectilePool> ProjectilePools;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment Item|Muzzle Component")
	TSoftClassPtr<UGameplayEffect> DamageEffectClass;

private:
	APawn* GetOwningPawn() const;
	AController* GetController() const;
	void InstantiateProjectilePools(AActor* Owner);
	void ShootProjectile(TSoftClassPtr<AXyzProjectile> ProjectileClass, FVector MuzzleLocation, FVector ViewPointLocation, FRotator ViewPointRotation, FVector EndLocation);
	void OnShootProjectile(AXyzProjectile* Projectile, FVector StartLocation, FVector LaunchDirection, FVector ResetLocation);
	FVector ShootHitScan(FVector ViewPointLocation, FRotator ViewPointRotation, OUT FHitResult& HitResult, FVector MuzzleLocation, FVector EndLocation) const;
	void ProcessHit(FVector MovementDirection, const FHitResult& HitResult) const;
	void ProcessProjectileHit(AXyzProjectile* Projectile, FVector MovementDirection, const FHitResult& HitResult, FVector ResetLocation);
	void OnProjectileExplosion(AExplosiveProjectile* ExplosiveProjectile, FVector ResetLocation);
	void ResetProjectile(AXyzProjectile* Projectile, FVector ResetLocation) const;
	
	FWeaponModeParameters* ModeParameters;
};
