// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "Actors/Equipment/EquipmentItem.h"
#include "ThrowableItem.generated.h"

class AXyzProjectile;
class AExplosiveProjectile;

/** Base class of all throwable items, such as grenades. */
UCLASS()
class XYZHOMEWORK_API AThrowableItem : public AEquipmentItem
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE(FOnThrowEndEvent)
	DECLARE_MULTICAST_DELEGATE(FOnThrowAnimationFinishedEvent)
	FOnThrowEndEvent OnItemThrownEvent;
	FOnThrowAnimationFinishedEvent OnThrowAnimationFinishedEvent;

	AThrowableItem();
	virtual EWeaponAmmoType GetAmmoType() override;
	TSoftClassPtr<AXyzProjectile> GetProjectileClass() const { return ProjectileClass; }
	float GetThrowingWalkSpeed() const { return ThrowingWalkSpeed; }
	/**
	 * Plays a throwing anim montage.
	 * @param ThrowableProjectile projectile that will be thrown.
	 * @param ResetLocation Location where the projectile will idle while it is not used (location of the projectile pool). 
	 */
	void Throw(AXyzProjectile* ThrowableProjectile, const FVector ResetLocation);
	/** Binds delegates and launches the 'ThrowableProjectile' projectile passed to the Throw() function. */
	void LaunchProjectile();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Throwable Parameters")
	UStaticMeshComponent* StaticMesh;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable Parameters")
	EWeaponAmmoType AmmoType = EWeaponAmmoType::Grenade;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable Parameters")
	TSoftClassPtr<AXyzProjectile> ProjectileClass;
	/** Socket on the character mesh where projectiles are spawned. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable Parameters")
	FName ThrowableSocketName = "ThrowableSocket";
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Throwable Parameters", meta = (ClampMin = -90.f, UIMin = -90.f, ClampMax = 90.f, UIMax = 90.f))
	float LaunchPitchAngle = 30.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Throwable Parameters", meta = (ClampMin = -90.f, UIMin = -90.f, ClampMax = 90.f, UIMax = 90.f))
	float LaunchYawAngle = 0.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable Parameters")
	UAnimMontage* ThrowItemAnimMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Throwable Parameters", meta = (ClampMin = 0.f, UIMin = 0.f))
	float ThrowingWalkSpeed = 150.f;

private:
	void OnThrowEnd() const;
	void OnThrowAnimationFinished();
	void ProcessThrowableProjectileHit(AXyzProjectile* Projectile, FVector MovementDirection, const FHitResult& HitResult, FVector ResetLocation);
	void OnProjectileExplosion(AExplosiveProjectile* ExplosiveProjectile, FVector ResetLocation);
	void ResetThrowableProjectile(AXyzProjectile* Projectile, FVector ResetLocation);

	/** Location where the projectile will idle while it is not used (location of the projectile pool). */
	FVector ProjectileResetLocation = FVector::ZeroVector;
	TWeakObjectPtr<AXyzProjectile> CurrentProjectile;
	FTimerHandle ThrowAnimationTimer;
};
