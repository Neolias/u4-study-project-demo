// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "Components/ActorComponent.h"
#include "CharacterInventoryComponent.generated.h"

struct FInventoryTableRow;
class UDataTable;
class UInventoryItem;
class UInventoryViewWidget;

/**
 *
 */
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
	void AddUsedSlotCount(int32 Amount);
	void CreateViewWidget(APlayerController* PlayerController);
	void OpenViewInventory(APlayerController* PlayerController);
	void CloseViewInventory();
	bool IsViewInventoryVisible() const;
	bool AddInventoryItem(EInventoryItemType ItemType, int32 Amount);
	int32 RemoveInventoryItem(EInventoryItemType ItemType, int32 Amount);
	int32 RemoveInventoryItem(int32 SlotIndex, int32 Amount);
	int32 RemoveInventoryItem(UInventorySlot* Slot, int32 Amount);
	bool AddAmmoItem(EWeaponAmmoType AmmoType, int32 Amount);
	int32 RemoveAmmoItem(EWeaponAmmoType AmmoType, int32 Amount);
	UFUNCTION(Server, Reliable)
	void Server_SetLinkedInventoryItem(UInventorySlot* InventorySlot, UInventoryItem* InventoryItem);
	UFUNCTION(Server, Reliable)
	void Server_StackItems(UInventorySlot* InventorySlot, EInventoryItemType ItemType, int32 AmountToAdd);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Character|Inventory Component", meta = (ClampMin = 0, UIMin = 0))
	int32 Capacity = 24;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Character|Inventory Component")
	TSoftClassPtr<UInventoryViewWidget> InventoryViewWidgetClass;

private:
	int32 StackItems(EInventoryItemType ItemType, int32 Amount);
	bool FillEmptySlots(EInventoryItemType ItemType, int32 Amount);

	UPROPERTY(Replicated)
	int32 UsedSlotCount = 0;
	UPROPERTY(Replicated)
	TArray<UInventorySlot*> ItemSlots;
	UPROPERTY()
	UInventoryViewWidget* InventoryViewWidget;
	TSoftObjectPtr<UDataTable> InventoryItemDataTable;
};
