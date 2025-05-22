// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "ExplosionComponent.generated.h"

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
