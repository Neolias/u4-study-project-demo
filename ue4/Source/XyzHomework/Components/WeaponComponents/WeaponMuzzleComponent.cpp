// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/WeaponComponents/WeaponMuzzleComponent.h"

#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/DecalComponent.h"

#include "DrawDebugHelpers.h"
#include "XyzHomeworkTypes.h"
#include "Subsystems/DebugSubsystem.h"

UWeaponMuzzleComponent::UWeaponMuzzleComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWeaponMuzzleComponent::Shoot(const FWeaponModeParameters* WeaponModeParameters, const FVector ViewPointLocation, const FRotator ViewPointRotation, AController* Controller)
{
	if (!WeaponModeParameters)
	{
		return;
	}
	ModeParameters = WeaponModeParameters;

	FHitResult HitResult;
	const FVector MuzzleLocation = GetComponentLocation();
	const FVector EndLocation = ViewPointLocation + ViewPointRotation.Vector() * ModeParameters->WeaponRange;
	FVector TraceEnd = EndLocation;

	switch (ModeParameters->HitRegistrationType)
	{
	case EHitRegistrationType::Projectile:
		ShootProjectile(MuzzleLocation, ViewPointLocation, ViewPointRotation, EndLocation);
		break;
	case EHitRegistrationType::HitScan:
	default:
		TraceEnd = ShootHitScan(ViewPointLocation, ViewPointRotation, Controller, HitResult, MuzzleLocation, EndLocation);
		break;
	}

	if (IsValid(ModeParameters->BulletTraceFX))
	{
		UNiagaraComponent* BulletTraceFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ModeParameters->BulletTraceFX, GetComponentLocation(), GetComponentRotation());
		BulletTraceFXComponent->SetVectorParameter(ModeParameters->TraceEndParamName, TraceEnd);
	}
}

APawn* UWeaponMuzzleComponent::GetOwningPawn() const
{
	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (!IsValid(PawnOwner))
	{
		PawnOwner = Cast<APawn>(GetOwner()->GetOwner());
	}
	return PawnOwner;
}

void UWeaponMuzzleComponent::ShootProjectile(const FVector MuzzleLocation, const FVector ViewPointLocation, const FRotator ViewPointRotation, const FVector EndLocation) const
{
	const FVector OffsetFromPOV = MuzzleLocation - ViewPointLocation;
	const FVector POVForwardVector = ViewPointRotation.Vector();
	const float ForwardOffsetFromPOV = FVector::DotProduct(OffsetFromPOV, POVForwardVector);
	const FVector StartLocation = ViewPointLocation + POVForwardVector * ForwardOffsetFromPOV;
	const FVector LaunchDirection = (EndLocation - StartLocation).GetSafeNormal();
	AXyzProjectile* Projectile = GetWorld()->SpawnActor<AXyzProjectile>(ModeParameters->ProjectileClass, StartLocation, LaunchDirection.ToOrientationRotator());
	if (IsValid(Projectile))
	{
		Projectile->SetOwner(GetOwningPawn());
		Projectile->OnCollisionComponentHitEvent.AddDynamic(this, &UWeaponMuzzleComponent::ProcessHit);
		Projectile->Launch(LaunchDirection);
	}
}

FVector UWeaponMuzzleComponent::ShootHitScan(const FVector ViewPointLocation, const FRotator ViewPointRotation, AController* Controller, FHitResult& HitResult,
	const FVector MuzzleLocation, const FVector EndLocation)
{
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(GetOwningPawn());

	if (GetWorld()->LineTraceSingleByChannel(HitResult, ViewPointLocation, EndLocation, ECC_Bullet, CollisionQueryParams))
	{
#if (UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT) && ENABLE_DRAW_DEBUG
		const UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
		if (DebugSubsystem->IsCategoryEnabled(DebugCategoryRangedWeapon))
		{
			DrawDebugLine(GetWorld(), ViewPointLocation, HitResult.ImpactPoint, FColor::Red, false, 2.f, 0, 1.f);
		}
#endif

		FPointDamageEvent DamageEvent;
		DamageEvent.HitInfo = HitResult;
		DamageEvent.ShotDirection = ViewPointRotation.Vector();
		DamageEvent.DamageTypeClass = ModeParameters->DamageTypeClass;
		AActor* DamagedActor = HitResult.GetActor();
		if (IsValid(DamagedActor))
		{
			float DamageFallOff = 0.f;
			if (IsValid(ModeParameters->WeaponDamageFallOff))
			{
				const float Distance = FVector::Dist(MuzzleLocation, HitResult.ImpactPoint);
				DamageFallOff = ModeParameters->WeaponDamageFallOff->GetFloatValue(Distance / ModeParameters->WeaponRange);
			}

			DamagedActor->TakeDamage(ModeParameters->WeaponMaxDamage * DamageFallOff, DamageEvent, Controller, GetOwner());
		}

		ProcessHit(HitResult.ImpactPoint - MuzzleLocation, HitResult);

		return HitResult.ImpactPoint;
	}
	return EndLocation;
}

void UWeaponMuzzleComponent::ProcessHit(const FVector MovementDirection, const FHitResult& HitResult)
{
#if (UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT) && ENABLE_DRAW_DEBUG
	const UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	if (DebugSubsystem->IsCategoryEnabled(DebugCategoryRangedWeapon))
	{
		DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 10.f, 12, FColor::Red, false, 2.f);
	}
#endif

	if (!ModeParameters)
	{
		return;
	}

	FDecalInfo DecalInfo = ModeParameters->DefaultDecalInfo;
	if (IsValid(DecalInfo.DecalMaterial))
	{
		UDecalComponent* DecalComponent = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), DecalInfo.DecalMaterial, DecalInfo.DecalSize, HitResult.ImpactPoint, HitResult.ImpactNormal.ToOrientationRotator());
		if (IsValid(DecalComponent))
		{
			DecalComponent->SetFadeScreenSize(0.0001f);
			DecalComponent->SetFadeOut(DecalInfo.DecalLifeTime, DecalInfo.DecalFadeOutTime);
		}
	}

	FPointDamageEvent DamageEvent;
	DamageEvent.HitInfo = HitResult;
	DamageEvent.ShotDirection = MovementDirection;
	DamageEvent.DamageTypeClass = ModeParameters->DamageTypeClass;
	AActor* DamagedActor = HitResult.GetActor();
	if (IsValid(DamagedActor))
	{
		float DamageFallOff = 0.f;
		if (IsValid(ModeParameters->WeaponDamageFallOff))
		{
			const float Distance = FVector::Dist(GetComponentLocation(), HitResult.ImpactPoint);
			DamageFallOff = ModeParameters->WeaponDamageFallOff->GetFloatValue(Distance / ModeParameters->WeaponRange);
		}

		APawn* PawnOwner = GetOwningPawn();
		if (!IsValid(PawnOwner))
		{
			return;
		}
		AController* Controller = PawnOwner->GetController();
		if (IsValid(Controller))
		{
			DamagedActor->TakeDamage(ModeParameters->WeaponMaxDamage * DamageFallOff, DamageEvent, Controller, PawnOwner);
		}
	}
}
