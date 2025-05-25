// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Components/ActorComponent.h"
#include "CharacterInventoryComponent.generated.h"

struct FInventoryTableRow;
class UDataTable;
class UInventoryItem;
class UInventoryViewWidget;

/** Struct describing an inventory slot in which inventory items can be stored. */
UCLASS(BlueprintType)
class XYZHOMEWORK_API UInventorySlot : public UObject
{
	GENERATED_BODY()

public:
	DECLARE_DELEGATE(FOnSlotUpdatedEvent)
	FOnSlotUpdatedEvent OnSlotUpdatedEvent;

	virtual bool IsSupportedForNetworking() const override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	UInventoryItem* GetLinkedInventoryItem() const { return Item; }
	void SetLinkedInventoryItem(UInventoryItem* InventoryItem);
	void ClearSlot();

private:
	void OnSlotUpdated() const;
	void OnLinkedInventoryItemChanged();

	UPROPERTY(ReplicatedUsing=OnRep_Item)
	UInventoryItem* Item;
	UFUNCTION()
	void OnRep_Item();
};

/** Custom inventory component that manages items stored in inventory slots. */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class XYZHOMEWORK_API UCharacterInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterInventoryComponent();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	void SetInventoryItemDataTable(const TSoftObjectPtr<UDataTable>& NewDataTable) { InventoryItemDataTable = NewDataTable; }
	FInventoryTableRow* LoadItemDataFromDataTable(EInventoryItemType ItemType) const;
	FInventoryTableRow* LoadItemDataFromDataTable(EWeaponAmmoType AmmoType) const;
	/** Updates 'UsedSlotCount' with 'Amount'. */
	void AddUsedSlotCount(int32 Amount);
	void CreateViewWidget(APlayerController* PlayerController);
	void OpenViewInventory(APlayerController* PlayerController);
	void CloseViewInventory();
	bool IsViewInventoryVisible() const;
	/**
	 * Tries to add an item of 'ItemType' type to the inventory. First, tries to stack items. Then, tries to fill empty slots.
	 * @return Returns false if fails.
	 */
	bool AddInventoryItem(EInventoryItemType ItemType, int32 Amount);
	/**
	 * Tries to find slots reserved for items of 'ItemType' type. Removes an 'Amount' number of items from those slots.
	 * @return Returns the number of items removed.
	 */
	int32 RemoveInventoryItemByType(EInventoryItemType ItemType, int32 Amount);
	/**
	 * Removes an 'Amount' number of items from the slot. Clears the slot if 'Amount' >= slot capacity.
	 * @return Returns the number of items removed.
	 */
	int32 RemoveInventoryItemByIndex(int32 SlotIndex, int32 Amount);
	/**
	 * Removes an 'Amount' number of items from the slot. Clears the slot if 'Amount' >= slot capacity.
	 * @return Returns the number of items removed.
	 */
	int32 RemoveInventoryItemBySlot(UInventorySlot* Slot, int32 Amount);
	/**
	 * Tries to add an item of 'AmmoType' type to the inventory. First, tries to stack items. Then, tries to fill empty slots.
	 * @return Returns false if fails.
	 */
	bool AddAmmoItem(EWeaponAmmoType AmmoType, int32 Amount);
	/**
	 * Tries to find slots reserved for items of 'AmmoType' type. Removes an 'Amount' number of items from those slots.
	 * @return Returns the number of items removed.
	 */
	int32 RemoveAmmoItem(EWeaponAmmoType AmmoType, int32 Amount);
	UFUNCTION(Server, Reliable)
	void Server_SetLinkedInventoryItem(UInventorySlot* InventorySlot, UInventoryItem* InventoryItem);
	UFUNCTION(Server, Reliable)
	void Server_StackItems(UInventorySlot* InventorySlot, EInventoryItemType ItemType, int32 AmountToAdd);

protected:
	/** Number of slots in the inventory. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Character|Inventory Component", meta = (ClampMin = 0, UIMin = 0))
	int32 Capacity = 24;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Character|Inventory Component")
	TSoftClassPtr<UInventoryViewWidget> InventoryViewWidgetClass;

private:
	/**
	 * Tries to find inventory slots reserved for items of the same 'ItemType' type and fill them with an 'Amount' number of items.
	 * @return Returns the "Remainder" number of items that did not fit.
	 */
	int32 StackItems(EInventoryItemType ItemType, int32 Amount);
	/**
	 * Tries to find empty inventory slots and fill them with an 'Amount' number of items.
	 * @return Returns false if fails.
	 */
	bool FillEmptySlots(EInventoryItemType ItemType, int32 Amount);

	UPROPERTY(Replicated)
	int32 UsedSlotCount = 0;
	UPROPERTY(Replicated)
	TArray<UInventorySlot*> ItemSlots;
	UPROPERTY()
	UInventoryViewWidget* InventoryViewWidget;
	TSoftObjectPtr<UDataTable> InventoryItemDataTable;
};
