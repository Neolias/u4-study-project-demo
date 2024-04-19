// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectiles/XyzProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AXyzProjectile::AXyzProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetCollisionProfileName("BlockAllDynamic");
	CollisionComponent->InitSphereRadius(5.f);
	RootComponent = CollisionComponent;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(RootComponent);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->SetUpdatedComponent(CollisionComponent);
	ProjectileMovementComponent->MaxSpeed = MaxSpeed;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = true;
	ProjectileMovementComponent->Bounciness = Bounciness;
}

void AXyzProjectile::BeginPlay()
{
	Super::BeginPlay();
	CollisionComponent->OnComponentHit.AddDynamic(this, &AXyzProjectile::OnCollisionComponentHit);
}

void AXyzProjectile::Launch(const FVector Direction)
{
	CollisionComponent->IgnoreActorWhenMoving(GetOwner(), true);
	ProjectileMovementComponent->MaxSpeed = MaxSpeed;
	ProjectileMovementComponent->Velocity = LaunchSpeed * Direction;
	ProjectileMovementComponent->Bounciness = Bounciness;


	OnProjectileLaunched();
}

void AXyzProjectile::OnCollisionComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OnCollisionComponentHitEvent.IsBound())
	{
		OnCollisionComponentHitEvent.Broadcast(ProjectileMovementComponent->Velocity.GetSafeNormal(), Hit);
	}
}
