// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "BasePlatform.generated.h"

UENUM()
enum class EPlatformBehavior : uint8
{
	OnDemand,
	Loop,
	Default
};

/**
 *
 */
UCLASS()
class XYZHOMEWORK_API ABasePlatform : public AActor
{
	GENERATED_BODY()

public:
	ABasePlatform();
	UFUNCTION(BlueprintCallable)
	void PlatformTimelinePlay() { PlatformTimeline.Play(); }
	void PlatformTimelineUpdate(float Alpha) const;
	UFUNCTION(BlueprintCallable)
	void PlatformTimelineReverse() { PlatformTimeline.Reverse(); }
	void OnPlatformInvoked();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* PlatformMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient)
	FVector StartLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector EndLocation;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* TimelineCurve;
	FTimeline PlatformTimeline;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.f, UIMin = 0.f))
	float MovementCooldown = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EPlatformBehavior PlatformBehavior = EPlatformBehavior::OnDemand;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	class APlatformInvocator* PlatformInvocator;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaSeconds) override;
};
