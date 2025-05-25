// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "Actors/Projectiles/ExplosiveProjectile.h"

#include "Components/ExplosionComponent.h"

AExplosiveProjectile::AExplosiveProjectile()
{
	ExplosionComponent = CreateDefaultSubobject<UExplosionComponent>(TEXT("ExplosionComponent"));
	ExplosionComponent->SetupAttachment(RootComponent);
}

void AExplosiveProjectile::Explode()
{
	GetWorld()->GetTimerManager().ClearTimer(DetonationTimer);
	ExplosionComponent->Explode();
	if (OnProjectileExplosionEvent.IsBound())
	{
		OnProjectileExplosionEvent.Broadcast(this, ResetLocation);
	}
}

void AExplosiveProjectile::OnProjectileLaunched()
{
	Super::OnProjectileLaunched();

	if (!bShouldDetonateOnCollision)
	{
		GetWorld()->GetTimerManager().SetTimer(DetonationTimer, this, &AExplosiveProjectile::OnDetonationTimerElapsed, DetonationTime, false);
	}
}

void AExplosiveProjectile::OnDetonationTimerElapsed()
{
	Explode();
}

void AExplosiveProjectile::OnCollisionComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bShouldDetonateOnCollision)
	{
		OnDetonationTimerElapsed();
	}
}
