// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "GameFramework/Actor.h"
#include "Subsystems/SaveSubsystem/SaveSubsystemInterface.h"
#include "EquipmentItem.generated.h"

class UInventoryItem;
class AXyzBaseCharacter;

UCLASS()
class XYZHOMEWORK_API AEquipmentItem : public AActor, public ISaveSubsystemInterface
{
	GENERATED_BODY()

public:	
	AEquipmentItem();
	virtual void SetOwner(AActor* NewOwner) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	EEquipmentItemType GetEquipmentItemType() const { return EquipmentItemType; }
	virtual EWeaponAmmoType GetAmmoType() { return EWeaponAmmoType::None; }
	FName GetEquippedSocketName() const { return EquippedSocketName; }
	FName GetUnequippedSocketName() const { return UnequippedSocketName; }
	UAnimMontage* GetEquipItemAnimMontage() const { return EquipItemAnimMontage; }
	EReticleType GetReticleType() const { return ReticleType; }
	EReticleType GetAimingReticleType() const { return AimingReticleType; }
	bool IsEquipped() const { return bIsEquipped; }
	void SetIsEquipped(bool bIsEquipped_In) { bIsEquipped = bIsEquipped_In; }
	bool CanAimWithThisItem() const { return bCanAimWithThisItem; }
	float GetAimingWalkSpeed() const { return AimingWalkSpeed; }
	float GetAimingFOV() const { return AimingFOV; }
	UInventoryItem* GetLinkedInventoryItem() const { return LinkedInventoryItem; }
	void SetLinkedInventoryItem(UInventoryItem* InventoryItem);
	const TArray<EEquipmentItemSlot>& GetCompatibleEquipmentSlots() const { return CompatibleEquipmentSlots; }
	bool IsEquipmentSlotCompatible(EEquipmentItemSlot EquipmentSlot) const;

	//@ SaveSubsystemInterface
	virtual void OnLevelDeserialized_Implementation() override;
	//~ SaveSubsystemInterface

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Item")
	FName EquippedSocketName = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Item")
	FName UnequippedSocketName = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Item")
	EEquipmentItemType EquipmentItemType = EEquipmentItemType::None;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Item")
	EInventoryItemType InventoryItemType = EInventoryItemType::None;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Item")
	TArray<EEquipmentItemSlot> CompatibleEquipmentSlots;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Item")
	UAnimMontage* EquipItemAnimMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Item")
	EReticleType ReticleType = EReticleType::None;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Item")
	EReticleType AimingReticleType = EReticleType::Default;
	UPROPERTY(EditAnywhere, Category = "Equipment Item")
	bool bCanAimWithThisItem = true;
	UPROPERTY(EditAnywhere, Category = "Equipment Item", meta = (ClampMin = 0.f, UIMin = 0.f))
	float AimingWalkSpeed = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Item", meta = (ClampMin = 0.f, UIMin = 0.f, ClampMax = 120.f, UIMax = 120.f))
	float AimingFOV = 90.f;

	TWeakObjectPtr<AXyzBaseCharacter> CachedBaseCharacter;
	bool bIsEquipped = false;
	UPROPERTY(Replicated)
	UInventoryItem* LinkedInventoryItem;
};
