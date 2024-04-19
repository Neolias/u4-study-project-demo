// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Equipment/Throwables/ThrowableItem.h"

#include "Actors/Projectiles/XyzProjectile.h"
#include "Characters/XyzBaseCharacter.h"

AThrowableItem::AThrowableItem()
{
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	StaticMesh->SetupAttachment(RootComponent);
}

void AThrowableItem::Throw()
{
	if (CachedBaseCharacterOwner.IsValid() && IsValid(ThrowItemAnimMontage))
	{
		const float Duration = CachedBaseCharacterOwner->PlayAnimMontage(ThrowItemAnimMontage);
		GetWorld()->GetTimerManager().SetTimer(ThrowAnimationTimer, this, &AThrowableItem::OnThrowAnimationFinished, Duration, false);
		bIsThrowing = true;
	}
}

void AThrowableItem::LaunchProjectile() const
{
	if (!CachedBaseCharacterOwner.IsValid())
	{
		return;
	}

	const AController* Controller = CachedBaseCharacterOwner->GetInstigatorController();
	if (!IsValid(Controller))
	{
		return;
	}

	const USkeletalMeshComponent* CharacterMesh = CachedBaseCharacterOwner->GetMesh();
	if (!IsValid(CharacterMesh))
	{
		return;
	}

	FVector POVLocation;
	FRotator POVRotation;
	Controller->GetPlayerViewPoint(POVLocation, POVRotation);
	const FVector ThrowableSocketLocation = CharacterMesh->GetSocketLocation(ThrowableSocketName);

	const FVector OffsetFromPOV = ThrowableSocketLocation - POVLocation;
	const FVector POVForwardVector = POVRotation.Vector();
	const float ForwardOffsetFromPOV = FVector::DotProduct(OffsetFromPOV, POVForwardVector);
	const FVector SpawnLocation = POVLocation + POVForwardVector * ForwardOffsetFromPOV;

	AXyzProjectile* Projectile = GetWorld()->SpawnActor<AXyzProjectile>(ProjectileClass, SpawnLocation, POVRotation);
	if (IsValid(Projectile))
	{
		const FRotator LaunchRotation = POVRotation + FRotator(LaunchPitchAngle, LaunchYawAngle, 0.f);
		const FVector LaunchDirection = LaunchRotation.Vector();
		Projectile->SetOwner(GetOwner());
		Projectile->Launch(LaunchDirection);
	}

	OnThrowEnd();
}

void AThrowableItem::OnThrowEnd() const
{
	if (OnThrowEndEvent.IsBound())
	{
		OnThrowEndEvent.Broadcast();
	}
}

void AThrowableItem::OnThrowAnimationFinished()
{
	if (OnThrowAnimationFinishedEvent.IsBound())
	{
		OnThrowAnimationFinishedEvent.Broadcast();
	}
	bIsThrowing = false;
}
