// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "PlatformTrigger.generated.h"

class ABasePlatform;

UCLASS()
class XYZHOMEWORK_API APlatformTrigger : public AActor
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTriggerStatusChanged, bool, bIsTriggered);

	UPROPERTY(BlueprintAssignable)
	FOnTriggerStatusChanged OnTriggerStatusChanged;

	APlatformTrigger();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetIsTriggered(bool bIsTriggered_In);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* TriggerMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UBoxComponent* TriggerCollision;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<ABasePlatform*> ControlledPlatforms;

private:
	void OnSetIsTriggered() const;
	UFUNCTION()
	void RegisterPawns(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void UnRegisterPawns(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(ReplicatedUsing = OnRep_SetIsTriggered)
	bool bIsTriggered = false;
	UFUNCTION()
	void OnRep_SetIsTriggered(bool bIsTriggered_Old);
	UPROPERTY(Transient)
	TArray<APawn*> OverlappedPawns;
};
