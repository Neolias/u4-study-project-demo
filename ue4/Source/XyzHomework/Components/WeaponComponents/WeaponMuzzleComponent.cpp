// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/WeaponComponents/WeaponMuzzleComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "DrawDebugHelpers.h"
#include "GameplayEffect.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "XyzHomeworkTypes.h"
#include "Actors/Projectiles/ExplosiveProjectile.h"
#include "Components/DecalComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/DebugSubsystem.h"

UWeaponMuzzleComponent::UWeaponMuzzleComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UWeaponMuzzleComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwningPawn();
	if (IsValid(Owner) && Owner->GetLocalRole() == ROLE_Authority)
	{
		InstantiateProjectilePools(Owner);
	}
}

void UWeaponMuzzleComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UWeaponMuzzleComponent, ProjectilePools);
}

void UWeaponMuzzleComponent::Shoot(FWeaponModeParameters* WeaponModeParameters, FVector ViewPointLocation, FRotator ViewPointRotation)
{
	if (!WeaponModeParameters)
	{
		return;
	}
	ModeParameters = WeaponModeParameters;

	FHitResult HitResult;
	FVector MuzzleLocation = GetComponentLocation();
	FVector EndLocation = ViewPointLocation + ViewPointRotation.Vector() * ModeParameters->WeaponRange;

	switch (ModeParameters->HitRegistrationType)
	{
		case EHitRegistrationType::Projectile:
			ShootProjectile(WeaponModeParameters->ProjectileClass, MuzzleLocation, ViewPointLocation, ViewPointRotation, EndLocation);
			break;
		case EHitRegistrationType::HitScan:
		default:
			FVector TraceEnd = ShootHitScan(ViewPointLocation, ViewPointRotation, HitResult, MuzzleLocation, EndLocation);
			if (ModeParameters->BulletTraceFX)
			{
				UNiagaraComponent* BulletTraceFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ModeParameters->BulletTraceFX, GetComponentLocation(), GetComponentRotation());
				if (BulletTraceFXComponent)
				{
					BulletTraceFXComponent->SetVectorParameter(ModeParameters->TraceEndParamName, TraceEnd);
				}
			}
			break;
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

AController* UWeaponMuzzleComponent::GetController() const
{
	const APawn* PawnOwner = GetOwningPawn();
	return IsValid(PawnOwner) ? PawnOwner->GetController() : nullptr;
}

void UWeaponMuzzleComponent::InstantiateProjectilePools(AActor* Owner)
{
	for (FProjectilePool& Pool : ProjectilePools)
	{
		Pool.InstantiatePool(GetWorld(), Owner);
	}
}

void UWeaponMuzzleComponent::ShootProjectile(TSoftClassPtr<AXyzProjectile> ProjectileClass, FVector MuzzleLocation, FVector ViewPointLocation, FRotator ViewPointRotation, FVector EndLocation)
{
	FProjectilePool* ProjectilePool = ProjectilePools.FindByPredicate([ProjectileClass](const FProjectilePool& Pool) { return Pool.ProjectileClass == ProjectileClass; });
	if (!ProjectilePool)
	{
		return;
	}

	AXyzProjectile* Projectile = ProjectilePool->GetNextAvailableProjectile();
	if (IsValid(Projectile))
	{
		FVector OffsetFromPOV = MuzzleLocation - ViewPointLocation;
		FVector POVForwardVector = ViewPointRotation.Vector();
		float ForwardOffsetFromPOV = FVector::DotProduct(OffsetFromPOV, POVForwardVector);
		FVector StartLocation = ViewPointLocation + POVForwardVector * ForwardOffsetFromPOV;
		FVector LaunchDirection = (EndLocation - StartLocation).GetSafeNormal();

		OnShootProjectile(Projectile, StartLocation, LaunchDirection, ProjectilePool->PoolWorldLocation);
	}
}

void UWeaponMuzzleComponent::OnShootProjectile(AXyzProjectile* Projectile, FVector StartLocation, FVector LaunchDirection, FVector ResetLocation)
{
	Projectile->SetActorLocation(StartLocation);
	Projectile->SetActorRotation(LaunchDirection.ToOrientationRotator());
	Projectile->SetProjectileActive(true);

	AExplosiveProjectile* ExplosiveProjectile = Cast<AExplosiveProjectile>(Projectile);
	if (IsValid(ExplosiveProjectile))
	{
		ExplosiveProjectile->OnProjectileExplosionEvent.AddUObject(this, &UWeaponMuzzleComponent::OnProjectileExplosion);
	}
	else
	{
		Projectile->OnCollisionComponentHitEvent.AddUObject(this, &UWeaponMuzzleComponent::ProcessProjectileHit);
	}

	Projectile->Launch(LaunchDirection, ResetLocation);
}

FVector UWeaponMuzzleComponent::ShootHitScan(FVector ViewPointLocation, FRotator ViewPointRotation, FHitResult& HitResult, FVector MuzzleLocation, FVector EndLocation) const
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

		ProcessHit(HitResult.ImpactPoint - MuzzleLocation, HitResult);

		return HitResult.ImpactPoint;
	}
	return EndLocation;
}

void UWeaponMuzzleComponent::ProcessHit(FVector MovementDirection, const FHitResult& HitResult) const
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
	if (DecalInfo.DecalMaterial)
	{
		UDecalComponent* DecalComponent = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), DecalInfo.DecalMaterial, DecalInfo.DecalSize, HitResult.ImpactPoint, HitResult.ImpactNormal.ToOrientationRotator());
		if (DecalComponent)
		{
			DecalComponent->SetFadeScreenSize(0.0001f);
			DecalComponent->SetFadeOut(DecalInfo.DecalLifeTime, DecalInfo.DecalFadeOutTime);
		}
	}

	APawn* PawnOwner = GetOwningPawn();
	if (!IsValid(PawnOwner) || PawnOwner->GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	// Two ways to apply damage: via GAS Effects or using the default UE damage system
	// Currently the project uses a mix of both so we check for IAbilitySystemInterface
	AActor* DamagedActor = HitResult.GetActor();
	if (IsValid(DamagedActor))
	{
		IAbilitySystemInterface* AbilitySystemActor = Cast<IAbilitySystemInterface>(DamagedActor);
		if (AbilitySystemActor && DamageEffectClass.LoadSynchronous())
		{
			UAbilitySystemComponent* AbilitySystem = AbilitySystemActor->GetAbilitySystemComponent();
			UGameplayEffect* DamageEffect = DamageEffectClass.LoadSynchronous()->GetDefaultObject<UGameplayEffect>();
			FGameplayEffectSpec EffectSpec(DamageEffect, FGameplayEffectContextHandle(new FGameplayEffectContext(GetController(), PawnOwner)));
			EffectSpec.SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(AbilitiesAttributeHealth), -ModeParameters->WeaponMaxDamage);

			AbilitySystem->ApplyGameplayEffectSpecToSelf(EffectSpec);

			// Used to trigger AI perception
			DamagedActor->TakeDamage(0.0001f, FPointDamageEvent(), GetController(), PawnOwner);
		}
		else
		{
			float DamageFallOff = 0.f;
			if (ModeParameters->WeaponDamageFallOff)
			{
				float Distance = FVector::Dist(GetComponentLocation(), HitResult.ImpactPoint);
				DamageFallOff = ModeParameters->WeaponDamageFallOff->GetFloatValue(Distance / ModeParameters->WeaponRange);
			}

			FPointDamageEvent DamageEvent;
			DamageEvent.HitInfo = HitResult;
			DamageEvent.ShotDirection = MovementDirection;
			DamageEvent.DamageTypeClass = ModeParameters->DamageTypeClass.LoadSynchronous();
			DamagedActor->TakeDamage(ModeParameters->WeaponMaxDamage * DamageFallOff, DamageEvent, GetController(), PawnOwner);
		}
	}
}

void UWeaponMuzzleComponent::ProcessProjectileHit(AXyzProjectile* Projectile, FVector MovementDirection, const FHitResult& HitResult, FVector ResetLocation)
{
	ResetProjectile(Projectile, ResetLocation);
	ProcessHit(MovementDirection, HitResult);
}

void UWeaponMuzzleComponent::OnProjectileExplosion(AExplosiveProjectile* ExplosiveProjectile, FVector ResetLocation)
{
	ExplosiveProjectile->OnProjectileExplosionEvent.RemoveAll(this);
	ResetProjectile(ExplosiveProjectile, ResetLocation);
}

void UWeaponMuzzleComponent::ResetProjectile(AXyzProjectile* Projectile, FVector ResetLocation) const
{
	Projectile->SetProjectileActive(false);
	Projectile->SetActorLocation(ResetLocation);
	Projectile->SetActorRotation(FRotator::ZeroRotator);

	Projectile->OnCollisionComponentHitEvent.RemoveAll(this);
}
