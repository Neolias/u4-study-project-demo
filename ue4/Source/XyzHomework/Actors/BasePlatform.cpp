// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlatform.h"

#include "PlatformInvocator.h"

ABasePlatform::ABasePlatform()
{
	PrimaryActorTick.bCanEverTick = true;
	USceneComponent* DefaultPlatformRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Platform root"));
	RootComponent = DefaultPlatformRoot;

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Platform"));
	PlatformMesh->SetupAttachment(DefaultPlatformRoot);
}

void ABasePlatform::BeginPlay()
{
	Super::BeginPlay();
	StartLocation = PlatformMesh->GetRelativeLocation();

	if (IsValid(TimelineCurve))
	{
		FOnTimelineFloatStatic PlatformMovementTimelineUpdate;
		PlatformMovementTimelineUpdate.BindUObject(this, &ABasePlatform::PlatformTimelineUpdate);
		PlatformTimeline.AddInterpFloat(TimelineCurve, PlatformMovementTimelineUpdate);
	}

	if (IsValid(PlatformInvocator))
	{
		PlatformInvocator->OnInvocatorActivated.AddUObject(this, &ABasePlatform::OnPlatformInvoked);
	}
}

void ABasePlatform::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	PlatformTimeline.TickTimeline(DeltaSeconds);
}

void ABasePlatform::PlatformTimelineUpdate(const float Alpha) const
{
	const FVector PlatformTargetLocation = FMath::Lerp(StartLocation, EndLocation, Alpha);
	PlatformMesh->SetRelativeLocation(PlatformTargetLocation);
}

void ABasePlatform::OnPlatformInvoked()
{
	PlatformTimelinePlay();

	FTimerHandle PlatformIdleTimer;
	FTimerDelegate PlatformIdleDelegate;
	PlatformIdleDelegate.BindUFunction(this, FName("PlatformTimelineReverse"));
	GetWorldTimerManager().SetTimer(PlatformIdleTimer, PlatformIdleDelegate, MovementCooldown, false);
}
