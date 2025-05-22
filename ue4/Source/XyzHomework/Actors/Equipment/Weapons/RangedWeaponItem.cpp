// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Equipment/Weapons/RangedWeaponItem.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

#include "Characters/PlayerCharacter.h"
#include "Components/WeaponComponents/WeaponMuzzleComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

ARangedWeaponItem::ARangedWeaponItem()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("WeaponMeshComponent");
	MeshComponent->SetupAttachment(RootComponent);
	MuzzleComponent = CreateDefaultSubobject<UWeaponMuzzleComponent>("WeaponMuzzleComponent");
	MuzzleComponent->SetupAttachment(MeshComponent, MuzzleSocketName);

	EquipmentItemType = EEquipmentItemType::Pistol;
	ReticleType = EReticleType::Default;

	CurrentWeaponAmmo.AddZeroed((uint32)EWeaponAmmoType::Max);

	if (WeaponModes.Num() == 0)
	{
		WeaponModes.Add(FWeaponModeParameters());
		CurrentWeaponModeIndex = 0;
	}
	else if (DefaultWeaponModeIndex >= 0 && DefaultWeaponModeIndex < WeaponModes.Num())
	{
		CurrentWeaponModeIndex = DefaultWeaponModeIndex;
	}
	CurrentWeaponMode = &WeaponModes[CurrentWeaponModeIndex];
}

void ARangedWeaponItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARangedWeaponItem, CurrentWeaponAmmo);
	DOREPLIFETIME(ARangedWeaponItem, CurrentWeaponModeIndex);
}

const FWeaponModeParameters* ARangedWeaponItem::GetWeaponModeParameters(int32 ModeIndex/* = -1*/) const
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

EWeaponAmmoType ARangedWeaponItem::GetAmmoType()
{
	return CurrentWeaponMode ? CurrentWeaponMode->AmmoType : EWeaponAmmoType::None;
}

int32 ARangedWeaponItem::GetMagazineSize() const
{
	return CurrentWeaponMode ? CurrentWeaponMode->MagazineSize : 1;
}

EWeaponReloadType ARangedWeaponItem::GetReloadType() const
{
	return CurrentWeaponMode ? CurrentWeaponMode->ReloadType : EWeaponReloadType::ByClip;
}

FName ARangedWeaponItem::GetReloadLoopStartSectionName() const
{
	return CurrentWeaponMode ? CurrentWeaponMode->ReloadLoopStartSectionName : FName("ReloadLoopStart");
}

FName ARangedWeaponItem::GetReloadEndSectionName() const
{
	return CurrentWeaponMode ? CurrentWeaponMode->ReloadEndSectionName : FName("ReloadEnd");
}

UAnimMontage* ARangedWeaponItem::GetWeaponReloadAnimMontage() const
{
	return CurrentWeaponMode ? CurrentWeaponMode->IronsightsWeaponReloadAnimMontage : nullptr;
}

UAnimMontage* ARangedWeaponItem::GetCharacterReloadAnimMontage() const
{
	return CurrentWeaponMode ? CurrentWeaponMode->IronsightsCharacterReloadAnimMontage : nullptr;
}

int32 ARangedWeaponItem::GetCurrentAmmo()
{
	return CurrentWeaponMode ? CurrentWeaponAmmo[(int32)CurrentWeaponMode->AmmoType] : 0;
}

void ARangedWeaponItem::SetCurrentWeaponMode(int32 ModeIndex, bool bIsAuthority/* = false*/)
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (bIsAuthority || (IsValid(BaseCharacter) && BaseCharacter->IsLocallyControlled()))
	{
		Server_SetCurrentWeaponMode(ModeIndex);
	}
}

void ARangedWeaponItem::SetCurrentAmmo(int32 AmmoAmount)
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (!CurrentWeaponMode || !IsValid(BaseCharacter) || BaseCharacter->GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	CurrentWeaponAmmo[(int32)CurrentWeaponMode->AmmoType] = AmmoAmount;
	OnAmmoChanged();
}

bool ARangedWeaponItem::CanFire() const
{
	return bIsEquipped && !IsFiring() && CurrentWeaponMode && CurrentWeaponAmmo[(int32)CurrentWeaponMode->AmmoType] > 0;
}

void ARangedWeaponItem::StartFire()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (!IsValid(BaseCharacter) || !CanFire())
	{
		return;
	}

	if (BaseCharacter->IsLocallyControlled())
	{
		bool bIsLooping = CurrentWeaponMode->FireMode == EWeaponFireMode::FullAuto ? true : false;
		GetWorld()->GetTimerManager().SetTimer(OneShotTimer, [=]() { MakeOneShot(); }, 60.f / CurrentWeaponMode->FireRate, bIsLooping, CurrentWeaponMode->FirstShotDelay);
	}

	bIsFiring = true;
}

void ARangedWeaponItem::StopFire()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (IsFiring() && IsValid(BaseCharacter) && BaseCharacter->IsLocallyControlled())
	{
		if (CurrentWeaponMode->FireMode == EWeaponFireMode::FullAuto)
		{
			bIsFiring = false;
			GetWorld()->GetTimerManager().ClearTimer(OneShotTimer);
		}
	}
	else
	{
		bIsFiring = false;
	}
}

void ARangedWeaponItem::MakeOneShot()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (!IsValid(BaseCharacter) || !CurrentWeaponMode)
	{
		return;
	}

	const AController* Controller = BaseCharacter->GetController();
	if (!IsValid(Controller))
	{
		return;
	}

	FVector ViewPointLocation;
	FRotator ViewPointRotation;
	Controller->GetPlayerViewPoint(ViewPointLocation, ViewPointRotation);
	TArray<FShotInfo> ShotInfoArray;
	for (int32 Ammo = 0; Ammo < CurrentWeaponMode->AmmoPerShot; Ammo++)
	{
		FRotator ShotDirection = GetShotDirection(ViewPointRotation);
		ShotInfoArray.Emplace(ViewPointLocation, ShotDirection.Vector());
	}

	Server_OnMakeOneShot(ShotInfoArray);
}

FRotator ARangedWeaponItem::GetShotDirection(FRotator ViewPointRotation) const
{
	float MaxSpreadAngle = CurrentWeaponMode ? CurrentWeaponMode->MaxBulletSpreadAngle : 0.f;

	FRotator ShotDirection = ViewPointRotation;
	float SpreadAngle = FMath::FRandRange(0.f, MaxSpreadAngle);
	float SpreadRoll = FMath::FRandRange(0.f, 360.f);
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

void ARangedWeaponItem::StartReload()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (!IsValid(BaseCharacter))
	{
		return;
	}

	StopFire();

	if (CurrentWeaponMode)
	{
		UAnimInstance* CharAnimInstance = BaseCharacter->GetMesh()->GetAnimInstance();
		if (CharAnimInstance && CurrentWeaponMode->IronsightsCharacterReloadAnimMontage)
		{
			CharAnimInstance->Montage_Play(CurrentWeaponMode->IronsightsCharacterReloadAnimMontage);
			FOnMontageEnded OnMontageEndedDelegate;
			OnMontageEndedDelegate.BindLambda([this](UAnimMontage* Montage, bool bInterrupted)
			{
				if (!bInterrupted)
				{
					OnReloadComplete();
				}
			});
			CharAnimInstance->Montage_SetEndDelegate(OnMontageEndedDelegate, CurrentWeaponMode->IronsightsCharacterReloadAnimMontage);

			UAnimInstance* WeaponAnimInstance = MeshComponent->GetAnimInstance();
			if (WeaponAnimInstance && CurrentWeaponMode->IronsightsWeaponReloadAnimMontage)
			{
				WeaponAnimInstance->Montage_Play(CurrentWeaponMode->IronsightsWeaponReloadAnimMontage);
			}
			return;
		}
	}

	OnReloadComplete();
}

void ARangedWeaponItem::StopReload()
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (CurrentWeaponMode && CurrentWeaponMode->IronsightsWeaponReloadAnimMontage && IsValid(BaseCharacter))
	{
		BaseCharacter->StopAnimMontage(CurrentWeaponMode->IronsightsCharacterReloadAnimMontage);
		if (UAnimInstance* WeaponAnimInstance = MeshComponent->GetAnimInstance())
		{
			WeaponAnimInstance->Montage_Stop(0.f, CurrentWeaponMode->IronsightsWeaponReloadAnimMontage);
		}
	}
}

void ARangedWeaponItem::OnReloadComplete()
{
	if (OnWeaponReloadedEvent.IsBound())
	{
		OnWeaponReloadedEvent.Broadcast();
	}
}

//@ SaveSubsystemInterface
void ARangedWeaponItem::OnLevelDeserialized_Implementation()
{
	Super::OnLevelDeserialized_Implementation();
	OnAmmoChanged();
}

//~ SaveSubsystemInterface

void ARangedWeaponItem::OnAmmoChanged()
{
	if (OnAmmoChangedEvent.IsBound())
	{
		OnAmmoChangedEvent.Broadcast(CurrentWeaponAmmo[(int32)CurrentWeaponMode->AmmoType]);
	}
}

void ARangedWeaponItem::OnMakeOneShot(const TArray<FShotInfo>& ShotInfoArray)
{
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (!IsValid(BaseCharacter) || !CurrentWeaponMode)
	{
		return;
	}

	if (CurrentWeaponAmmo[(int32)CurrentWeaponMode->AmmoType] <= 0)
	{
		return;
	}

	StopReload();
	if (CharacterFireMontage)
	{
		BaseCharacter->PlayAnimMontage(CharacterFireMontage);
	}

	UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
	if (AnimInstance && WeaponFireMontage)
	{
		AnimInstance->Montage_Play(WeaponFireMontage);
	}

	if (UNiagaraSystem* MuzzleFlashFX = CurrentWeaponMode->MuzzleFlashFX)
	{
		if (CurrentWeaponMode->FireMode == EWeaponFireMode::Single)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), MuzzleFlashFX, MuzzleComponent->GetComponentLocation());
		}

		for (const FShotInfo& ShotInfo : ShotInfoArray)
		{
			if (CurrentWeaponMode->FireMode == EWeaponFireMode::FullAuto)
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), MuzzleFlashFX, MuzzleComponent->GetComponentLocation());
			}
			MuzzleComponent->Shoot(CurrentWeaponMode, ShotInfo.GetLocation(), ShotInfo.GetDirection().ToOrientationRotator());
		}
	}

	if (CurrentWeaponMode->FireMode == EWeaponFireMode::Single)
	{
		GetWorld()->GetTimerManager().SetTimer(OnShotEndTimer, this, &ARangedWeaponItem::OnShotEnd, 60.f / CurrentWeaponMode->FireRate, false);
	}

	SetCurrentAmmo(CurrentWeaponAmmo[(int32)CurrentWeaponMode->AmmoType] - 1);
	if (CurrentWeaponAmmo[(int32)CurrentWeaponMode->AmmoType] <= 0 && OnMagazineEmptyEvent.IsBound())
	{
		OnMagazineEmptyEvent.Broadcast();
	}
}

void ARangedWeaponItem::Server_OnMakeOneShot_Implementation(const TArray<FShotInfo>& ShotInfoArray)
{
	Multicast_OnMakeOneShot(ShotInfoArray);
}

void ARangedWeaponItem::Multicast_OnMakeOneShot_Implementation(const TArray<FShotInfo>& ShotInfoArray)
{
	OnMakeOneShot(ShotInfoArray);
}

void ARangedWeaponItem::OnShotEnd()
{
	bIsFiring = false;
	if (OnShotEndEvent.IsBound())
	{
		OnShotEndEvent.Broadcast();
	}
}

void ARangedWeaponItem::Server_SetCurrentWeaponMode_Implementation(int32 ModeIndex)
{
	if (ModeIndex >= 0 && ModeIndex < WeaponModes.Num())
	{
		CurrentWeaponModeIndex = ModeIndex;
	}
	else
	{
		CurrentWeaponModeIndex = 0;
	}
	CurrentWeaponMode = &WeaponModes[CurrentWeaponModeIndex];
	OnAmmoChanged();
}

void ARangedWeaponItem::OnRep_CurrentWeaponModeIndex()
{
	if (CurrentWeaponModeIndex >= 0 && CurrentWeaponModeIndex < WeaponModes.Num())
	{
		CurrentWeaponMode = &WeaponModes[CurrentWeaponModeIndex];
		OnAmmoChanged();
	}
	else
	{
		SetCurrentWeaponMode(0);
	}
}

void ARangedWeaponItem::OnRep_CurrentWeaponAmmo(TArray<int32> CurrentWeaponAmmo_Old)
{
	OnAmmoChanged();
}
