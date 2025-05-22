// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "MeleeHitRegistrationComponent.generated.h"

/**
 *
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class XYZHOMEWORK_API UMeleeHitRegistrationComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHitRegisteredEvent, FVector, const FHitResult&);
	FOnHitRegisteredEvent OnHitRegisteredEvent;

	UMeleeHitRegistrationComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	bool IsHitRegistrationEnabled() const { return bIsHitRegistrationEnabled; }
	void EnableHitRegistration(bool bIsHitRegistrationEnabled_In);

private:
	void ProcessHitRegistration();

	TWeakObjectPtr<APawn> CachedPawn;
	bool bIsHitRegistrationEnabled = false;
	FVector PreviousComponentLocation = FVector::ZeroVector;
};
