// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

#include "Actors/Equipment/Throwables/ThrowableItem.h"
#include "Actors/Equipment/Weapons/MeleeWeaponItem.h"
#include "Actors/Equipment/Weapons/RangedWeaponItem.h"
#include "Actors/Projectiles/ProjectilePool.h"
#include "Blueprint/UserWidget.h"
#include "Characters/XyzBaseCharacter.h"
#include "Inventory/Items/InventoryItem.h"
#include "Net/UnrealNetwork.h"
#include "UI/Widgets/Equipment/EquipmentViewWidget.h"
#include "UI/Widgets/Equipment/RadialMenuWidget.h"

UCharacterEquipmentComponent::UCharacterEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	EquipmentAmmoArray.AddZeroed((uint32)EWeaponAmmoType::Max);
	EquippedItemsArray.AddZeroed((uint32)EEquipmentItemSlot::Max);
}

void UCharacterEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(GetOwner()->IsA<AXyzBaseCharacter>(), TEXT("UCharacterEquipmentComponent::BeginPlay(): UCharacterEquipmentComponent can only be used with AXyzBaseCharacter."))
	BaseCharacterOwner = StaticCast<AXyzBaseCharacter*>(GetOwner());

	if (IsValid(BaseCharacterOwner) && BaseCharacterOwner->GetLocalRole() == ROLE_Authority)
	{
		InstantiateProjectilePools(BaseCharacterOwner);
	}

	FTimerHandle HUDUpdateTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(HUDUpdateTimerHandle, [this]
	{
		// Ensuring that the character's animations and equipped weapon are valid on clients
		if (IsValid(BaseCharacterOwner))
		{
			UnequipCurrentItem();
			EquipItemFromCurrentSlot(true);
		}
		// Ensuring that the player's ammo HUD is valid on BeginPlay on clients
		UpdateAmmoHUDWidgets();
	}, .2f, false);
}

void UCharacterEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCharacterEquipmentComponent, CurrentSlotIndex);
	DOREPLIFETIME(UCharacterEquipmentComponent, EquippedItemsArray);
	DOREPLIFETIME(UCharacterEquipmentComponent, EquipmentAmmoArray);
	DOREPLIFETIME(UCharacterEquipmentComponent, ProjectilePools);
}

FInventoryTableRow* UCharacterEquipmentComponent::LoadItemDataFromDataTable(EInventoryItemType ItemType) const
{
	if (const UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *InventoryItemDataTable.GetUniqueID().GetAssetPathString()))
	{
		FString RowID = UEnum::GetDisplayValueAsText<EInventoryItemType>(ItemType).ToString();
		return DataTable->FindRow<FInventoryTableRow>(FName(RowID), TEXT("Find item data"));
	}

	return nullptr;
}

FInventoryTableRow* UCharacterEquipmentComponent::LoadItemDataFromDataTable(EEquipmentItemType ItemType) const
{
	if (const UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *InventoryItemDataTable.GetUniqueID().GetAssetPathString()))
	{
		FString RowID = UEnum::GetDisplayValueAsText<EEquipmentItemType>(ItemType).ToString();
		return DataTable->FindRow<FInventoryTableRow>(FName(RowID), TEXT("Find item data"));
	}

	return nullptr;
}

//@ SaveSubsystemInterface
void UCharacterEquipmentComponent::OnLevelDeserialized_Implementation()
{
	if (GetCurrentItemSlot() != DefaultEquipmentItemSlot)
	{
		UnequipCurrentItem();
		BaseCharacterOwner->EquipItemFromCurrentSlot(true);
	}
}

//~ SaveSubsystemInterface

void UCharacterEquipmentComponent::InstantiateProjectilePools(AActor* Owner)
{
	for (FProjectilePool& Pool : ProjectilePools)
	{
		Pool.InstantiatePool(GetWorld(), Owner);
	}
}

void UCharacterEquipmentComponent::JumpToAnimMontageSection(FName SectionName) const
{
	if (!IsValid(BaseCharacterOwner))
	{
		return;
	}

	ARangedWeaponItem* EquippedRangedWeapon = CurrentRangedWeapon.Get();
	if (!IsValid(EquippedRangedWeapon))
	{
		return;
	}

	if (UAnimInstance* CharacterAnimInstance = BaseCharacterOwner->GetMesh()->GetAnimInstance())
	{
		const UAnimMontage* CharacterReloadAnimMontage = EquippedRangedWeapon->GetCharacterReloadAnimMontage();
		CharacterAnimInstance->Montage_JumpToSection(SectionName, CharacterReloadAnimMontage);
	}

	if (UAnimInstance* WeaponAnimInstance = EquippedRangedWeapon->GetMesh()->GetAnimInstance())
	{
		const UAnimMontage* WeaponReloadAnimMontage = EquippedRangedWeapon->GetWeaponReloadAnimMontage();
		WeaponAnimInstance->Montage_JumpToSection(SectionName, WeaponReloadAnimMontage);
	}
}

#pragma region INVENTORY MANAGEMENT

AEquipmentItem* UCharacterEquipmentComponent::GetEquipmentItemInSlot(int32 SlotIndex)
{
	if (EquippedItemsArray.Num() > 0 && SlotIndex >= 0 && SlotIndex < EquippedItemsArray.Num())
	{
		return EquippedItemsArray[SlotIndex];
	}

	return nullptr;
}

EEquipmentItemType UCharacterEquipmentComponent::GetCurrentEquipmentItemType() const
{
	const AEquipmentItem* EquippedItem = CurrentEquipmentItem.Get();
	return IsValid(EquippedItem) ? EquippedItem->GetEquipmentItemType() : EEquipmentItemType::None;
}

void UCharacterEquipmentComponent::CreateLoadout()
{
	for (const TPair<EWeaponAmmoType, int32>& AmmoPair : MaxEquippedWeaponAmmo)
	{
		AddAmmo(AmmoPair.Key, AmmoPair.Value);
	}

	for (const TPair<EEquipmentItemSlot, TSoftClassPtr<AEquipmentItem>>& SlotPair : EquipmentSlots)
	{
		if (!SlotPair.Value.LoadSynchronous())
		{
			continue;
		}

		LoadoutOneItem(SlotPair.Key, SlotPair.Value.LoadSynchronous(), BaseCharacterOwner->GetMesh());
	}

	if (CurrentSlotIndex == -1)
	{
		EquipItemBySlot(DefaultEquipmentItemSlot, true);
	}
}

bool UCharacterEquipmentComponent::AddEquipmentItemByType(EInventoryItemType ItemType, int32 Amount/* = 1*/, int32 EquipmentSlotIndex/* = -1*/)
{
	const FInventoryTableRow* ItemData = LoadItemDataFromDataTable(ItemType);
	if (ItemData && ItemData->InventoryItemDescription.EquipmentItemClass.LoadSynchronous())
	{
		return AddEquipmentItemByClass(ItemData->InventoryItemDescription.EquipmentItemClass.LoadSynchronous(), Amount, EquipmentSlotIndex);
	}

	return false;
}

bool UCharacterEquipmentComponent::AddEquipmentItemByClass(TSubclassOf<AEquipmentItem> EquipmentItemClass, int32 Amount/* = 1*/, int32 EquipmentSlotIndex/* = -1*/)
{
	if (Amount < 1 || EquipmentSlotIndex == 0 || EquipmentSlotIndex < -1 || !EquipmentItemClass)
	{
		return false;
	}

	AEquipmentItem* DefaultItem = Cast<AEquipmentItem>(EquipmentItemClass->GetDefaultObject());
	if (!DefaultItem)
	{
		return false;
	}

	EEquipmentItemSlot EquipmentSlot = (EEquipmentItemSlot)EquipmentSlotIndex;
	if (EquipmentSlotIndex == -1) // indicates the request to find a compatible slot
	{
		EquipmentSlot = FindCompatibleSlot(DefaultItem);
	}

	if (!DefaultItem->IsEquipmentSlotCompatible(EquipmentSlot))
	{
		return false;
	}

	// If an item is already equipped in the slot
	AEquipmentItem* EquippedItem = EquippedItemsArray[(int32)EquipmentSlot];
	if (IsValid(EquippedItem))
	{
		UInventoryItem* InventoryItem = EquippedItem->GetLinkedInventoryItem();
		if (!IsValid(InventoryItem))
		{
			return false;
		}

		if (EquippedItem->GetEquipmentItemType() == DefaultItem->GetEquipmentItemType())
		{
			return StackEquipmentItems((int32)EquipmentSlot, InventoryItem->GetInventoryItemType(), Amount);
		}

		if (!RemoveEquipmentItem((int32)EquipmentSlot))
		{
			return false;
		}

		if (!BaseCharacterOwner->PickupItem(InventoryItem->GetInventoryItemType(), InventoryItem->GetCount()))
		{
			BaseCharacterOwner->DropItem(InventoryItem->GetInventoryItemType(), InventoryItem->GetCount());
		}
	}

	LoadoutOneItem(EquipmentSlot, EquipmentItemClass, BaseCharacterOwner->GetMesh(), Amount);
	if (EquipmentViewWidget)
	{
		EquipmentViewWidget->UpdateSlot((int32)EquipmentSlot);
	}
	CloseRadialMenu();
	BaseCharacterOwner->EquipItemFromCurrentSlot();

	return true;
}

bool UCharacterEquipmentComponent::RemoveEquipmentItem(int32 EquipmentSlotIndex)
{
	if (EquipmentSlotIndex > EquippedItemsArray.Num())
	{
		return false;
	}

	AEquipmentItem* EquipmentItem = EquippedItemsArray[EquipmentSlotIndex];
	if (IsValid(EquipmentItem))
	{
		if (CurrentSlotIndex == EquipmentSlotIndex)
		{
			UnequipCurrentItem();
		}

		const UInventoryItem* LinkedItem = EquipmentItem->GetLinkedInventoryItem();
		if (IsValid(LinkedItem) && LinkedItem->CanStackItems())
		{
			// Only applies to equipment items that serve as ammo units, e.g. grenades
			SetAmmo(EquipmentItem->GetAmmoType(), 0);
		}

		ARangedWeaponItem* RangedWeapon = Cast<ARangedWeaponItem>(EquipmentItem);
		if (IsValid(RangedWeapon))
		{
			if (!AddAmmo(RangedWeapon->GetAmmoType(), RangedWeapon->GetCurrentAmmo()))
			{
				BaseCharacterOwner->EquipItemFromCurrentSlot();
				return false;
			}
		}

		EquippedItemsArray[EquipmentSlotIndex] = nullptr;
		EquipmentItem->Destroy();
		if (EquipmentViewWidget)
		{
			EquipmentViewWidget->UpdateSlot(EquipmentSlotIndex);
		}
		CloseRadialMenu();

		return true;
	}

	return false;
}

bool UCharacterEquipmentComponent::StackEquipmentItems(int32 EquipmentSlotIndex, EInventoryItemType ItemType, int32 Amount)
{
	AEquipmentItem* EquippedItem = EquippedItemsArray[EquipmentSlotIndex];
	if (!IsValid(EquippedItem))
	{
		return false;
	}

	UInventoryItem* InventoryItem = EquippedItem->GetLinkedInventoryItem();
	if (!IsValid(InventoryItem) || InventoryItem->GetInventoryItemType() != ItemType)
	{
		return false;
	}

	if (InventoryItem->CanStackItems() && InventoryItem->GetAvailableSpaceInStack() > 0)
	{
		int32 PreviousCount = InventoryItem->GetCount();
		int32 Remainder = Amount - InventoryItem->AddCount(Amount);
		if (Remainder && !BaseCharacterOwner->PickupItem(ItemType, Remainder))
		{
			InventoryItem->SetCount(PreviousCount);
			return false;
		}
		SetAmmo(EquippedItem->GetAmmoType(), InventoryItem->GetCount());

		return true;
	}

	return false;
}

void UCharacterEquipmentComponent::Server_StackEquipmentItems_Implementation(int32 EquipmentSlotIndex, EInventoryItemType ItemType, int32 Amount)
{
	if (!StackEquipmentItems(EquipmentSlotIndex, ItemType, Amount))
	{
		if (!BaseCharacterOwner->PickupItem(ItemType, Amount))
		{
			BaseCharacterOwner->DropItem(ItemType, Amount);
		}
	}
}

bool UCharacterEquipmentComponent::EquipItemBySlot(EEquipmentItemSlot EquipmentItemSlot, bool bShouldSkipAnimation/* = false*/, bool bShouldUpdateCurrentSlotIndex/* = true*/, bool bIsReplicated/* = true*/)
{
	if (!IsValid(BaseCharacterOwner))
	{
		return false;
	}

	if (bIsReplicated && BaseCharacterOwner->GetLocalRole() == ROLE_Authority)
	{
		Multicast_EquipItem(EquipmentItemSlot, bShouldSkipAnimation, bShouldUpdateCurrentSlotIndex);
	}

	if (EquipmentItemSlot == EEquipmentItemSlot::None)
	{
		UnequipCurrentItem();
		if (bShouldUpdateCurrentSlotIndex)
		{
			SetCurrentSlotIndex(0);
		}
		return true;
	}

	AEquipmentItem* EquippedItem = CurrentEquipmentItem.Get();
	if (GetCurrentItemSlot() == EquipmentItemSlot && IsValid(EquippedItem))
	{
		return true;
	}

	int32 SlotIndex = (int32)EquipmentItemSlot;
	AEquipmentItem* ItemToEquip = EquippedItemsArray[SlotIndex];
	if (!IsValid(ItemToEquip))
	{
		return false;
	}

	if (IsPrimaryItemEquipped())
	{
		BaseCharacterOwner->SheathePrimaryItem();
	}

	UnequipItem(EquippedItem);
	EquipItem(ItemToEquip, bShouldSkipAnimation);
	if (bShouldUpdateCurrentSlotIndex)
	{
		SetCurrentSlotIndex(SlotIndex);
	}
	return true;
}

void UCharacterEquipmentComponent::Multicast_EquipItem_Implementation(EEquipmentItemSlot EquipmentItemSlot, bool bShouldSkipAnimation/* = false*/, bool bShouldUpdateCurrentSlotIndex/* = true*/)
{
	if (IsValid(BaseCharacterOwner) && BaseCharacterOwner->GetLocalRole() != ROLE_Authority)
	{
		EquipItemBySlot(EquipmentItemSlot, bShouldSkipAnimation, bShouldUpdateCurrentSlotIndex);
	}
}

void UCharacterEquipmentComponent::EquipItemFromCurrentSlot(bool bShouldSkipAnimation/* = false*/)
{
	EquipItemBySlot(GetCurrentItemSlot(), bShouldSkipAnimation);
}

void UCharacterEquipmentComponent::UnequipCurrentItem()
{
	UnequipItem(CurrentEquipmentItem.Get());
}

void UCharacterEquipmentComponent::AttachCurrentEquipmentItemToCharacterMesh()
{
	if (!IsValid(BaseCharacterOwner))
	{
		return;
	}

	AEquipmentItem* EquippedItem = CurrentEquipmentItem.Get();
	if (IsValid(EquippedItem))
	{
		EquippedItem->AttachToComponent(BaseCharacterOwner->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, EquippedItem->GetEquippedSocketName());
		EquippedItem->SetIsEquipped(true);

		if (IsCurrentWeaponMagazineEmpty() && CanReloadCurrentWeapon())
		{
			Multicast_OnWeaponMagazineEmpty();
		}
	}
}

void UCharacterEquipmentComponent::DrawNextItem()
{
	int32 NextSlotIndex = CurrentSlotIndex;
	GetNextSlotIndex(NextSlotIndex);
	EquipItemBySlot((EEquipmentItemSlot)NextSlotIndex);
}

void UCharacterEquipmentComponent::DrawPreviousItem()
{
	int32 PreviousSlotIndex = CurrentSlotIndex;
	GetPreviousSlotIndex(PreviousSlotIndex);
	EquipItemBySlot((EEquipmentItemSlot)PreviousSlotIndex);
}

EEquipmentItemSlot UCharacterEquipmentComponent::GetCurrentItemSlot() const
{
	return (CurrentSlotIndex < 0 || CurrentSlotIndex >= (int32)EEquipmentItemSlot::Max) ? EEquipmentItemSlot::None : (EEquipmentItemSlot)CurrentSlotIndex;
}

void UCharacterEquipmentComponent::SetCurrentSlotIndex(int32 NewSlotIndex)
{
	if (CurrentSlotIndex != NewSlotIndex && IsValid(BaseCharacterOwner) && BaseCharacterOwner->GetLocalRole() == ROLE_Authority)
	{
		CurrentSlotIndex = FMath::Clamp(NewSlotIndex, 0, (int32)EEquipmentItemSlot::Max - 1);
	}
}

void UCharacterEquipmentComponent::LoadoutOneItem(EEquipmentItemSlot EquipmentSlot, TSubclassOf<AEquipmentItem> EquipmentItemClass, USkeletalMeshComponent* SkeletalMesh, int32 CountInSlot/* = -1*/)
{
	if (CountInSlot == 0 || CountInSlot < -1 || !EquipmentItemClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = BaseCharacterOwner;
	AEquipmentItem* EquipmentItem = GetWorld()->SpawnActor<AEquipmentItem>(EquipmentItemClass, SpawnParameters);
	EquipmentItem->AttachToComponent(SkeletalMesh, FAttachmentTransformRules::KeepRelativeTransform, EquipmentItem->GetUnequippedSocketName());

	InitializeInventoryItem(EquipmentItem);
	UInventoryItem* InventoryItem = EquipmentItem->GetLinkedInventoryItem();
	if (IsValid(InventoryItem) && InventoryItem->CanStackItems())
	{
		// Applies only to equipment items that serve as ammo units, e.g. grenades
		// Filling the slot using ammo from EquipmentAmmoArray
		// Adding the remaining ammo to the inventory as items

		if (CountInSlot != -1) // -1 indicates loadout on BeginPlay
		{
			AddAmmo(EquipmentItem->GetAmmoType(), CountInSlot);
		}

		InventoryItem->SetCount(0);
		int32 AmmoToLoad = EquipmentAmmoArray[(uint32)EquipmentItem->GetAmmoType()];
		AmmoToLoad -= InventoryItem->AddCount(AmmoToLoad);
		SetAmmo(EquipmentItem->GetAmmoType(), InventoryItem->GetCount());
		BaseCharacterOwner->PickupItem(InventoryItem->GetInventoryItemType(), AmmoToLoad);
	}
	else
	{
		ARangedWeaponItem* RangedWeaponItem = Cast<ARangedWeaponItem>(EquipmentItem);
		if (IsValid(RangedWeaponItem))
		{
			for (int i = 0; i < RangedWeaponItem->GetWeaponModesArray().Num(); ++i)
			{
				RangedWeaponItem->SetCurrentWeaponMode(i, true);

				const FWeaponModeParameters* WeaponModeParameters = RangedWeaponItem->GetWeaponModeParameters(i);
				if (WeaponModeParameters && WeaponModeParameters->ReloadType == EWeaponReloadType::ByBullet)
				{
					LoadWeaponMagazineByBullet(RangedWeaponItem);
				}
				else
				{
					int32 AmmoToLoad = GetAvailableAmmoForWeaponMagazine(RangedWeaponItem);
					AmmoToLoad = RemoveAmmo(RangedWeaponItem->GetAmmoType(), AmmoToLoad);
					if (AmmoToLoad)
					{
						RangedWeaponItem->SetCurrentAmmo(AmmoToLoad);
					}
				}
			}
			RangedWeaponItem->SetCurrentWeaponMode(RangedWeaponItem->GetDefaultWeaponModeIndex(), true);
		}
	}

	AThrowableItem* ThrowableItem = Cast<AThrowableItem>(EquipmentItem);
	if (ThrowableItem && !CanItemBeThrown(ThrowableItem))
	{
		ThrowableItem->SetActorHiddenInGame(true);
	}

	EquippedItemsArray[(int32)EquipmentSlot] = EquipmentItem;
}

void UCharacterEquipmentComponent::InitializeInventoryItem(AEquipmentItem* EquipmentItem, int32 Count/* = 1*/) const
{
	if (!IsValid(EquipmentItem))
	{
		return;
	}

	const FInventoryTableRow* ItemData = LoadItemDataFromDataTable(EquipmentItem->GetEquipmentItemType());
	if (ItemData && ItemData->InventoryItemClass.LoadSynchronous())
	{
		UInventoryItem* NewItem = NewObject<UInventoryItem>(EquipmentItem, ItemData->InventoryItemClass.LoadSynchronous());
		NewItem->InitializeItem(ItemData->InventoryItemDescription);
		EquipmentItem->SetLinkedInventoryItem(NewItem);
		NewItem->SetCount(Count);
	}
}

EEquipmentItemSlot UCharacterEquipmentComponent::FindCompatibleSlot(AEquipmentItem* EquipmentItem)
{
	EEquipmentItemSlot EquipmentSlot = EEquipmentItemSlot::None;
	for (EEquipmentItemSlot Slot : EquipmentItem->GetCompatibleEquipmentSlots())
	{
		const AEquipmentItem* EquippedItem = EquippedItemsArray[(int32)Slot];
		if (IsValid(EquippedItem))
		{
			if (EquippedItem->GetEquipmentItemType() == EquipmentItem->GetEquipmentItemType())
			{
				const UInventoryItem* LinkedItem = EquippedItem->GetLinkedInventoryItem();
				if (!IsValid(LinkedItem) || !LinkedItem->CanStackItems())
				{
					continue;
				}
			}
			EquipmentSlot = EquipmentSlot == EEquipmentItemSlot::None ? Slot : EquipmentSlot;
		}
		else
		{
			EquipmentSlot = Slot;
			break;
		}
	}

	return EquipmentSlot;
}

void UCharacterEquipmentComponent::GetNextSlotIndex(int32& NextSlotIndex)
{
	NextSlotIndex++;
	if (NextSlotIndex == 0)
	{
		return;
	}
	if (NextSlotIndex >= EquipmentSlots.Num())
	{
		NextSlotIndex = 0;
		return;
	}

	if (WeaponSwitchIgnoredSlots.Contains((EEquipmentItemSlot)NextSlotIndex) || !IsValid(EquippedItemsArray[NextSlotIndex]))
	{
		GetNextSlotIndex(NextSlotIndex);
	}
}

void UCharacterEquipmentComponent::GetPreviousSlotIndex(int32& PreviousSlotIndex)
{
	PreviousSlotIndex--;
	if (PreviousSlotIndex == 0)
	{
		return;
	}
	if (PreviousSlotIndex < 0)
	{
		PreviousSlotIndex = EquipmentSlots.Num() - 1;
		return;
	}

	if (WeaponSwitchIgnoredSlots.Contains((EEquipmentItemSlot)PreviousSlotIndex) || !IsValid(EquippedItemsArray[PreviousSlotIndex]))
	{
		GetPreviousSlotIndex(PreviousSlotIndex);
	}
}

void UCharacterEquipmentComponent::EquipItem(AEquipmentItem* ItemToEquip, bool bShouldSkipAnimation/* = false*/)
{
	if (!IsValid(ItemToEquip) || !IsValid(BaseCharacterOwner))
	{
		return;
	}

	ARangedWeaponItem* RangedWeaponToEquip = Cast<ARangedWeaponItem>(ItemToEquip);
	AThrowableItem* ThrowableToEquip = nullptr;
	AMeleeWeaponItem* MeleeWeaponToEquip = nullptr;
	if (RangedWeaponToEquip)
	{
		CurrentWeaponReloadWalkSpeed = RangedWeaponToEquip->GetReloadingWalkSpeed();
		OnAmmoChangedDelegate = RangedWeaponToEquip->OnAmmoChangedEvent.AddUObject(this, &UCharacterEquipmentComponent::OnCurrentWeaponAmmoChanged);
		OnWeaponReloadedDelegate = RangedWeaponToEquip->OnWeaponReloadedEvent.AddLambda([this] { OnCurrentWeaponReloaded(true); });
		OnWeaponMagazineEmptyDelegate = RangedWeaponToEquip->OnMagazineEmptyEvent.AddUObject(this, &UCharacterEquipmentComponent::Multicast_OnWeaponMagazineEmpty);
	}
	else
	{
		ThrowableToEquip = Cast<AThrowableItem>(ItemToEquip);
		if (ThrowableToEquip)
		{
			CurrentThrowableWalkSpeed = ThrowableToEquip->GetThrowingWalkSpeed();
			OnItemThrownDelegate = ThrowableToEquip->OnItemThrownEvent.AddUObject(this, &UCharacterEquipmentComponent::OnItemThrown);
		}
		else
		{
			MeleeWeaponToEquip = Cast<AMeleeWeaponItem>(ItemToEquip);
		}
	}

	CurrentEquipmentItem = ItemToEquip;
	CurrentRangedWeapon = RangedWeaponToEquip;
	CurrentThrowableItem = ThrowableToEquip;
	CurrentMeleeWeapon = MeleeWeaponToEquip;

	UAnimMontage* EquipItemAnimMontage = ItemToEquip->GetEquipItemAnimMontage();
	if (!bShouldSkipAnimation && EquipItemAnimMontage)
	{
		bIsEquipAnimMontagePlaying = true;
		float Duration = BaseCharacterOwner->PlayAnimMontage(EquipItemAnimMontage) / EquipItemAnimMontage->RateScale;
		GetWorld()->GetTimerManager().SetTimer(EquipItemTimer, [this] { bIsEquipAnimMontagePlaying = false; }, Duration, false);
	}
	else
	{
		AttachCurrentEquipmentItemToCharacterMesh();
	}

	UpdateAmmoHUDWidgets();

	if (OnEquipmentItemChangedEvent.IsBound())
	{
		OnEquipmentItemChangedEvent.Broadcast(CurrentEquipmentItem.Get());
	}
}

void UCharacterEquipmentComponent::UnequipItem(AEquipmentItem* ItemToUnequip)
{
	if (!IsValid(ItemToUnequip) || !IsValid(BaseCharacterOwner))
	{
		return;
	}

	if (UAnimMontage* EquipItemAnimMontage = ItemToUnequip->GetEquipItemAnimMontage())
	{
		BaseCharacterOwner->StopAnimMontage(EquipItemAnimMontage);
	}

	if (ARangedWeaponItem* RangedWeaponToUnequip = Cast<ARangedWeaponItem>(ItemToUnequip))
	{
		BaseCharacterOwner->StopWeaponFire();
		BaseCharacterOwner->StopWeaponReload();
		RangedWeaponToUnequip->OnAmmoChangedEvent.Remove(OnAmmoChangedDelegate);
		RangedWeaponToUnequip->OnWeaponReloadedEvent.Remove(OnWeaponReloadedDelegate);
		RangedWeaponToUnequip->OnMagazineEmptyEvent.Remove(OnWeaponMagazineEmptyDelegate);
	}
	else if (AThrowableItem* ThrowableItemToUnequip = Cast<AThrowableItem>(ItemToUnequip))
	{
		if (UAnimMontage* ThrowItemAnimMontage = ThrowableItemToUnequip->GetEquipItemAnimMontage())
		{
			BaseCharacterOwner->StopAnimMontage(ThrowItemAnimMontage);
		}
		if (!CanItemBeThrown(ThrowableItemToUnequip))
		{
			ThrowableItemToUnequip->SetActorHiddenInGame(true);
		}
		ThrowableItemToUnequip->OnItemThrownEvent.Remove(OnItemThrownDelegate);
	}

	ItemToUnequip->AttachToComponent(BaseCharacterOwner->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, ItemToUnequip->GetUnequippedSocketName());
	ItemToUnequip->SetIsEquipped(false);
	CurrentEquipmentItem.Reset();
	CurrentRangedWeapon.Reset();
	CurrentWeaponReloadWalkSpeed = 0.f;
	CurrentThrowableItem.Reset();
	CurrentThrowableWalkSpeed = 0.f;
	CurrentMeleeWeapon.Reset();

	if (CurrentSlotIndex == 0)
	{
		UpdateAmmoHUDWidgets();
	}

	if (OnEquipmentItemChangedEvent.IsBound())
	{
		OnEquipmentItemChangedEvent.Broadcast(CurrentEquipmentItem.Get());
	}
}

void UCharacterEquipmentComponent::OnRep_EquippedItemsArray()
{
	// Temporary solution for cases when spawned equipment items are not yet replicated to clients
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]
	{
		if (EquipmentViewWidget)
		{
			EquipmentViewWidget->UpdateAllSlots();
		}
		CloseRadialMenu();

		if (IsValid(BaseCharacterOwner))
		{
			BaseCharacterOwner->EquipItemFromCurrentSlot();
		}
	}, .01f, false);
}

void UCharacterEquipmentComponent::OnRep_EquipmentAmmoArray()
{
	UpdateAmmoHUDWidgets();
}
#pragma endregion

#pragma region RANGED WEAPONS

void UCharacterEquipmentComponent::SetAmmo(EWeaponAmmoType AmmoType, int32 NewAmmo)
{
	if (NewAmmo >= 0)
	{
		EquipmentAmmoArray[(uint32)AmmoType] = NewAmmo;
	}
}

bool UCharacterEquipmentComponent::AddAmmo(EWeaponAmmoType AmmoType, int32 Amount)
{
	if (Amount < 1 || AmmoType == EWeaponAmmoType::None || !IsValid(BaseCharacterOwner))
	{
		return false;
	}

	if (BaseCharacterOwner->AddAmmoToInventory(AmmoType, Amount))
	{
		EquipmentAmmoArray[(uint32)AmmoType] += Amount;
		UpdateAmmoHUDWidgets();
		return true;
	}

	return false;
}

int32 UCharacterEquipmentComponent::RemoveAmmo(EWeaponAmmoType AmmoType, int32 Amount)
{
	if (Amount < 1 || AmmoType == EWeaponAmmoType::None || !IsValid(BaseCharacterOwner))
	{
		return 0;
	}

	uint32 AmmoIndex = (uint32)AmmoType;
	int32 AmmoToRemove = Amount < (int32)EquipmentAmmoArray[AmmoIndex] ? Amount : EquipmentAmmoArray[AmmoIndex];
	AmmoToRemove = BaseCharacterOwner->RemoveAmmoFromInventory(AmmoType, AmmoToRemove);
	EquipmentAmmoArray[AmmoIndex] -= AmmoToRemove;

	return AmmoToRemove;
}

bool UCharacterEquipmentComponent::IsCurrentWeaponMagazineEmpty() const
{
	ARangedWeaponItem* EquippedRangedWeapon = CurrentRangedWeapon.Get();
	return IsValid(EquippedRangedWeapon) && EquippedRangedWeapon->GetCurrentAmmo() <= 0;
}

bool UCharacterEquipmentComponent::IsCurrentWeaponMagazineFull() const
{
	ARangedWeaponItem* EquippedRangedWeapon = CurrentRangedWeapon.Get();
	return IsValid(EquippedRangedWeapon) && (EquippedRangedWeapon->GetMagazineSize() - EquippedRangedWeapon->GetCurrentAmmo()) <= 0;
}

int32 UCharacterEquipmentComponent::GetAvailableAmmoForWeaponMagazine(ARangedWeaponItem* RangedWeaponItem)
{
	if (IsValid(RangedWeaponItem))
	{
		int32 AvailableEquipmentAmmo = EquipmentAmmoArray[(uint32)RangedWeaponItem->GetAmmoType()];
		if (RangedWeaponItem->GetReloadType() == EWeaponReloadType::ByBullet)
		{
			return FMath::Min(1, AvailableEquipmentAmmo);
		}

		int32 AvailableSpaceInWeaponMagazine = RangedWeaponItem->GetMagazineSize() - RangedWeaponItem->GetCurrentAmmo();
		return FMath::Min(AvailableSpaceInWeaponMagazine, AvailableEquipmentAmmo);
	}
	return 0;
}

void UCharacterEquipmentComponent::ActivateNextWeaponMode()
{
	ARangedWeaponItem* EquippedRangedWeapon = CurrentRangedWeapon.Get();
	if (IsValid(EquippedRangedWeapon))
	{
		if (UAnimMontage* EquipAnimMontage = EquippedRangedWeapon->GetEquipItemAnimMontage())
		{
			BaseCharacterOwner->StopAnimMontage(EquipAnimMontage);
		}

		EquippedRangedWeapon->StopFire();
		EquippedRangedWeapon->StopReload();
		EquippedRangedWeapon->SetCurrentWeaponMode(EquippedRangedWeapon->GetCurrentWeaponModeIndex() + 1);

		if (IsCurrentWeaponMagazineEmpty() && CanReloadCurrentWeapon())
		{
			OnWeaponMagazineEmpty();
		}
	}
}

bool UCharacterEquipmentComponent::CanReloadCurrentWeapon()
{
	ARangedWeaponItem* EquippedRangedWeapon = CurrentRangedWeapon.Get();
	return IsValid(EquippedRangedWeapon) && !IsCurrentWeaponMagazineFull() && GetAvailableAmmoForWeaponMagazine(EquippedRangedWeapon) > 0;
}

bool UCharacterEquipmentComponent::TryReloadCurrentWeapon()
{
	if (CanReloadCurrentWeapon())
	{
		CurrentRangedWeapon->StartReload();
		return true;
	}
	return false;
}

void UCharacterEquipmentComponent::TryReloadNextBullet()
{
	ARangedWeaponItem* EquippedRangedWeapon = CurrentRangedWeapon.Get();
	if (!IsValid(EquippedRangedWeapon))
	{
		return;
	}

	if (IsCurrentWeaponMagazineFull() || EquipmentAmmoArray[(uint32)EquippedRangedWeapon->GetAmmoType()] <= 0)
	{
		EquippedRangedWeapon->StopReload();
	}
	else if (EquippedRangedWeapon->GetMagazineSize() - EquippedRangedWeapon->GetCurrentAmmo() <= 1)
	{
		JumpToAnimMontageSection(EquippedRangedWeapon->GetReloadEndSectionName());
	}
	else
	{
		OnCurrentWeaponReloaded(false);
		JumpToAnimMontageSection(EquippedRangedWeapon->GetReloadLoopStartSectionName());
	}
}

void UCharacterEquipmentComponent::LoadWeaponMagazineByBullet(ARangedWeaponItem* RangedWeaponItem)
{
	int32 AmmoToLoad = 0;
	for (int32 Bullet = 0; Bullet < RangedWeaponItem->GetMagazineSize(); Bullet++)
	{
		int32 AvailableAmmo = GetAvailableAmmoForWeaponMagazine(RangedWeaponItem);
		if (AvailableAmmo == 0)
		{
			break;
		}
		AmmoToLoad += AvailableAmmo;
	}

	AmmoToLoad = RemoveAmmo(RangedWeaponItem->GetAmmoType(), AmmoToLoad);
	if (AmmoToLoad)
	{
		RangedWeaponItem->SetCurrentAmmo(AmmoToLoad);
	}
}

void UCharacterEquipmentComponent::OnCurrentWeaponAmmoChanged(int32 NewAmmo)
{
	if (!OnCurrentWeaponAmmoChangedEvent.IsBound())
	{
		return;
	}

	ARangedWeaponItem* EquippedRangedWeapon = CurrentRangedWeapon.Get();
	if (IsValid(EquippedRangedWeapon))
	{
		uint32 EquipmentAmmo = EquipmentAmmoArray[(uint32)EquippedRangedWeapon->GetAmmoType()];
		OnCurrentWeaponAmmoChangedEvent.Broadcast(NewAmmo, EquipmentAmmo);
	}
	else
	{
		OnCurrentWeaponAmmoChangedEvent.Broadcast(0, 0);
	}
}

void UCharacterEquipmentComponent::OnCurrentWeaponReloaded(bool bIsReloadComplete)
{
	if (IsValid(BaseCharacterOwner) && BaseCharacterOwner->GetLocalRole() == ROLE_Authority)
	{
		ARangedWeaponItem* EquippedRangedWeapon = CurrentRangedWeapon.Get();
		if (IsValid(EquippedRangedWeapon))
		{
			int32 ReloadedAmmo = GetAvailableAmmoForWeaponMagazine(EquippedRangedWeapon);
			ReloadedAmmo = RemoveAmmo(EquippedRangedWeapon->GetAmmoType(), ReloadedAmmo);
			if (ReloadedAmmo)
			{
				EquippedRangedWeapon->SetCurrentAmmo(EquippedRangedWeapon->GetCurrentAmmo() + ReloadedAmmo);
			}
		}
	}

	if (bIsReloadComplete)
	{
		BaseCharacterOwner->StopWeaponReload();
	}
}

void UCharacterEquipmentComponent::OnWeaponMagazineEmpty()
{
	if (IsValid(BaseCharacterOwner) && BaseCharacterOwner->IsLocallyControlled())
	{
		BaseCharacterOwner->StartWeaponAutoReload();
	}
}

void UCharacterEquipmentComponent::Multicast_OnWeaponMagazineEmpty_Implementation()
{
	OnWeaponMagazineEmpty();
}
#pragma endregion

#pragma region OTHER ITEMS

void UCharacterEquipmentComponent::EquipPrimaryItem(bool bForceEquip/* = false*/)
{
	if (!bForceEquip && IsPrimaryItemEquipped())
	{
		return;
	}

	AEquipmentItem* PrimaryItem = EquippedItemsArray[(int32)EEquipmentItemSlot::PrimaryItem];
	if (IsValid(PrimaryItem) && CanItemBeThrown(Cast<AThrowableItem>(PrimaryItem)))
	{
		UnequipCurrentItem();
		// Primary items should not change CurrentSlotIndex as they are drawn using a separate function DrawPrimaryItem()
		if (EquipItemBySlot(EEquipmentItemSlot::PrimaryItem, false, false, false))
		{
			PrimaryItem->SetActorHiddenInGame(false);
			bIsPrimaryItemEquipped = true;
		}
	}
}

void UCharacterEquipmentComponent::UnequipPrimaryItem()
{
	if (!IsPrimaryItemEquipped())
	{
		return;
	}

	UnequipCurrentItem();
	BaseCharacterOwner->EquipItemFromCurrentSlot();
	bIsPrimaryItemEquipped = false;
}

void UCharacterEquipmentComponent::UpdatePrimaryItemSlot()
{
	uint32 SlotIndex = (uint32)EEquipmentItemSlot::PrimaryItem; 
	if (EquipmentAmmoArray[SlotIndex] <= 0)
	{
		RemoveEquipmentItem(SlotIndex);
	}
}

bool UCharacterEquipmentComponent::CanItemBeThrown(AThrowableItem* ThrowableItem)
{
	return IsValid(ThrowableItem) && EquipmentAmmoArray[(int32)ThrowableItem->GetAmmoType()] > 0;
}

void UCharacterEquipmentComponent::ThrowItem()
{
	AThrowableItem* ItemToThrow = CurrentThrowableItem.Get();
	if (!CanItemBeThrown(ItemToThrow))
	{
		return;
	}

	TSoftClassPtr<AXyzProjectile> ProjectileClass = ItemToThrow->GetProjectileClass();
	FProjectilePool* ProjectilePool = ProjectilePools.FindByPredicate([ProjectileClass](const FProjectilePool& Pool) { return Pool.ProjectileClass == ProjectileClass; });
	if (ProjectilePool)
	{
		AXyzProjectile* ThrowableProjectile = ProjectilePool->GetNextAvailableProjectile();
		if (IsValid(ThrowableProjectile))
		{
			ItemToThrow->Throw(ThrowableProjectile, ProjectilePool->PoolWorldLocation);
		}
	}
}

void UCharacterEquipmentComponent::OnItemThrown()
{
	AThrowableItem* ThrownItem = CurrentThrowableItem.Get();
	if (IsValid(ThrownItem) && IsValid(BaseCharacterOwner) && BaseCharacterOwner->GetLocalRole() == ROLE_Authority)
	{
		RemoveAmmo(ThrownItem->GetAmmoType(), 1);
		OnCurrentThrowableAmmoChanged(EquipmentAmmoArray[(uint32)ThrownItem->GetAmmoType()]);
	}
}

void UCharacterEquipmentComponent::OnCurrentThrowableAmmoChanged(int32 NewAmmo) const
{
	if (OnCurrentThrowableAmmoChangedEvent.IsBound())
	{
		OnCurrentThrowableAmmoChangedEvent.Broadcast(NewAmmo);
	}
}
#pragma endregion

#pragma region WIDGETS

bool UCharacterEquipmentComponent::IsViewEquipmentVisible() const
{
	if (EquipmentViewWidget)
	{
		return EquipmentViewWidget->IsVisible();
	}
	return false;
}

void UCharacterEquipmentComponent::OpenViewEquipment(APlayerController* PlayerController)
{
	if (!EquipmentViewWidget)
	{
		CreateEquipmentViewWidget(PlayerController);
	}

	if (EquipmentViewWidget && !EquipmentViewWidget->IsVisible())
	{
		EquipmentViewWidget->AddToViewport();
	}
}

void UCharacterEquipmentComponent::CloseViewEquipment() const
{
	if (EquipmentViewWidget->IsVisible())
	{
		EquipmentViewWidget->RemoveFromParent();
	}
}

bool UCharacterEquipmentComponent::IsRadialMenuVisible() const
{
	if (RadialMenuWidget)
	{
		return RadialMenuWidget->IsVisible();
	}
	return false;
}

void UCharacterEquipmentComponent::OpenRadialMenu(APlayerController* PlayerController)
{
	if (!RadialMenuWidget)
	{
		CreateRadialMenuWidget(PlayerController);
	}

	if (RadialMenuWidget && !RadialMenuWidget->IsVisible())
	{
		RadialMenuWidget->AddToViewport();
		RadialMenuWidget->UpdateMenuSegmentWidgets();
	}
}

void UCharacterEquipmentComponent::CloseRadialMenu() const
{
	if (RadialMenuWidget && RadialMenuWidget->IsVisible())
	{
		RadialMenuWidget->RemoveFromParent();
	}
}

void UCharacterEquipmentComponent::OnEquipmentSlotUpdated(int32 SlotIndex)
{
	UpdateAmmoHUDWidgets();
}

void UCharacterEquipmentComponent::UpdateAmmoHUDWidgets()
{
	if (!IsValid(BaseCharacterOwner) || !BaseCharacterOwner->IsPlayerControlled())
	{
		return;
	}

	ARangedWeaponItem* EquippedRangedItem = CurrentRangedWeapon.Get();
	if (IsValid(EquippedRangedItem))
	{
		OnCurrentWeaponAmmoChanged(EquippedRangedItem->GetCurrentAmmo());
	}
	else
	{
		OnCurrentWeaponAmmoChanged(0);
	}

	AThrowableItem* EquippedThrowable = CurrentThrowableItem.Get();
	if (IsValid(EquippedThrowable))
	{
		OnCurrentThrowableAmmoChanged(EquipmentAmmoArray[(int32)EquippedThrowable->GetAmmoType()]);
	}
	else
	{
		// Update the primary item ammo UI even if no throwables are equipped
		AThrowableItem* PrimaryItem = Cast<AThrowableItem>(EquippedItemsArray[(int32)EEquipmentItemSlot::PrimaryItem]);
		if (IsValid(PrimaryItem))
		{
			OnCurrentThrowableAmmoChanged(EquipmentAmmoArray[(int32)PrimaryItem->GetAmmoType()]);
		}
		else
		{
			OnCurrentThrowableAmmoChanged(0);
		}
	}
}

void UCharacterEquipmentComponent::CreateEquipmentViewWidget(APlayerController* PlayerController)
{
	if (EquipmentViewWidget || !IsValid(PlayerController) || !EquipmentViewWidgetClass.LoadSynchronous())
	{
		return;
	}

	EquipmentViewWidget = CreateWidget<UEquipmentViewWidget>(PlayerController, EquipmentViewWidgetClass.LoadSynchronous());
	EquipmentViewWidget->InitializeWidget(BaseCharacterOwner);
}

void UCharacterEquipmentComponent::CreateRadialMenuWidget(APlayerController* PlayerController)
{
	if (RadialMenuWidget || !IsValid(PlayerController) || !RadialMenuWidgetClass.LoadSynchronous())
	{
		return;
	}

	RadialMenuWidget = CreateWidget<URadialMenuWidget>(PlayerController, RadialMenuWidgetClass.LoadSynchronous());
	RadialMenuWidget->InitializeWidget(this);
	RadialMenuWidget->OnMenuSegmentSelectedEvent.AddLambda([this]
	{
		CloseRadialMenu();
		if (IsValid(BaseCharacterOwner))
		{
			BaseCharacterOwner->TogglePlayerMouseInput(Cast<APlayerController>(BaseCharacterOwner->GetController()));
		}
	});
}
#pragma endregion
