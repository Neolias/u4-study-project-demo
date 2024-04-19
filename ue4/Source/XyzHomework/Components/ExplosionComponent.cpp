// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ExplosionComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

void UExplosionComponent::BeginPlay()
{
	Super::BeginPlay();
	GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UExplosionComponent::OnDamageTaken);
}

void UExplosionComponent::OnDamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType_In, AController* InstigatedBy, AActor* DamageCauser)
{
	if (IsValid(InstigatedBy))
	{
		Explode(InstigatedBy);
	}
}

void UExplosionComponent::Explode(AController* Controller) const
{
	AActor* Owner = GetOwner();
	Owner->SetActorEnableCollision(false);
	Owner->SetActorHiddenInGame(true);

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(Owner);
	const FVector DamageLocation = GetComponentLocation();

	if (OnExplosionEvent.IsBound())
	{
		OnExplosionEvent.Broadcast();
	}

	UGameplayStatics::ApplyRadialDamageWithFalloff(
		GetWorld(), BaseDamage, MinimumDamage, DamageLocation,
		InnerRadius, OuterRadius, DamageFalloff, DamageType, IgnoreActors,
		Owner, Controller, ECC_Visibility);

	if (IsValid(ExplosionVFX))
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionVFX, GetComponentLocation());
	}
}
