// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Equipment/Weapons/RangedWeaponItem.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

#include "Characters/PlayerCharacter.h"
#include "GameFramework/Character.h"
#include "Components/WeaponComponents/WeaponMuzzleComponent.h"

ARangedWeaponItem::ARangedWeaponItem()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("WeaponMeshComponent");
	MeshComponent->SetupAttachment(RootComponent);

	MuzzleComponent = CreateDefaultSubobject<UWeaponMuzzleComponent>("WeaponMuzzleComponent");
	MuzzleComponent->SetupAttachment(MeshComponent, MuzzleSocketName);

	EquipmentItemType = EEquipmentItemType::Pistol;
	ReticleType = EReticleType::Default;
}

void ARangedWeaponItem::BeginPlay()
{
	Super::BeginPlay();

	Loadout();
}

void ARangedWeaponItem::Loadout()
{
	if (WeaponModes.Num() < 1)
	{
		WeaponModes.Add(FWeaponModeParameters());
		CurrentWeaponModeIndex = 0;
	}
	else if (WeaponModes.Num() - 1 >= DefaultWeaponModeIndex)
	{
		CurrentWeaponModeIndex = DefaultWeaponModeIndex;
	}
	else
	{
		CurrentWeaponModeIndex = 0;
	}

	CurrentWeaponMode = &WeaponModes[CurrentWeaponModeIndex];

	for (int i = 0; i < WeaponModes.Num(); i++)
	{
		CurrentWeaponAmmo.Add(WeaponModes[i].AmmoType, 0);
	}
}

const FWeaponModeParameters* ARangedWeaponItem::GetWeaponModeParameters(const int32 ModeIndex/* = -1*/) const
{
	if (WeaponModes.Num() < 1 || WeaponModes.Num() - 1 < ModeIndex)
	{
		return nullptr;
	}

	if (ModeIndex == -1)
	{
		return &WeaponModes[0];
	}

	return &WeaponModes[ModeIndex];
}

void ARangedWeaponItem::ActivateWeaponMode(const int32 ModeIndex)
{
	if (ModeIndex < 0 || ModeIndex > WeaponModes.Num())
	{
		return;
	}
	if (ModeIndex == WeaponModes.Num())
	{
		CurrentWeaponModeIndex = 0;
	}
	else
	{
		CurrentWeaponModeIndex = ModeIndex;
	}

	CurrentWeaponMode = &WeaponModes[CurrentWeaponModeIndex];
}

void ARangedWeaponItem::SetCurrentAmmo(const int32 AmmoAmount)
{
	if (!CurrentWeaponMode)
	{
		return;
	}

	CurrentWeaponAmmo[CurrentWeaponMode->AmmoType] = AmmoAmount;
	if (OnAmmoChanged.IsBound())
	{
		OnAmmoChanged.Broadcast(AmmoAmount);
	}
}

bool ARangedWeaponItem::CanFire() const
{
	return bIsEquipped && !bIsFiring && CurrentWeaponMode && CurrentWeaponAmmo[CurrentWeaponMode->AmmoType] > 0;
}

void ARangedWeaponItem::StartFire()
{
	if (!CachedBaseCharacterOwner.IsValid() || !CanFire())
	{
		return;
	}

	const bool bIsLooping = CurrentWeaponMode->FireMode == EWeaponFireMode::FullAuto ? true : false;

	bIsFiring = true;
	GetWorld()->GetTimerManager().ClearTimer(OneShotTimer);
	GetWorld()->GetTimerManager().SetTimer(OneShotTimer, [=]() { MakeOneShot(); }, 60.f / CurrentWeaponMode->FireRate, bIsLooping, CurrentWeaponMode->FirstShotDelay);
}

void ARangedWeaponItem::StopFire()
{
	if (CurrentWeaponMode->FireMode == EWeaponFireMode::FullAuto)
	{
		bIsFiring = false;
		GetWorld()->GetTimerManager().ClearTimer(OneShotTimer);
	}
}

void ARangedWeaponItem::MakeOneShot()
{
	if (!CachedBaseCharacterOwner.IsValid() || !CurrentWeaponMode)
	{
		return;
	}

	EndReload(false);
	SetCurrentAmmo(CurrentWeaponAmmo[CurrentWeaponMode->AmmoType] - 1);

	if (IsValid(CharacterFireMontage))
	{
		CachedBaseCharacterOwner->PlayAnimMontage(CharacterFireMontage);
	}

	UAnimInstance* WeaponAnimInstance = MeshComponent->GetAnimInstance();
	if (IsValid(WeaponAnimInstance) && IsValid(WeaponFireMontage))
	{
		WeaponAnimInstance->Montage_Play(WeaponFireMontage);
	}

	AController* Controller = CachedBaseCharacterOwner->GetController();
	if (!IsValid(Controller))
	{
		return;
	}

	FVector ViewPointLocation;
	FRotator ViewPointRotation;
	Controller->GetPlayerViewPoint(ViewPointLocation, ViewPointRotation);

	UNiagaraSystem* MuzzleFlashFX = CurrentWeaponMode->MuzzleFlashFX;
	if (IsValid(MuzzleFlashFX) && CurrentWeaponMode->FireMode == EWeaponFireMode::Single)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), MuzzleFlashFX, MuzzleComponent->GetComponentLocation());
	}

	for (int32 Ammo = 0; Ammo < CurrentWeaponMode->AmmoPerShot; Ammo++)
	{
		if (IsValid(MuzzleFlashFX) && CurrentWeaponMode->FireMode == EWeaponFireMode::FullAuto)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), MuzzleFlashFX, MuzzleComponent->GetComponentLocation());
		}
		const FRotator ShotDirection = GetShotDirection(ViewPointRotation);
		MuzzleComponent->Shoot(CurrentWeaponMode, ViewPointLocation, ShotDirection, Controller);
	}

	if (CurrentWeaponMode->FireMode == EWeaponFireMode::Single)
	{
		GetWorld()->GetTimerManager().SetTimer(OnShotEndTimer, this, &ARangedWeaponItem::OnShotEnd, 60.f / CurrentWeaponMode->FireRate, false);
	}

	if (CurrentWeaponAmmo[CurrentWeaponMode->AmmoType] == 0 && OnMagazineEmpty.IsBound())
	{
		OnMagazineEmpty.Broadcast();
	}
}

void ARangedWeaponItem::OnShotEnd()
{
	bIsFiring = false;
}

FRotator ARangedWeaponItem::GetShotDirection(const FRotator ViewPointRotation) const
{
	const float MaxSpreadAngle = CurrentWeaponMode ? CurrentWeaponMode->MaxBulletSpreadAngle : 0.f;

	FRotator ShotDirection = ViewPointRotation;
	const float SpreadAngle = FMath::FRandRange(0.f, MaxSpreadAngle);
	const float SpreadRoll = FMath::FRandRange(0.f, 360.f);
	ShotDirection.Yaw += SpreadAngle * FMath::Cos(SpreadRoll);
	ShotDirection.Pitch += SpreadAngle * FMath::Sin(SpreadRoll);
	return ShotDirection;
}

FTransform ARangedWeaponItem::GetForeGripSocketTransform() const
{
	FTransform ForeGripTransform = MeshComponent->GetSocketTransform(ForeGripSocketName, ERelativeTransformSpace::RTS_World);
	ForeGripTransform.SetLocation(ForeGripTransform.GetLocation() + GetActorRotation().RotateVector(ForeGripOffsetFromSocket));
	return ForeGripTransform;
}

void ARangedWeaponItem::StartAutoReload() const
{
	if (CachedBaseCharacterOwner.IsValid())
	{
		CachedBaseCharacterOwner->ReloadWeapon();
	}
}

void ARangedWeaponItem::StartReload()
{
	if (!CachedBaseCharacterOwner.IsValid())
	{
		return;
	}

	bIsReloading = true;
	StopFire();

	if (CurrentWeaponMode && IsValid(CurrentWeaponMode->IronsightsCharacterReloadAnimMontage))
	{
		const float Duration = CachedBaseCharacterOwner->PlayAnimMontage(CurrentWeaponMode->IronsightsCharacterReloadAnimMontage);

		UAnimInstance* WeaponAnimInstance = MeshComponent->GetAnimInstance();
		if (IsValid(WeaponAnimInstance) && IsValid(CurrentWeaponMode->IronsightsWeaponReloadAnimMontage))
		{
			WeaponAnimInstance->Montage_Play(CurrentWeaponMode->IronsightsWeaponReloadAnimMontage);
		}

		if (CurrentWeaponMode->ReloadType == EWeaponReloadType::ByClip)
		{
			GetWorld()->GetTimerManager().SetTimer(ReloadTimer, [this]() {EndReload(true); }, Duration, false);
		}
		return;
	}

	EndReload(true);
}

void ARangedWeaponItem::EndReload(const bool bIsSuccess)
{
	if (!CachedBaseCharacterOwner.IsValid())
	{
		return;
	}

	if (!bIsSuccess && CurrentWeaponMode)
	{
		if (IsValid(CurrentWeaponMode->IronsightsCharacterReloadAnimMontage))
		{
			CachedBaseCharacterOwner->StopAnimMontage(CurrentWeaponMode->IronsightsCharacterReloadAnimMontage);
		}

		UAnimInstance* WeaponAnimInstance = MeshComponent->GetAnimInstance();
		if (IsValid(WeaponAnimInstance) && IsValid(CurrentWeaponMode->IronsightsWeaponReloadAnimMontage))
		{
			WeaponAnimInstance->Montage_Stop(0.f, CurrentWeaponMode->IronsightsWeaponReloadAnimMontage);
		}
	}

	if (bIsSuccess && OnWeaponReloaded.IsBound())
	{
		OnWeaponReloaded.Broadcast();
	}

	if (CurrentWeaponMode && CurrentWeaponMode->ReloadType == EWeaponReloadType::ByClip)
	{
		GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);
	}
	bIsReloading = false;
}
