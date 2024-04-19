// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericStructs.h"
#include "Actors/Projectiles/XyzProjectile.h"
#include "Components/SceneComponent.h"
#include "WeaponMuzzleComponent.generated.h"

class UNiagaraSystem;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class XYZHOMEWORK_API UWeaponMuzzleComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UWeaponMuzzleComponent();
	void Shoot(const FWeaponModeParameters* WeaponModeParameters, FVector ViewPointLocation, FRotator ViewPointRotation, AController* Controller);

private:
	const FWeaponModeParameters* ModeParameters;

	APawn* GetOwningPawn() const;
	void ShootProjectile(FVector MuzzleLocation, FVector ViewPointLocation, FRotator ViewPointRotation, FVector EndLocation) const;
	FVector ShootHitScan(FVector ViewPointLocation, FRotator ViewPointRotation, AController* Controller, OUT FHitResult& HitResult, FVector MuzzleLocation, FVector EndLocation);
	UFUNCTION()
	void ProcessHit(FVector MovementDirection, const FHitResult& HitResult);
};
