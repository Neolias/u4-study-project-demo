// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Actor.h"
#include "BasePlatform.generated.h"

UENUM(BlueprintType)
enum class EPlatformBehavior : uint8
{
	OnDemand = 0,
	Loop,
	Default,
	Max UMETA(Hidden)
};

/** Base class of all movable platforms. */
UCLASS()
class XYZHOMEWORK_API ABasePlatform : public AActor
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlatformStatusChangedEvent, bool, bIsActivated);

	UPROPERTY(BlueprintAssignable)
	FOnPlatformStatusChangedEvent OnPlatformStatusChanged;

	ABasePlatform();
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetIsActivated(bool bIsActivated_In);
	void InvokePlatform();
	void ResetPlatform();

protected:
	virtual void BeginPlay() override;
	UFUNCTION(BlueprintCallable)
	void PlatformTimelinePlay() { PlatformTimeline.Play(); }
	UFUNCTION(BlueprintCallable)
	void PlatformTimelineReverse() { PlatformTimeline.Reverse(); }
	UFUNCTION(BlueprintCallable)
	void PlatformTimelinePlayFromStart() { PlatformTimeline.PlayFromStart(); }
	UFUNCTION(BlueprintCallable)
	void PlatformTimelineReverseFromEnd() { PlatformTimeline.ReverseFromEnd(); }

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* PlatformMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UBoxComponent* PlatformCollision;
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Transient)
	FVector StartLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector EndLocation;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* TimelineCurve;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.f, UIMin = 0.f))
	float MovementCooldown = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EPlatformBehavior PlatformBehavior = EPlatformBehavior::OnDemand;

private:
	void OnSetIsActivated() const;
	void PlatformTimelineUpdate(float Alpha);

	UPROPERTY(ReplicatedUsing = OnRep_SetIsActivated)
	bool bIsActivated = false;
	UFUNCTION()
	void OnRep_SetIsActivated(bool bIsActivated_Old);
	FTimeline PlatformTimeline;
	FTimerHandle PlatformIdleTimer;
};
