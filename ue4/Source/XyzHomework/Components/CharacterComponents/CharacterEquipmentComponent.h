// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Actors/Projectiles/ProjectilePool.h"
#include "Components/ActorComponent.h"
#include "Inventory/Items/InventoryItem.h"
#include "Subsystems/SaveSubsystem/SaveSubsystemInterface.h"
#include "CharacterEquipmentComponent.generated.h"

class AEquipmentItem;
class AMeleeWeaponItem;
class ARangedWeaponItem;
class AThrowableItem;
class AXyzBaseCharacter;
class AXyzProjectile;
class UDataTable;
class UEquipmentViewWidget;
class URadialMenuWidget;

typedef TArray<AEquipmentItem*, TInlineAllocator<(uint32)EEquipmentItemSlot::Max>> TEquippedItemArray;
typedef TArray<uint32, TInlineAllocator<(uint32)EWeaponAmmoType::Max>> TEquipmentAmmoArray;

/** Custom equipment component that manages items placed in equipment slots, e.g. weapons and grenades. Additionally, provides the interface for updating ammo stored in the inventory. */
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
	/** Slots that are ignored while scrolling through equipped weapons using DrawNext() and DrawPrevious(). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Equipment Component|Loadout")
	TArray<EEquipmentItemSlot> WeaponSwitchIgnoredSlots;
	/** Description of projectiles, such as grenades and sniper rifle bullets, spawned by the server on BeginPlay. */
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Base Character|Equipment Component|Loadout")
	TArray<FProjectilePool> ProjectilePools;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Character|Equipment Component")
	TSoftClassPtr<UEquipmentViewWidget> EquipmentViewWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Character|Equipment Component")
	TSoftClassPtr<URadialMenuWidget> RadialMenuWidgetClass;

private:
	/** Spawns projectiles defined in ProjectilePools. */
	void InstantiateProjectilePools(AActor* Owner);
	void JumpToAnimMontageSection(FName SectionName) const;

	UPROPERTY()
	AXyzBaseCharacter* BaseCharacterOwner;
	TSoftObjectPtr<UDataTable> InventoryItemDataTable;

#pragma region EQUIPMENT MANAGEMENT

public:
	bool IsEquipAnimMontagePlaying() const { return bIsEquipAnimMontagePlaying; }
	AEquipmentItem* GetEquipmentItemInSlot(int32 SlotIndex);
	AEquipmentItem* GetCurrentEquipmentItem() const { return CurrentEquipmentItem.Get(); }
	const TArray<AEquipmentItem*>& GetEquippedItemsArray() const { return EquippedItemsArray; }
	EEquipmentItemType GetCurrentEquipmentItemType() const;
	UFUNCTION(BlueprintCallable, Category = "Character Equipment Component")
	EEquipmentItemSlot GetDefaultEquipmentItemSlot() const { return DefaultEquipmentItemSlot; }

	/** Spawns equipment items defined in EquippedItemsArray and loads ammo if needed. Attaches the items to 'UnequippedSockets' on the character mesh. */
	void CreateLoadout();
	/**
	 * Loads the subclass of an item and calls AddEquipmentItemByClass().
	 * @param EquipmentSlotIndex Slot in which the item will be placed. -1 indicates the request to find a compatible slot.
	 * @return false indicates a failed operation due to an internal error.
	 */
	bool AddEquipmentItemByType(EInventoryItemType ItemType, int32 Amount = 1, int32 EquipmentSlotIndex = -1);
	/**
	 * Creates a loadout of an item of this subclass. If an identical item is already equipped in the slot, tries to stack the items.
	 * If failed or the items are of different classes, tries to add the old item to the inventory. If failed again, drops the old item.
	 * @param EquipmentSlotIndex Slot in which the item will be placed. -1 indicates the request to find a compatible slot.
	 * @return Returns false if the operation fails due to an internal error.
	 */
	bool AddEquipmentItemByClass(TSubclassOf<AEquipmentItem> EquipmentItemClass, int32 Amount = 1, int32 EquipmentSlotIndex = -1);
	/** Destroys an item equipped in this slot. Moves the item's ammo to the inventory if needed. */
	bool RemoveEquipmentItem(int32 EquipmentSlotIndex);
	/**
	 * Tries to add an 'Amount' number of items of 'ItemType' type in this slot.
	 * @return Returns false if failed.
	 * */
	bool StackEquipmentItems(int32 EquipmentSlotIndex, EInventoryItemType ItemType, int32 Amount);
	UFUNCTION(Server, Reliable)
	void Server_StackEquipmentItems(int32 EquipmentSlotIndex, EInventoryItemType ItemType, int32 Amount);
	/**
	 * Makes the character appear equipping an item from this equipment slot. Updates the CurrentSlotIndex and CurrentEquipmentItem variables. Can be replicated to clients if needed.
	 * @return Returns false if failed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Character Equipment Component")
	bool EquipItemBySlot(EEquipmentItemSlot EquipmentItemSlot, bool bShouldSkipAnimation = false, bool bShouldUpdateCurrentSlotIndex = true, bool bIsReplicated = true);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EquipItem(EEquipmentItemSlot EquipmentItemSlot, bool bShouldSkipAnimation = false, bool bShouldUpdateCurrentSlotIndex = true);
	/** If unequipped, makes the character appear equipping an item from CurrentSlotIndex. */
	void EquipItemFromCurrentSlot(bool bShouldSkipAnimation = false);
	/** If equipped, makes the character appear unequipping the item into CurrentSlotIndex. */
	void UnequipCurrentItem();
	void AttachCurrentEquipmentItemToCharacterMesh();
	/** Increments CurrentSlotIndex and makes the character appear equipping an item from the new slot. */
	void DrawNextItem();
	/** Decrements CurrentSlotIndex and makes the character appear equipping an item from the new slot. */
	void DrawPreviousItem();

private:
	EEquipmentItemSlot GetCurrentItemSlot() const;
	void SetCurrentSlotIndex(int32 NewSlotIndex);
	/**
	 * Spawns an equipment item of EquipmentItemClass subclass and loads ammo if needed. Attaches the item to 'UnequippedSockets' on the character mesh.
	 * @param CountInSlot Applies only to equipment items that serve as ammo units, e.g. grenades. -1 indicates a loadout on BeginPlay when extra ammo is placed in the inventory.
	 */
	void LoadoutOneItem(EEquipmentItemSlot EquipmentSlot, TSubclassOf<AEquipmentItem> EquipmentItemClass, USkeletalMeshComponent* SkeletalMesh, int32 CountInSlot = -1);
	/** Creates a new inventory item for this equipment item. */
	void InitializeInventoryItem(AEquipmentItem* EquipmentItem, int32 Count = 1) const;
	/** Finds a free slot or a slot with an item of a different type. */
	EEquipmentItemSlot FindCompatibleSlot(AEquipmentItem* EquipmentItem);
	void GetNextSlotIndex(OUT int32& NextSlotIndex);
	void GetPreviousSlotIndex(OUT int32& PreviousSlotIndex);
	/** Plays the equipping animation, binds delegates, and attaches this item to the 'EquippedSocket' of the character mesh. */
	void EquipItem(AEquipmentItem* ItemToEquip, bool bShouldSkipAnimation = false);
	/** Stops the equipping animation, unbinds delegates, and attaches this item to the 'UnequippedSocket' of the character mesh. */
	void UnequipItem(AEquipmentItem* ItemToUnequip);

	/** Equipment slot of the item that the character is currently using. */
	UPROPERTY(Replicated, SaveGame)
	int32 CurrentSlotIndex = -1;
	/** List of equipment items that are placed into available equipment slots. */
	UPROPERTY(ReplicatedUsing=OnRep_EquippedItemsArray, SaveGame)
	TArray<AEquipmentItem*> EquippedItemsArray;
	UFUNCTION()
	void OnRep_EquippedItemsArray();
	/** Ammo that is stored in the inventory and can be used to reload equipment items. */
	UPROPERTY(ReplicatedUsing=OnRep_EquipmentAmmoArray, SaveGame)
	TArray<uint32> EquipmentAmmoArray;
	UFUNCTION()
	void OnRep_EquipmentAmmoArray();
	/** Equipment item that the character is currently using. */
	TWeakObjectPtr<AEquipmentItem> CurrentEquipmentItem;
	FTimerHandle EquipItemTimer;
	bool bIsEquipAnimMontagePlaying = false;
#pragma endregion

#pragma region RANGED WEAPONS

public:
	ARangedWeaponItem* GetCurrentRangedWeapon() const { return CurrentRangedWeapon.Get(); }
	float GetCurrentWeaponReloadWalkSpeed() const { return CurrentWeaponReloadWalkSpeed; }
	/** Updates EquipmentAmmoArray with a new value. */
	void SetAmmo(EWeaponAmmoType AmmoType, int32 NewAmmo);
	/**
	 * Updates EquipmentAmmoArray with a new value by adding Amount.
	 * @return Returns false if failed.
	 */
	bool AddAmmo(EWeaponAmmoType AmmoType, int32 Amount);
	/**
	 * Updates EquipmentAmmoArray with a new value by removing Amount.
	 * @return Returns the actual amount removed. 
	 */
	int32 RemoveAmmo(EWeaponAmmoType AmmoType, int32 Amount);
	bool IsCurrentWeaponMagazineEmpty() const;
	bool IsCurrentWeaponMagazineFull() const;
	/** Calculates the amount of ammo that can be loaded into the magazine. */
	int32 GetAvailableAmmoForWeaponMagazine(ARangedWeaponItem* RangedWeaponItem);
	void ActivateNextWeaponMode();
	bool CanReloadCurrentWeapon();
	bool TryReloadCurrentWeapon();
	/** Reloads one bullet at a time. Used by anim notifies during anim montage sections. */
	void TryReloadNextBullet();

private:
	/** Loads a weapon magazine to full capacity one bullet at a time. */
	void LoadWeaponMagazineByBullet(ARangedWeaponItem* RangedWeaponItem);
	void OnCurrentWeaponAmmoChanged(int32 NewAmmo);
	void OnCurrentWeaponReloaded(bool bIsReloadComplete);
	void OnWeaponMagazineEmpty();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnWeaponMagazineEmpty();

	/** Ranged weapon item that the character is currently using. */
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
	/**
	 * Equips an item stored in EEquipmentItemSlot::PrimaryItem.
	 * @param bForceEquip Forces equipping the item even if another primary items is being already used.
	 */
	void EquipPrimaryItem(bool bForceEquip = false);
	/** Unequips an item stored in EEquipmentItemSlot::PrimaryItem. */
	void UnequipPrimaryItem();
	void UpdatePrimaryItemSlot();
	bool CanItemBeThrown(AThrowableItem* ThrowableItem);
	void ThrowItem();

private:
	void OnItemThrown();
	void OnCurrentThrowableAmmoChanged(int32 NewAmmo0) const;

	/** Melee weapon item that the character is currently using. */
	TWeakObjectPtr<AMeleeWeaponItem> CurrentMeleeWeapon;
	/** Throwable item that the character is currently using. */
	TWeakObjectPtr<AThrowableItem> CurrentThrowableItem;
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
