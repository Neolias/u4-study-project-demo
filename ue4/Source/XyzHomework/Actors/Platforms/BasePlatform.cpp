// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "BasePlatform.h"

#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"

ABasePlatform::ABasePlatform()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* PlatformRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = PlatformRoot;

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
	PlatformMesh->SetupAttachment(PlatformRoot);

	PlatformCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerCollision"));
	PlatformCollision->SetupAttachment(PlatformRoot);

	bReplicates = true;
	SetReplicateMovement(true);
	NetUpdateFrequency = 2.f;
	MinNetUpdateFrequency = 2.f;
}

void ABasePlatform::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	PlatformTimeline.TickTimeline(DeltaSeconds);
}

void ABasePlatform::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABasePlatform, bIsActivated);
	DOREPLIFETIME(ABasePlatform, StartLocation);
}

void ABasePlatform::SetIsActivated(bool bIsActivated_In)
{
	bIsActivated = bIsActivated_In;
	OnSetIsActivated();
}

void ABasePlatform::InvokePlatform()
{
	SetIsActivated(true);
	PlatformTimelinePlay();
}

void ABasePlatform::ResetPlatform()
{
	SetIsActivated(false);
	PlatformTimeline.Stop();
	PlatformTimelineReverse();
}

void ABasePlatform::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority)
	{
		StartLocation = RootComponent->GetComponentLocation();
	}

	if (TimelineCurve)
	{
		FOnTimelineFloatStatic PlatformMovementTimelineUpdate;
		PlatformMovementTimelineUpdate.BindUObject(this, &ABasePlatform::PlatformTimelineUpdate);
		PlatformTimeline.AddInterpFloat(TimelineCurve, PlatformMovementTimelineUpdate);
	}
}

void ABasePlatform::OnSetIsActivated() const
{
	if (OnPlatformStatusChanged.IsBound())
	{
		OnPlatformStatusChanged.Broadcast(bIsActivated);
	}
}

void ABasePlatform::PlatformTimelineUpdate(float Alpha)
{
	FVector PlatformTargetLocation = FMath::Lerp(StartLocation, StartLocation + EndLocation, Alpha);
	RootComponent->SetWorldLocation(PlatformTargetLocation);
}

void ABasePlatform::OnRep_SetIsActivated(bool bIsActivated_Old)
{
	OnSetIsActivated();
}
