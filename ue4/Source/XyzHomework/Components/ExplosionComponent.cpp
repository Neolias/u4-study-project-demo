// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "Components/ExplosionComponent.h"

#include "Actors/Projectiles/XyzProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

UExplosionComponent::UExplosionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UExplosionComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (GetOwner()->GetLocalRole() == ROLE_Authority)
	{
		GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UExplosionComponent::OnDamageTaken);
	}
}

void UExplosionComponent::Explode()
{
	Multicast_Explode();
}

APawn* UExplosionComponent::GetOwningPawn() const
{
	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (!PawnOwner)
	{
		PawnOwner = Cast<APawn>(GetOwner()->GetOwner());
	}
	return PawnOwner;
}

void UExplosionComponent::OnDamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType_In, AController* InstigatedBy, AActor* DamageCauser)
{
	Explode();
}

void UExplosionComponent::Multicast_Explode_Implementation()
{
	if (!DamageType.LoadSynchronous())
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (IsValid(Owner))
	{
		Owner->SetActorEnableCollision(false);
		Owner->SetActorHiddenInGame(true);
	}

	AActor* OwnerActor = Owner;
	APawn* OwningPawn = GetOwningPawn();
	AController* Controller = nullptr;
	if (IsValid(OwningPawn))
	{
		OwnerActor = OwningPawn;
		Controller = OwningPawn->GetController();
	}

	if (IsValid(OwnerActor) && OwnerActor->GetLocalRole() == ROLE_Authority)
	{
		TArray<AActor*> IgnoreActors;
		IgnoreActors.Add(Owner);
		FVector DamageLocation = GetComponentLocation();

		UGameplayStatics::ApplyRadialDamageWithFalloff(
			GetWorld(), BaseDamage, MinimumDamage, DamageLocation,
			InnerRadius, OuterRadius, DamageFalloff, DamageType.LoadSynchronous(), IgnoreActors,
			OwnerActor, Controller, ECC_Visibility);
	}

	if (ExplosionVFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionVFX, GetComponentLocation());
	}

	if (OnExplosionEvent.IsBound())
	{
		OnExplosionEvent.Broadcast();
	}
}
