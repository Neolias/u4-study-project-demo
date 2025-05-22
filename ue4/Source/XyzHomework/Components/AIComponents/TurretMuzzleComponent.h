// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericStructs.h"
#include "Components/SceneComponent.h"
#include "TurretMuzzleComponent.generated.h"

/**
 *
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class XYZHOMEWORK_API UTurretMuzzleComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UTurretMuzzleComponent();
	void StartFire();
	void StopFire();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret AI|Muzzle Component", meta = (ClampMin = 0.f, UIMin = 0.f))
	float FireRate = 150.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret AI|Muzzle Component", meta = (ClampMin = 0.f, UIMin = 0.f))
	float FirstShotDelay = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret AI|Muzzle Component", meta = (ClampMin = 0.f, UIMin = 0.f))
	float MaxBulletSpreadAngle = 5.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret AI|Muzzle Component", meta = (ClampMin = 0.f, UIMin = 0.f))
	float WeaponRange = 5000.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret AI|Muzzle Component", meta = (ClampMin = 0.f, UIMin = 0.f))
	float WeaponMaxDamage = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret AI|Muzzle Component")
	UCurveFloat* WeaponDamageFallOff;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret AI|Muzzle Component")
	TSoftClassPtr<UDamageType> DamageTypeClass = UBulletDamageType::StaticClass();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret AI|Muzzle Component")
	UNiagaraSystem* MuzzleFlashFX;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret AI|Muzzle Component")
	UNiagaraSystem* BulletTraceFX;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret AI|Muzzle Component")
	FName TraceEndParamName = "TraceEnd";
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret AI|Muzzle Component")
	FDecalInfo DefaultDecalInfo;

private:
	APawn* GetOwningPawn() const;
	AController* GetController() const;
	void Shoot(AController* Controller) const;
	FVector GetShotDirection(FRotator MuzzleRotation) const;
	void ProcessHit(FVector MovementDirection, const FHitResult& HitResult) const;

	FTimerHandle FireTimer;
};
