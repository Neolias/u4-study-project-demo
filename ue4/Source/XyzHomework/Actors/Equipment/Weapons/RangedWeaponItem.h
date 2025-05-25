// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericStructs.h"
#include "Actors/Equipment/EquipmentItem.h"
#include "RangedWeaponItem.generated.h"

class UWeaponMuzzleComponent;

USTRUCT(BlueprintType)
struct FShotInfo
{
	GENERATED_BODY()

	FShotInfo()
		: Location_Mul_10(FVector::ZeroVector),
		  Direction(FVector::ZeroVector) {};

	FShotInfo(FVector Location, FVector Direction)
		: Location_Mul_10(Location * 10.0f),
		  Direction(Direction) {};

	FVector GetLocation() const { return Location_Mul_10 * 0.1f; }
	FVector GetDirection() const { return Direction; }

	UPROPERTY()
	FVector_NetQuantize100 Location_Mul_10;
	UPROPERTY()
	FVector_NetQuantizeNormal Direction;
};

/** Base class of all ranged weapon items, such as pistols, rifles, and shotguns. */
UCLASS()
class XYZHOMEWORK_API ARangedWeaponItem : public AEquipmentItem
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnAmmoChangedEvent, int32)
	DECLARE_MULTICAST_DELEGATE(FOnWeaponReloadedEvent)
	DECLARE_MULTICAST_DELEGATE(FOnMagazineEmptyEvent)
	DECLARE_MULTICAST_DELEGATE(FOnShotEndEvent)
	FOnAmmoChangedEvent OnAmmoChangedEvent;
	FOnWeaponReloadedEvent OnWeaponReloadedEvent;
	FOnMagazineEmptyEvent OnMagazineEmptyEvent;
	FOnShotEndEvent OnShotEndEvent;

	ARangedWeaponItem();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OUT OutLifetimeProps) const override;
	const FWeaponModeParameters* GetWeaponModeParameters(int32 ModeIndex = -1) const;
	const TArray<FWeaponModeParameters>& GetWeaponModesArray() const { return WeaponModes; }
	int32 GetCurrentWeaponModeIndex() const { return CurrentWeaponModeIndex; }
	int32 GetDefaultWeaponModeIndex() const { return DefaultWeaponModeIndex; }
	void SetCurrentWeaponMode(int32 ModeIndex, bool bIsAuthority = false);
	USkeletalMeshComponent* GetMesh() const { return MeshComponent; }
	UWeaponMuzzleComponent* GetMuzzleComponent() const { return MuzzleComponent; }
	virtual EWeaponAmmoType GetAmmoType() override;
	int32 GetMagazineSize() const;
	EWeaponReloadType GetReloadType() const;
	FName GetReloadLoopStartSectionName() const;
	FName GetReloadEndSectionName() const;
	UAnimMontage* GetWeaponReloadAnimMontage() const;
	UAnimMontage* GetCharacterReloadAnimMontage() const;
	int32 GetCurrentAmmo();
	void SetCurrentAmmo(int32 AmmoAmount);
	bool IsFiring() const { return bIsFiring; }
	bool CanFire() const;
	void StartFire();
	void StopFire();
	/** Shoots one bullet or projectile in the direction the character is currently looking in. Calls OnMakeOneShot() to execute the actual shot logic. */
	void MakeOneShot();
	/** Calculates a random shot direction based on the bullet spread. */
	FRotator GetShotDirection(FRotator ViewPointRotation) const;
	FTransform GetForeGripSocketTransform() const;
	float GetReloadingWalkSpeed() const { return ReloadingWalkSpeed; }
	void StartReload();
	void StopReload();
	void OnReloadComplete();

	//@ SaveSubsystemInterface
	virtual void OnLevelDeserialized_Implementation() override;
	//~ SaveSubsystemInterface

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Item|Ranged Weapon", meta = (ClampMin = 0, UIMin = 0))
	int32 DefaultWeaponModeIndex = 0;
	/** Description of bullets, magazine, fire rate, effects, etc. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Item|Ranged Weapon")
	TArray<FWeaponModeParameters> WeaponModes;
	UPROPERTY(EditAnywhere, Category = "Equipment Item|Ranged Weapon", meta = (ClampMin = 0.f, UIMin = 0.f))
	float ReloadingWalkSpeed = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Item|Ranged Weapon")
	USkeletalMeshComponent* MeshComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Item|Ranged Weapon")
	FName ForeGripSocketName = "ForeGripSocket";
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Item|Ranged Weapon")
	FVector ForeGripOffsetFromSocket = FVector(9.f, 0.f, 0.f);
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UWeaponMuzzleComponent* MuzzleComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Item|Ranged Weapon")
	FName MuzzleSocketName = "MuzzleSocket";
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Item|Ranged Weapon")
	UAnimMontage* CharacterFireMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Item|Ranged Weapon")
	UAnimMontage* WeaponFireMontage;

private:
	void OnAmmoChanged();
	/** Actual shooting logic.
	 * @param ShotInfoArray List of FShotInfo structs describing the trajectory of each bullet.
	 */
	void OnMakeOneShot(const TArray<FShotInfo>& ShotInfoArray);
	void OnShotEnd();
	UFUNCTION(Server, Reliable)
	void Server_OnMakeOneShot(const TArray<FShotInfo>& ShotInfoArray);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnMakeOneShot(const TArray<FShotInfo>& ShotInfoArray);
	UFUNCTION(Server, Reliable)
	void Server_SetCurrentWeaponMode(int32 ModeIndex);

	FTimerHandle OneShotTimer;
	FTimerHandle OnShotEndTimer;
	bool bIsFiring = false;
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeaponModeIndex, SaveGame)
	int32 CurrentWeaponModeIndex = 0;
	UFUNCTION()
	void OnRep_CurrentWeaponModeIndex();
	FWeaponModeParameters* CurrentWeaponMode;
	/** Amount of ammo currently loaded in the magazine for each weapon mode. */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeaponAmmo, SaveGame)
	TArray<int32> CurrentWeaponAmmo;
	UFUNCTION()
	void OnRep_CurrentWeaponAmmo(TArray<int32> CurrentWeaponAmmo_Old);
};
