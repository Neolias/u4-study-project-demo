// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TurretAttributesComponent.generated.h"

class ATurret;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class XYZHOMEWORK_API UTurretAttributesComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE(FOnDeathEvent);
	FOnDeathEvent OnDeathEvent;

	UTurretAttributesComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	float GetCurrentHealth() const { return CurrentHealth; }
	bool IsAlive() const;
	UFUNCTION()
	void OnDamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

protected:
	/** Distance at which the attributes debug text font scale is equal to 1. */
	UPROPERTY(EditAnywhere, Category = "AI Turret|Attributes Component|Debug", meta = (ClampMin = 0.f, UIMin = 0.f))
	float DefaultDebugDistanceFromCamera = 175.f;
	UPROPERTY(EditAnywhere, Category = "AI Turret|Attributes Component|Debug", meta = (ClampMin = 0.f, UIMin = 0.f))
	float DebugAttributesVisibilityRange = 1000.f;
	UPROPERTY(EditAnywhere, Category = "AI Turret|Attributes Component|Debug", meta = (ClampMin = 0.f, UIMin = 0.f))
	float AttributesFontSize = 1.5f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Turret|Attributes Component|Debug")
	float MaxHealth = 100.f;
	UPROPERTY(EditAnywhere, Category = "AI Turret|Attributes Component|Debug")
	FVector HealthBarOffset = FVector(0.f, -10.f, 35.f);

private:
	void TryTriggerDeath();

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	void DrawDebugAttributes() const;
#endif

	UPROPERTY()
	ATurret* TurretOwner;
	float CurrentHealth = 0.f;
	bool bIsDeathTriggered = false;
};
