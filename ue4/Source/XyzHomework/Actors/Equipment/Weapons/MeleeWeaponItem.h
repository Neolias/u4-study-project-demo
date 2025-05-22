// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericStructs.h"
#include "Actors/Equipment/EquipmentItem.h"
#include "MeleeWeaponItem.generated.h"

class UMeleeHitRegistrationComponent;

/**
 *
 */
UCLASS()
class XYZHOMEWORK_API AMeleeWeaponItem : public AEquipmentItem
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnAttackActivated, bool);
	FOnAttackActivated OnAttackActivatedEvent;

	AMeleeWeaponItem();
	void StartAttack(EMeleeAttackType AttackType);
	void EnableHitRegistration(bool bIsHitRegistrationEnabled);

protected:
	virtual void BeginPlay() override;
	void ProcessHit(FVector MovementDirection, const FHitResult& HitResult);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Item|Melee Weapon")
	UStaticMeshComponent* MeshComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Item|Melee Weapon")
	TMap<EMeleeAttackType, FMeleeAttackDescription> Attacks;

private:
	void EndAttack() const;

	FMeleeAttackDescription* CurrentAttackDescription;
	UPROPERTY()
	TArray<UMeleeHitRegistrationComponent*> HitRegistrationComponents;
	UPROPERTY()
	TArray<AActor*> HitActors;
	FTimerHandle AttackTimer;
};
