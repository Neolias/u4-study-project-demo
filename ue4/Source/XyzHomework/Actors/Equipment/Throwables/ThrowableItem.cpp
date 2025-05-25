// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "Actors/Equipment/Throwables/ThrowableItem.h"

#include "Actors/Projectiles/ExplosiveProjectile.h"
#include "Actors/Projectiles/XyzProjectile.h"
#include "Characters/XyzBaseCharacter.h"

AThrowableItem::AThrowableItem()
{
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	StaticMesh->SetupAttachment(RootComponent);
}

EWeaponAmmoType AThrowableItem::GetAmmoType()
{
	return AmmoType;
}

void AThrowableItem::Throw(AXyzProjectile* ThrowableProjectile, FVector ResetLocation)
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (!IsValid(BaseCharacter) || !IsValid(ThrowableProjectile))
	{
		return;
	}

	CurrentProjectile = ThrowableProjectile;
	ProjectileResetLocation = ResetLocation;

	if (ThrowItemAnimMontage)
	{
		float Duration = BaseCharacter->PlayAnimMontage(ThrowItemAnimMontage);
		GetWorld()->GetTimerManager().SetTimer(ThrowAnimationTimer, this, &AThrowableItem::OnThrowAnimationFinished, Duration, false);
	}
}

void AThrowableItem::LaunchProjectile()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	AXyzProjectile* Projectile = CurrentProjectile.Get();
	if (!IsValid(BaseCharacter) || !IsValid(Projectile))
	{
		return;
	}

	const AController* Controller = BaseCharacter->GetInstigatorController();
	if (!IsValid(Controller))
	{
		return;
	}

	FVector POVLocation;
	FRotator POVRotation;
	Controller->GetPlayerViewPoint(POVLocation, POVRotation);
	FVector ThrowableSocketLocation = BaseCharacter->GetMesh()->GetSocketLocation(ThrowableSocketName);
	FVector OffsetFromPOV = ThrowableSocketLocation - POVLocation;
	FVector POVForwardVector = POVRotation.Vector();
	float ForwardOffsetFromPOV = FVector::DotProduct(OffsetFromPOV, POVForwardVector);
	FVector StartLocation = POVLocation + POVForwardVector * ForwardOffsetFromPOV;
	FRotator LaunchRotation = POVRotation + FRotator(LaunchPitchAngle, LaunchYawAngle, 0.f);
	FVector LaunchDirection = LaunchRotation.Vector();

	Projectile->SetActorLocation(StartLocation);
	Projectile->SetActorRotation(LaunchDirection.ToOrientationRotator());
	Projectile->SetProjectileActive(true);
	
	if (AExplosiveProjectile* ExplosiveProjectile = Cast<AExplosiveProjectile>(Projectile))
	{
		ExplosiveProjectile->OnProjectileExplosionEvent.AddUObject(this, &AThrowableItem::OnProjectileExplosion);
	}
	else
	{
		Projectile->OnCollisionComponentHitEvent.AddUObject(this, &AThrowableItem::ProcessThrowableProjectileHit);
	}

	Projectile->Launch(LaunchDirection, ProjectileResetLocation);

	OnThrowEnd();
}

void AThrowableItem::OnThrowEnd() const
{
	if (OnItemThrownEvent.IsBound())
	{
		OnItemThrownEvent.Broadcast();
	}
}

void AThrowableItem::OnThrowAnimationFinished()
{
	if (OnThrowAnimationFinishedEvent.IsBound())
	{
		OnThrowAnimationFinishedEvent.Broadcast();
	}
}

void AThrowableItem::ProcessThrowableProjectileHit(AXyzProjectile* Projectile, FVector MovementDirection, const FHitResult& HitResult, FVector ResetLocation)
{
	ResetThrowableProjectile(Projectile, ResetLocation);
}

void AThrowableItem::OnProjectileExplosion(AExplosiveProjectile* ExplosiveProjectile, FVector ResetLocation)
{
	ExplosiveProjectile->OnProjectileExplosionEvent.RemoveAll(this);
	ResetThrowableProjectile(ExplosiveProjectile, ResetLocation);
}

void AThrowableItem::ResetThrowableProjectile(AXyzProjectile* Projectile, FVector ResetLocation)
{
	Projectile->SetProjectileActive(false);
	Projectile->SetActorLocation(ResetLocation);
	Projectile->SetActorRotation(FRotator::ZeroRotator);
	Projectile->OnCollisionComponentHitEvent.RemoveAll(this);

	CurrentProjectile.Reset();
}
