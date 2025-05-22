#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Actors/Projectiles/XyzProjectile.h"
#include "DamageTypes/Weapons/BulletDamageType.h"
#include "XyzGenericStructs.generated.h"

class UNiagaraSystem;
class UCurveVector;

USTRUCT(BlueprintType)
struct FMantlingMovementParameters
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector InitialLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRotator InitialRotation = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector TargetLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRotator TargetRotation = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UPrimitiveComponent* TargetActor;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector InitialAnimationLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0.f, UIMin = 0.f))
	float Duration = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0.f, UIMin = 0.f))
	float StartTime = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveVector* MantlingCurve;
};

USTRUCT(BlueprintType)
struct FLedgeDescription
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector TargetLocation;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRotator TargetRotation;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector LedgeNormal;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UPrimitiveComponent* LedgeActor;
};

USTRUCT(BlueprintType)
struct FMantlingSettings
{
	GENERATED_BODY();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* MantlingMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* FPMantlingMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveVector* MantlingCurve;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AnimationCorrectionXY = 65.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AnimationCorrectionZ = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0.f, UIMin = 0.f))
	float MaxHeight = 225.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0.f, UIMin = 0.f))
	float MinHeight = 125.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0.f, UIMin = 0.f))
	float MaxHeightStartTime = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0.f, UIMin = 0.f))
	float MinHeightStartTime = 0.6f;
};

USTRUCT(BlueprintType)
struct FDecalInfo
{
	GENERATED_BODY();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterialInterface* DecalMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector DecalSize = FVector(5.0f, 5.0f, 5.0f);
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DecalLifeTime = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DecalFadeOutTime = 5.0f;
};

USTRUCT(BlueprintType)
struct FWeaponModeParameters
{
	GENERATED_BODY();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EWeaponAmmoType AmmoType = EWeaponAmmoType::Pistol;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EHitRegistrationType HitRegistrationType = EHitRegistrationType::HitScan;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "HitRegistrationType == EHitRegistrationType::Projectile"))
	TSoftClassPtr<AXyzProjectile> ProjectileClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 1, UIMin = 1))
	int32 MagazineSize = 10;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EWeaponReloadType ReloadType = EWeaponReloadType::ByClip;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EWeaponFireMode FireMode = EWeaponFireMode::Single;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0.f, UIMin = 0.f))
	float FireRate = 600.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 1, UIMin = 1))
	int32 AmmoPerShot = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0.f, UIMin = 0.f))
	float FirstShotDelay = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0.f, UIMin = 0.f))
	float MaxBulletSpreadAngle = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0.f, UIMin = 0.f))
	float WeaponRange = 5000.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0.f, UIMin = 0.f))
	float WeaponMaxDamage = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* WeaponDamageFallOff;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<UDamageType> DamageTypeClass = UBulletDamageType::StaticClass();
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UNiagaraSystem* MuzzleFlashFX;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UNiagaraSystem* BulletTraceFX;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName TraceEndParamName = "TraceEnd";
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FDecalInfo DefaultDecalInfo;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* WeaponReloadAnimMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* CharacterReloadAnimMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* IronsightsWeaponReloadAnimMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* IronsightsCharacterReloadAnimMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ReloadLoopStartSectionName = "ReloadLoopStart";
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ReloadEndSectionName = "ReloadEnd";
};

USTRUCT(BlueprintType)
struct FMeleeAttackDescription
{
	GENERATED_BODY();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0.f, UIMin = 0.f))
	float DamageAmount = 50.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<UDamageType> DamageTypeClass = UDamageType::StaticClass();
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* AnimMontage;
};
