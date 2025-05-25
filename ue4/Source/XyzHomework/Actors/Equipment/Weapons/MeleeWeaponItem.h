// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericStructs.h"
#include "Actors/Equipment/EquipmentItem.h"
#include "MeleeWeaponItem.generated.h"

class UMeleeHitRegistrationComponent;

/** Base class of all melee weapon items, such as knives. */
UCLASS()
class XYZHOMEWORK_API AMeleeWeaponItem : public AEquipmentItem
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnAttackActivated, bool);
	FOnAttackActivated OnAttackActivatedEvent;

	AMeleeWeaponItem();
	/**
	 * Starts a melee attack based on 'EMeleeAttackType'. Plays an attack anim montage.
	 * @param AttackType ID of the attack description stored in the 'Attacks' list.
	 */
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
	/** Hit components with collisions. Hit events are processed in ProcessHit(). */
	UPROPERTY()
	TArray<UMeleeHitRegistrationComponent*> HitRegistrationComponents;
	UPROPERTY()
	TArray<AActor*> HitActors;
	FTimerHandle AttackTimer;
};
