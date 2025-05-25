// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "ExplosionComponent.generated.h"

/** Component that can be attached to an actor to add explosion damage and effects. */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class XYZHOMEWORK_API UExplosionComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExplostionEvent);
	UPROPERTY(BlueprintAssignable, Category = "Explosion Component")
	FOnExplostionEvent OnExplosionEvent;

	UExplosionComponent();
	virtual void BeginPlay() override;
	/** Applies radial damage and spawns visual effects. */
	void Explode();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 1.f, UIMin = 1.f), Category = "Explostion Component")
	float BaseDamage = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 1.f, UIMin = 1.f), Category = "Explostion Component")
	float MinimumDamage = 20.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 1.f, UIMin = 1.f), Category = "Explostion Component")
	float InnerRadius = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 1.f, UIMin = 1.f), Category = "Explostion Component")
	float OuterRadius = 500.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 1.f, UIMin = 1.f), Category = "Explostion Component")
	float DamageFalloff = 1.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explostion Component")
	TSoftClassPtr<UDamageType> DamageType;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explostion Component")
	UParticleSystem* ExplosionVFX;

private:
	APawn* GetOwningPawn() const;
	UFUNCTION()
	void OnDamageTaken(AActor* DamagedActor, float Damage, const class UDamageType* DamageType_In, class AController* InstigatedBy, AActor* DamageCauser);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Explode();
};
