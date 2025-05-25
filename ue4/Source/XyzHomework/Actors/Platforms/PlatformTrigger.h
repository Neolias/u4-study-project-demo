// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

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
	/** Platforms that will be invoked when pawns interact with this trigger. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<ABasePlatform*> ControlledPlatforms;

private:
	void OnSetIsTriggered() const;
	/** Adds a new actor to the list of 'OverlappedPawns'. */
	UFUNCTION()
	void RegisterPawns(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	/** Removes an actor from the list of 'OverlappedPawns'. */
	UFUNCTION()
	void UnRegisterPawns(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(ReplicatedUsing = OnRep_SetIsTriggered)
	bool bIsTriggered = false;
	UFUNCTION()
	void OnRep_SetIsTriggered(bool bIsTriggered_Old);
	UPROPERTY(Transient)
	TArray<APawn*> OverlappedPawns;
};
