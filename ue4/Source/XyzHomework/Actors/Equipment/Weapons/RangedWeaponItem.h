// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericStructs.h"
#include "Actors/Equipment/EquipmentItem.h"
#include "RangedWeaponItem.generated.h"

class UWeaponMuzzleComponent;
/**
 *
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAmmoChanged, int32)
DECLARE_MULTICAST_DELEGATE(FOnWeaponReloaded)
DECLARE_MULTICAST_DELEGATE(FOnMagazineEmpty)

UCLASS()
class XYZHOMEWORK_API ARangedWeaponItem : public AEquipmentItem
{
	GENERATED_BODY()

public:
	FOnAmmoChanged OnAmmoChanged;
	FOnWeaponReloaded OnWeaponReloaded;
	FOnMagazineEmpty OnMagazineEmpty;

	ARangedWeaponItem();
	virtual void BeginPlay() override;
	const FWeaponModeParameters* GetWeaponModeParameters(int32 ModeIndex = -1) const;
	const TArray<FWeaponModeParameters>* GetWeaponModesArray() const { return &WeaponModes; }
	int32 GetCurrentWeaponModeIndex() const { return CurrentWeaponModeIndex; }
	int32 GetDefaultWeaponModeIndex() const { return DefaultWeaponModeIndex; }
	void ActivateWeaponMode(int32 ModeIndex);
	USkeletalMeshComponent* GetMesh() const { return MeshComponent; }
	UWeaponMuzzleComponent* GetMuzzleComponent() const { return MuzzleComponent; }
	bool IsReloading() const { return bIsReloading; }
	EWeaponAmmoType GetAmmoType() const { return CurrentWeaponMode ? CurrentWeaponMode->AmmoType : EWeaponAmmoType::None; }
	int32 GetMagazineSize() const { return CurrentWeaponMode ? CurrentWeaponMode->MagazineSize : 1; }
	EWeaponReloadType GetReloadType() const { return CurrentWeaponMode ? CurrentWeaponMode->ReloadType : EWeaponReloadType::ByClip; }
	FName GetReloadLoopStartSectionName() const { return CurrentWeaponMode ? CurrentWeaponMode->ReloadLoopStartSectionName : FName("ReloadLoopStart"); }
	FName GetReloadEndSectionName() const { return CurrentWeaponMode ? CurrentWeaponMode->ReloadEndSectionName : FName("ReloadEnd"); }
	UAnimMontage* GetWeaponReloadAnimMontage() const { return CurrentWeaponMode ? CurrentWeaponMode->IronsightsWeaponReloadAnimMontage : nullptr; }
	UAnimMontage* GetCharacterReloadAnimMontage() const { return CurrentWeaponMode ? CurrentWeaponMode->IronsightsCharacterReloadAnimMontage : nullptr; }
	int32 GetCurrentAmmo() const { return CurrentWeaponMode ? CurrentWeaponAmmo[CurrentWeaponMode->AmmoType] : 0; }
	void SetCurrentAmmo(int32 AmmoAmount);
	bool IsFiring() const { return bIsFiring; }
	bool CanFire() const;
	void StartFire();
	void StopFire();
	void MakeOneShot();
	FRotator GetShotDirection(OUT FRotator ViewPointRotation) const;
	FTransform GetForeGripSocketTransform() const;
	float GetReloadingWalkSpeed() const { return ReloadingWalkSpeed; }
	void StartAutoReload() const;
	void StartReload();
	void EndReload(bool bIsSuccess);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Parameters | Modes", meta = (ClampMin = 0, UIMin = 0))
	int32 DefaultWeaponModeIndex = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Parameters | Modes")
	TArray<FWeaponModeParameters> WeaponModes;
	UPROPERTY(EditAnywhere, Category = "Weapon Parameters | Reloading", meta = (ClampMin = 0.f, UIMin = 0.f))
	float ReloadingWalkSpeed = 200.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Parameters | Mesh")
	USkeletalMeshComponent* MeshComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Parameters | Mesh")
	FName ForeGripSocketName = "ForeGripSocket";
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Parameters | Mesh")
	FVector ForeGripOffsetFromSocket = FVector(9.f, 0.f, 0.f);
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Parameters | Muzzle Component")
	UWeaponMuzzleComponent* MuzzleComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Parameters | Muzzle Component")
	FName MuzzleSocketName = "MuzzleSocket";
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Parameters | Animations")
	UAnimMontage* CharacterFireMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Parameters | Animations")
	UAnimMontage* WeaponFireMontage;

	FTimerHandle OneShotTimer;
	FTimerHandle ReloadTimer;
	FTimerHandle OnShotEndTimer;
	bool bIsReloading = false;
	bool bIsFiring = false;
	int32 CurrentAmmo = 0;
	int32 CurrentWeaponModeIndex = 0;
	const FWeaponModeParameters* CurrentWeaponMode;
	TMap<EWeaponAmmoType, int32> CurrentWeaponAmmo;

	void Loadout();
	void OnShotEnd();
};
