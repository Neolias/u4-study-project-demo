// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Actors/Projectiles/ProjectilePool.h"
#include "Components/ActorComponent.h"
#include "Inventory/Items/InventoryItem.h"
#include "Subsystems/SaveSubsystem/SaveSubsystemInterface.h"
#include "CharacterEquipmentComponent.generated.h"

class AXyzBaseCharacter;
class URadialMenuWidget;
class UDataTable;
class UEquipmentViewWidget;
class AXyzProjectile;
class AMeleeWeaponItem;
class AEquipmentItem;
class ARangedWeaponItem;
class AThrowableItem;

typedef TArray<AEquipmentItem*, TInlineAllocator<(uint32)EEquipmentItemSlot::Max>> TEquippedItemArray;
typedef TArray<uint32, TInlineAllocator<(uint32)EWeaponAmmoType::Max>> TEquipmentAmmoArray;

/**
 *
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class XYZHOMEWORK_API UCharacterEquipmentComponent : public UActorComponent, public ISaveSubsystemInterface
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCurrentWeaponAmmoChangedEvent, int32, int32)
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnCurrentThrowableAmmoChangedEvent, int32)
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnEquipmentItemChangedEvent, const AEquipmentItem*)
	FOnCurrentWeaponAmmoChangedEvent OnCurrentWeaponAmmoChangedEvent;
	FOnCurrentThrowableAmmoChangedEvent OnCurrentThrowableAmmoChangedEvent;
	FOnEquipmentItemChangedEvent OnEquipmentItemChangedEvent;

	UCharacterEquipmentComponent();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetInventoryItemDataTable(const TSoftObjectPtr<UDataTable>& NewDataTable) { InventoryItemDataTable = NewDataTable; }
	FInventoryTableRow* LoadItemDataFromDataTable(EInventoryItemType ItemType) const;
	FInventoryTableRow* LoadItemDataFromDataTable(EEquipmentItemType ItemType) const;

	//@ SaveSubsystemInterface
	virtual void OnLevelDeserialized_Implementation() override;
	//~ SaveSubsystemInterface

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Equipment Component|Loadout")
	EEquipmentItemSlot DefaultEquipmentItemSlot = EEquipmentItemSlot::PrimaryWeapon;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Equipment Component|Loadout")
	TMap<EWeaponAmmoType, int32> MaxEquippedWeaponAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Equipment Component|Loadout")
	TMap<EEquipmentItemSlot, TSoftClassPtr<AEquipmentItem>> EquipmentSlots;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Equipment Component|Loadout")
	TArray<EEquipmentItemSlot> WeaponSwitchIgnoredSlots;
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Base Character|Equipment Component|Loadout")
	TArray<FProjectilePool> ProjectilePools;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Character|Equipment Component")
	TSoftClassPtr<UEquipmentViewWidget> EquipmentViewWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Character|Equipment Component")
	TSoftClassPtr<URadialMenuWidget> RadialMenuWidgetClass;

private:
	void InstantiateProjectilePools(AActor* Owner);
	void JumpToAnimMontageSection(FName SectionName) const;

	UPROPERTY()
	AXyzBaseCharacter* BaseCharacterOwner;
	TSoftObjectPtr<UDataTable> InventoryItemDataTable;

#pragma region EQUIPMENT MANAGEMENT

public:
	bool IsEquipAnimMontagePlaying() const { return bIsEquipAnimMontagePlaying; }
	AEquipmentItem* GetEquippedItem(int32 SlotIndex);
	AEquipmentItem* GetCurrentEquipmentItem() const { return CurrentEquipmentItem.Get(); }
	const TArray<AEquipmentItem*>& GetEquippedItemsArray() const { return EquippedItemsArray; }
	EEquipmentItemType GetCurrentEquipmentItemType() const;
	UFUNCTION(BlueprintCallable, Category = "Character Equipment Component")
	EEquipmentItemSlot GetDefaultEquipmentItemSlot() const { return DefaultEquipmentItemSlot; }
	void CreateLoadout();
	bool AddEquipmentItem(EInventoryItemType ItemType, int32 Amount = 1, int32 EquipmentSlotIndex = -1);
	bool AddEquipmentItem(TSubclassOf<AEquipmentItem> EquipmentItemClass, int32 Amount = 1, int32 EquipmentSlotIndex = -1);
	bool RemoveEquipmentItem(int32 EquipmentSlotIndex);
	bool StackEquipmentItems(int32 EquipmentSlotIndex, EInventoryItemType ItemType, int32 Amount);
	UFUNCTION(Server, Reliable)
	void Server_StackEquipmentItems(int32 EquipmentSlotIndex, EInventoryItemType ItemType, int32 Amount);
	UFUNCTION(BlueprintCallable, Category = "Character Equipment Component")
	bool EquipItemBySlot(EEquipmentItemSlot EquipmentItemSlot, bool bShouldSkipAnimation = false, bool bShouldUpdateCurrentSlotIndex = true, bool bIsReplicated = true);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EquipItem(EEquipmentItemSlot EquipmentItemSlot, bool bShouldSkipAnimation = false, bool bShouldUpdateCurrentSlotIndex = true);
	void EquipItemFromCurrentSlot(bool bShouldSkipAnimation = false);
	void UnequipCurrentItem();
	void AttachCurrentEquipmentItemToCharacterMesh();
	void DrawNextItem();
	void DrawPreviousItem();

private:
	EEquipmentItemSlot GetCurrentItemSlot() const;
	void SetCurrentSlotIndex(int32 NewSlotIndex);
	void LoadoutOneItem(EEquipmentItemSlot EquipmentSlot, TSubclassOf<AEquipmentItem> EquipmentItemClass, USkeletalMeshComponent* SkeletalMesh, int32 CountInSlot = -1);
	void InitializeInventoryItem(AEquipmentItem* EquipmentItem, int32 Count = 1) const;
	EEquipmentItemSlot FindCompatibleSlot(AEquipmentItem* EquipmentItem);
	void GetNextSlotIndex(OUT int32& NextSlotIndex);
	void GetPreviousSlotIndex(OUT int32& PreviousSlotIndex);
	void EquipItem(AEquipmentItem* ItemToEquip, bool bShouldSkipAnimation = false);
	void UnequipItem(AEquipmentItem* ItemToUnequip);

	UPROPERTY(Replicated, SaveGame)
	int32 CurrentSlotIndex = -1;
	UPROPERTY(ReplicatedUsing=OnRep_EquippedItemsArray, SaveGame)
	TArray<AEquipmentItem*> EquippedItemsArray;
	UFUNCTION()
	void OnRep_EquippedItemsArray();
	UPROPERTY(ReplicatedUsing=OnRep_EquipmentAmmoArray, SaveGame)
	TArray<uint32> EquipmentAmmoArray;
	UFUNCTION()
	void OnRep_EquipmentAmmoArray();
	TWeakObjectPtr<AEquipmentItem> CurrentEquipmentItem;
	FTimerHandle EquipItemTimer;
	bool bIsEquipAnimMontagePlaying = false;
#pragma endregion

#pragma region RANGED WEAPONS

public:
	ARangedWeaponItem* GetCurrentRangedWeapon() const { return CurrentRangedWeapon.Get(); }
	float GetCurrentWeaponReloadWalkSpeed() const { return CurrentWeaponReloadWalkSpeed; }
	void SetAmmo(EWeaponAmmoType AmmoType, int32 NewAmmo);
	bool AddAmmo(EWeaponAmmoType AmmoType, int32 Amount);
	int32 RemoveAmmo(EWeaponAmmoType AmmoType, int32 Amount);
	bool IsCurrentWeaponMagazineEmpty() const;
	bool IsCurrentWeaponMagazineFull() const;
	int32 GetAvailableAmmoForWeaponMagazine(ARangedWeaponItem* RangedWeaponItem);
	void ActivateNextWeaponMode();
	bool CanReloadCurrentWeapon();
	bool TryReloadCurrentWeapon();
	void TryReloadNextBullet();

private:
	void LoadWeaponMagazineByBullet(ARangedWeaponItem* RangedWeaponItem);
	void OnCurrentWeaponAmmoChanged(int32 NewAmmo);
	void OnCurrentWeaponReloaded(bool bIsReloadComplete);
	void OnWeaponMagazineEmpty();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnWeaponMagazineEmpty();

	TWeakObjectPtr<ARangedWeaponItem> CurrentRangedWeapon;
	float CurrentWeaponReloadWalkSpeed = 0.f;
	FDelegateHandle OnAmmoChangedDelegate;
	FDelegateHandle OnWeaponReloadedDelegate;
	FDelegateHandle OnWeaponMagazineEmptyDelegate;
#pragma endregion

#pragma region OTHER ITEMS

public:
	AMeleeWeaponItem* GetCurrentMeleeWeapon() const { return CurrentMeleeWeapon.Get(); }
	bool IsPrimaryItemEquipped() const { return bIsPrimaryItemEquipped; }
	AThrowableItem* GetCurrentThrowableItem() const { return CurrentThrowableItem.Get(); }
	float GetCurrentThrowableWalkSpeed() const { return CurrentThrowableWalkSpeed; }
	void EquipPrimaryItem(bool bForceEquip = false);
	void UnequipPrimaryItem(bool bForceUnequip = false);
	bool CanItemBeThrown(AThrowableItem* ThrowableItem);
	void ThrowItem();

private:
	void OnItemThrown();
	void OnCurrentThrowableAmmoChanged(int32 NewAmmo0) const;

	TWeakObjectPtr<AThrowableItem> CurrentThrowableItem;
	TWeakObjectPtr<AMeleeWeaponItem> CurrentMeleeWeapon;
	float CurrentThrowableWalkSpeed = 0.f;
	FDelegateHandle OnItemThrownDelegate;
	FTimerHandle ThrowItemTimer;
	bool bIsPrimaryItemEquipped = false;
#pragma endregion

#pragma region WIDGETS

public:
	bool IsViewEquipmentVisible() const;
	void OpenViewEquipment(APlayerController* PlayerController);
	void CloseViewEquipment() const;
	bool IsRadialMenuVisible() const;
	void OpenRadialMenu(APlayerController* PlayerController);
	void CloseRadialMenu() const;
	void OnEquipmentSlotUpdated(int32 SlotIndex);

private:
	void UpdateAmmoHUDWidgets();
	void CreateEquipmentViewWidget(APlayerController* PlayerController);
	void CreateRadialMenuWidget(APlayerController* PlayerController);

	UPROPERTY()
	UEquipmentViewWidget* EquipmentViewWidget;
	UPROPERTY()
	URadialMenuWidget* RadialMenuWidget;
#pragma endregion
};
