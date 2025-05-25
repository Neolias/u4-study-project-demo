// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "Actors/Equipment/Weapons/MeleeWeaponItem.h"

#include "XyzGenericStructs.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/WeaponComponents/MeleeHitRegistrationComponent.h"

AMeleeWeaponItem::AMeleeWeaponItem()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("WeaponMeshComponent");
	MeshComponent->SetupAttachment(RootComponent);

	EquipmentItemType = EEquipmentItemType::Knife;
}

void AMeleeWeaponItem::StartAttack(EMeleeAttackType AttackType)
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (!IsValid(BaseCharacter))
	{
		return;
	}

	CurrentAttackDescription = Attacks.Find(AttackType);
	if (!CurrentAttackDescription)
	{
		return;
	}

	if (OnAttackActivatedEvent.IsBound())
	{
		OnAttackActivatedEvent.Broadcast(true);
	}
	
	if (UAnimMontage* AttackAnimMontage = CurrentAttackDescription->AnimMontage)
	{
		HitActors.Empty();
		float Duration = BaseCharacter->PlayAnimMontage(AttackAnimMontage) / AttackAnimMontage->RateScale;
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AMeleeWeaponItem::EndAttack, Duration);
	}
	else
	{
		EndAttack();
	}
}

void AMeleeWeaponItem::EnableHitRegistration(bool bIsHitRegistrationEnabled)
{
	HitActors.Empty();
	for (UMeleeHitRegistrationComponent* HitRegistrationComponent : HitRegistrationComponents)
	{
		HitRegistrationComponent->EnableHitRegistration(bIsHitRegistrationEnabled);
	}
}

void AMeleeWeaponItem::BeginPlay()
{
	Super::BeginPlay();

	GetComponents<UMeleeHitRegistrationComponent>(HitRegistrationComponents);

	const AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsValid(BaseCharacter) && BaseCharacter->GetLocalRole() == ROLE_Authority)
	{
		for (UMeleeHitRegistrationComponent* HitRegistrationComponent : HitRegistrationComponents)
		{
			HitRegistrationComponent->OnHitRegisteredEvent.AddUObject(this, &AMeleeWeaponItem::ProcessHit);
		}
	}
}

void AMeleeWeaponItem::ProcessHit(FVector MovementDirection, const FHitResult& HitResult)
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (!CurrentAttackDescription || !IsValid(BaseCharacter))
	{
		return;
	}

	AActor* HitActor = HitResult.GetActor();
	if (IsValid(HitActor) && !HitActors.Contains(HitActor))
	{
		AController* Controller = BaseCharacter->GetController();
		if (IsValid(Controller))
		{
			FPointDamageEvent DamageEvent;
			DamageEvent.DamageTypeClass = CurrentAttackDescription->DamageTypeClass.LoadSynchronous();
			DamageEvent.HitInfo = HitResult;
			DamageEvent.ShotDirection = MovementDirection;
			HitActor->TakeDamage(CurrentAttackDescription->DamageAmount, DamageEvent, Controller, BaseCharacter);

			HitActors.Add(HitActor);
		}
	}
}

void AMeleeWeaponItem::EndAttack() const
{
	if (OnAttackActivatedEvent.IsBound())
	{
		OnAttackActivatedEvent.Broadcast(false);
	}
}
