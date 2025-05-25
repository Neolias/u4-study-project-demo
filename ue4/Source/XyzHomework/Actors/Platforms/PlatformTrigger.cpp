// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "Actors/Platforms/PlatformTrigger.h"

#include "BasePlatform.h"
#include "Characters/XyzBaseCharacter.h"
#include "Net/UnrealNetwork.h"

APlatformTrigger::APlatformTrigger()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	NetUpdateFrequency = 2.f;
	MinNetUpdateFrequency = 2.f;

	USceneComponent* TriggerRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = TriggerRoot;

	TriggerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TriggerMesh"));
	TriggerMesh->SetupAttachment(TriggerRoot);

	TriggerCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerCollision"));
	TriggerCollision->SetupAttachment(TriggerRoot);
}

void APlatformTrigger::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APlatformTrigger, bIsTriggered);
}

void APlatformTrigger::SetIsTriggered(bool bIsTriggered_In)
{
	bIsTriggered = bIsTriggered_In;
	OnSetIsTriggered();
}

void APlatformTrigger::BeginPlay()
{
	Super::BeginPlay();

	TriggerCollision->OnComponentBeginOverlap.AddDynamic(this, &APlatformTrigger::RegisterPawns);
	TriggerCollision->OnComponentEndOverlap.AddDynamic(this, &APlatformTrigger::UnRegisterPawns);
}

void APlatformTrigger::OnSetIsTriggered() const
{
	if (OnTriggerStatusChanged.IsBound())
	{
		OnTriggerStatusChanged.Broadcast(bIsTriggered);
	}

	for (ABasePlatform* Platform : ControlledPlatforms)
	{
		if (!IsValid(Platform))
		{
			continue;
		}

		if (bIsTriggered)
		{
			Platform->InvokePlatform();
		}
		else
		{
			Platform->ResetPlatform();
		}
	}
}

void APlatformTrigger::RegisterPawns(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(OtherActor);
	if (!IsValid(BaseCharacter))
	{
		return;
	}

	OverlappedPawns.AddUnique(BaseCharacter);
	if (!bIsTriggered && OverlappedPawns.Num() > 0)
	{
		if (GetLocalRole() == ROLE_Authority)
		{
			SetIsTriggered(true);
		}
	}
}

void APlatformTrigger::UnRegisterPawns(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AXyzBaseCharacter* BaseCharacter = Cast<AXyzBaseCharacter>(OtherActor);
	if (!IsValid(BaseCharacter))
	{
		return;
	}

	OverlappedPawns.RemoveSingleSwap(BaseCharacter);
	if (bIsTriggered && OverlappedPawns.Num() < 1)
	{
		if (GetLocalRole() == ROLE_Authority)
		{
			SetIsTriggered(false);
		}
	}
}

void APlatformTrigger::OnRep_SetIsTriggered(bool bIsTriggered_Old)
{
	OnSetIsTriggered();
}
