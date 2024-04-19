// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Components/ActorComponent.h"
#include "CharacterEquipmentComponent.generated.h"

class AMeleeWeaponItem;
class AEquipmentItem;
class ARangedWeaponItem;
class AThrowableItem;

typedef TArray<AEquipmentItem*, TInlineAllocator<(uint32)EEquipmentItemSlot::Max>> TEquippedItemArray;
typedef TArray<uint32, TInlineAllocator<(uint32)EWeaponAmmoType::Max>> TEquipmentAmmoArray;
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCurrentWeaponAmmoChangedEvent, int32, int32)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCurrentThrowableAmmoChangedEvent, int32)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEquipmentItemChangedEvent, const AEquipmentItem*)

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class XYZHOMEWORK_API UCharacterEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	FOnCurrentWeaponAmmoChangedEvent OnCurrentWeaponAmmoChangedEvent;
	FOnCurrentThrowableAmmoChangedEvent OnCurrentThrowableAmmoChangedEvent;
	FOnEquipmentItemChangedEvent OnEquipmentItemChangedEvent;

	UCharacterEquipmentComponent();
	AEquipmentItem* GetCurrentEquipmentItem() const { return CurrentEquippedItem.Get(); }
	ARangedWeaponItem* GetCurrentRangedWeapon() const { return CurrentRangedWeapon.Get(); }
	EEquipmentItemType GetCurrentRangedWeaponType() const;
	bool IsThrowingItem() const;
	bool IsReloadingWeapon() const;
	bool IsFiringWeapon() const;
	bool IsPrimaryItemEquipped() const { return bIsPrimaryItemEquipped; }
	AThrowableItem* GetCurrentThrowableItem() const { return CurrentThrowableItem.Get(); }
	AMeleeWeaponItem* GetCurrentMeleeWeapon() const { return CurrentMeleeWeapon.Get(); }
	int32 GetAvailableAmmoForWeaponMagazine(const ARangedWeaponItem* RangedWeaponItem);
	UFUNCTION(BlueprintCallable, Category = "Character Equipment Component")
	EEquipmentItemSlot GetDefaultEquipmentItemSlot() const { return DefaultEquipmentItemSlot; }
	bool IsMeleeAttackActive() const { return bIsMeleeAttackActive; }
	UFUNCTION()
	void SetIsMeleeAttackActive(const bool bIsMeleeAttackActive_In) { bIsMeleeAttackActive = bIsMeleeAttackActive_In; }
	void EquipFromDefaultItemSlot();
	void DrawNextItem();
	void DrawPreviousItem();
	UFUNCTION(BlueprintCallable, Category = "Character Equipment Component")
	bool EquipItemBySlotType(EEquipmentItemSlot EquipmentItemSlot);
	void UnequipCurrentItem();
	void EquipPreviousItemIfUnequipped();
	void AttachCurrentEquippedItemToCharacterMesh();
	void ActivateNextWeaponMode();
	bool CanReloadCurrentWeapon();
	void TryReloadNextBullet();
	bool IsCurrentWeaponMagazineFull() const;
	void EquipPrimaryItem();
	UFUNCTION()
	void UnequipPrimaryItem();
	bool CanThrowItem(const AThrowableItem* ThrowableItem);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	EEquipmentItemSlot DefaultEquipmentItemSlot = EEquipmentItemSlot::PrimaryWeapon;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	TMap<EWeaponAmmoType, int32> MaxEquippedWeaponAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	TMap<EEquipmentItemSlot, TSubclassOf<AEquipmentItem>> EquipmentSlots;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	TArray<EEquipmentItemSlot> WeaponSwitchIgnoredSlots;

	UPROPERTY()
	class AXyzBaseCharacter* BaseCharacter;
	TWeakObjectPtr<AEquipmentItem> CurrentEquippedItem;
	TWeakObjectPtr<ARangedWeaponItem> CurrentRangedWeapon;
	TWeakObjectPtr<AThrowableItem> CurrentThrowableItem;
	TWeakObjectPtr<AMeleeWeaponItem> CurrentMeleeWeapon;
	int32 CurrentSlotIndex = 0;
	TEquippedItemArray EquippedItemsArray;
	TEquipmentAmmoArray EquipmentAmmoArray;
	EEquipmentItemSlot CurrentEquippedSlot = EEquipmentItemSlot::None;
	FDelegateHandle OnAmmoChangedDelegate;
	FDelegateHandle OnWeaponReloadedDelegate;
	FDelegateHandle OnWeaponMagazineEmptyDelegate;
	FDelegateHandle OnItemThrownDelegate;
	FDelegateHandle OnItemThrownAnimationFinishedDelegate;
	FDelegateHandle OnMeleeAttackActivated;

	FTimerHandle EquipItemTimer;
	FTimerHandle ThrowItemTimer;
	bool bIsPrimaryItemEquipped = false;
	bool bIsMeleeAttackActive = false;

	virtual void BeginPlay() override;
	UFUNCTION()
	void OnCurrentWeaponAmmoChanged(int32 AmmoAmount = 0.f);
	UFUNCTION()
	void OnCurrentWeaponReloaded();
	UFUNCTION()
	void OnWeaponMagazineEmpty();
	UFUNCTION()
	void OnThrowItemEnd();
	UFUNCTION()
	void OnThrowItemAnimationFinished();
	void OnCurrentThrowableAmmoChanged(int32 NewAmmo) const;
	void CreateLoadout();
	bool IncrementCurrentSlotIndex();
	bool DecrementCurrentSlotIndex();
	AEquipmentItem* GetNextItem();
	AEquipmentItem* GetPreviousItem();
	void UnequipItem(AEquipmentItem* EquipmentItem);
	void EquipItem(AEquipmentItem* EquipmentItem, bool bShouldSkipAnimation = false);
	void JumpToAnimMontageSection(FName ReloadLoopStartSectionName) const;
	void LoadWeaponMagazineByBullet(ARangedWeaponItem* RangedWeaponItem);
};
