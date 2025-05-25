// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "AI/Characters/Turret.h"

#include "AIController.h"
#include "GenericTeamAgentInterface.h"
#include "Components/AIComponents/TurretAttributesComponent.h"
#include "Components/AIComponents/TurretMuzzleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystem.h"

ATurret::ATurret()
{
	PrimaryActorTick.bCanEverTick = true;

	TurretAttributesComponent = CreateDefaultSubobject<UTurretAttributesComponent>(TEXT("TurretAttributesComponent"));

	USceneComponent* TurretRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TurretRoot"));
	SetRootComponent(TurretRoot);
	BaseMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMeshComponent->SetupAttachment(TurretRoot);
	HeadBaseComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HeadBase"));
	HeadBaseComponent->SetupAttachment(TurretRoot);
	HeadMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	HeadMeshComponent->SetupAttachment(HeadBaseComponent);
	BarrelBaseComponent = CreateDefaultSubobject<USceneComponent>(TEXT("BarrelBase"));
	BarrelBaseComponent->SetupAttachment(HeadBaseComponent);
	BarrelBodyComponent = CreateDefaultSubobject<USceneComponent>(TEXT("BarrelBody"));
	BarrelBodyComponent->SetupAttachment(BarrelBaseComponent);
	BarrelMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrelMesh"));
	BarrelMeshComponent->SetupAttachment(BarrelBodyComponent);
	TurretMuzzleComponent = CreateDefaultSubobject<UTurretMuzzleComponent>(TEXT("TurretMuzzleComponent"));
	TurretMuzzleComponent->SetupAttachment(BarrelBodyComponent);

	bReplicates = true;
	BaseMeshComponent->SetIsReplicated(true);
	HeadBaseComponent->SetIsReplicated(true);
}

void ATurret::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AAIController* AIController = Cast<AAIController>(NewController);
	if (IsValid(AIController))
	{
		FGenericTeamId TeamId = (uint8)Team;
		AIController->SetGenericTeamId(TeamId);
	}
}

void ATurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (CurrentTurretMode)
	{
		default:
		case ETurretMode::Dead:
			break;
		case ETurretMode::Searching:
			SearchEnemy(DeltaTime);
			break;
		case ETurretMode::Tracking:
			TrackEnemy(DeltaTime);
			break;
	}
}

void ATurret::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATurret, CachedTarget);
}

void ATurret::SetCurrentTarget(AActor* NewTarget)
{
	CachedTarget = NewTarget;
	OnTargetSet();
}

void ATurret::SetTurretMode(ETurretMode NewMode)
{
	if (NewMode == CurrentTurretMode)
	{
		return;
	}

	if (CurrentTurretMode != ETurretMode::Dead)
	{
		CurrentTurretMode = NewMode;
	}

	switch (CurrentTurretMode)
	{
		default:
		case ETurretMode::Dead:
		case ETurretMode::Searching:
			{
				CachedTarget.Reset();
				Multicast_StopFire();
				break;
			}
		case ETurretMode::Tracking:
			{
				Multicast_StartFire();
				break;
			}
	}
}

FVector ATurret::GetPawnViewLocation() const
{
	return BarrelBaseComponent->GetComponentLocation();
}

FRotator ATurret::GetViewRotation() const
{
	return BarrelBaseComponent->GetUpVector().ToOrientationRotator();
}

// IGenericTeamAgentInterface
FGenericTeamId ATurret::GetGenericTeamId() const
{
	return FGenericTeamId((uint8)Team);
}

// ~IGenericTeamAgentInterface

void ATurret::BeginPlay()
{
	Super::BeginPlay();

	OnTakeAnyDamage.AddDynamic(TurretAttributesComponent, &UTurretAttributesComponent::OnDamageTaken);
	TurretAttributesComponent->OnDeathEvent.AddUObject(this, &ATurret::OnDeath);

	TurretMeshHeight = BaseHeight + HeadHeight;
}

void ATurret::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	if (const UStaticMesh* BaseMesh = BaseMeshComponent->GetStaticMesh())
	{
		FVector BaseMeshSize = BaseMesh->GetBoundingBox().GetSize();
		if (!FMath::IsNearlyZero(BaseMeshSize.X) && !FMath::IsNearlyZero(BaseMeshSize.Y) && !FMath::IsNearlyZero(BaseMeshSize.Z))
		{
			BaseMeshComponent->SetRelativeScale3D(FVector(BaseDiameter / BaseMeshSize.X, BaseDiameter / BaseMeshSize.Y, BaseHeight / BaseMeshSize.Z));
		}
	}
	
	if (const UStaticMesh* HeadMesh = HeadMeshComponent->GetStaticMesh())
	{
		FVector HeadMeshSize = HeadMesh->GetBoundingBox().GetSize();
		if (!FMath::IsNearlyZero(HeadMeshSize.X) && !FMath::IsNearlyZero(HeadMeshSize.Y) && !FMath::IsNearlyZero(HeadMeshSize.Z))
		{
			HeadMeshComponent->SetRelativeScale3D(FVector(HeadDiameter / HeadMeshSize.X, HeadDiameter / HeadMeshSize.Y, HeadHeight / HeadMeshSize.Z));
		}
	}
	
	if (const UStaticMesh* BarrelMesh = BarrelMeshComponent->GetStaticMesh())
	{
		FVector BarrelMeshSize = BarrelMesh->GetBoundingBox().GetSize();
		if (!FMath::IsNearlyZero(BarrelMeshSize.X) && !FMath::IsNearlyZero(BarrelMeshSize.Y) && !FMath::IsNearlyZero(BarrelMeshSize.Z))
		{
			BarrelMeshComponent->SetRelativeScale3D(FVector(BarrelDiameter / BarrelMeshSize.X, BarrelDiameter / BarrelMeshSize.Y, BarrelLength / BarrelMeshSize.Z));
		}
	}

	BaseMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, BaseHeight / 2));
	HeadBaseComponent->SetRelativeLocation(FVector(0.f, 0.f, BaseHeight + HeadHeight / 2));
	BarrelBaseComponent->SetRelativeLocation(FVector(HeadDiameter / 2, 0.f, 0.f));
	BarrelBaseComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	float OffsetFromHead = -BarrelDiameter;
	BarrelMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, BarrelLength / 2 + OffsetFromHead));
	TurretMuzzleComponent->SetRelativeLocation(FVector(0.f, 0.f, BarrelLength + OffsetFromHead));
	TurretMuzzleComponent->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));

	TurretMeshHeight = BaseHeight + HeadHeight;
}

void ATurret::SearchEnemy(float DeltaTime) const
{
	FRotator NewHeadRotation = HeadBaseComponent->GetRelativeRotation();
	NewHeadRotation.Yaw = FMath::FInterpTo(NewHeadRotation.Yaw, NewHeadRotation.Yaw + 90.f, DeltaTime, HeadSearchRotationSpeed);
	HeadBaseComponent->SetRelativeRotation(NewHeadRotation);
}

void ATurret::TrackEnemy(float DeltaTime)
{
	AActor* Target = CachedTarget.Get();
	if (!IsValid(Target))
	{
		CurrentTurretMode = ETurretMode::Searching;
		return;
	}

	FVector DesiredHeadDirection = Target->GetActorLocation() - GetActorLocation() - HeadBaseComponent->GetRelativeLocation();
	DesiredHeadDirection.Z = 0.f;

	FRotator CurrentHeadRotation = HeadBaseComponent->GetRelativeRotation();
	FRotator NewHeadRotation = FMath::RInterpTo(CurrentHeadRotation, DesiredHeadDirection.ToOrientationRotator(), DeltaTime, HeadTrackRotationSpeed);
	HeadBaseComponent->SetRelativeRotation(NewHeadRotation);

	FVector DesiredBarrelDirection = Target->GetActorLocation() - BarrelBaseComponent->GetComponentLocation();
	float DesiredBarrelPitch = DesiredBarrelDirection.ToOrientationRotator().Pitch;
	DesiredBarrelPitch = FMath::Clamp(DesiredBarrelPitch, BarrelMinPitchAngle, BarrelMaxPitchAngle);

	FRotator CurrentBarrelBaseRotation = BarrelBodyComponent->GetRelativeRotation();
	float NewBarrelBasePitch = FMath::FInterpTo(CurrentBarrelBaseRotation.Pitch, DesiredBarrelPitch, DeltaTime, BarrelPitchRotationSpeed);
	FRotator NewBarrelBaseRotation = FRotator(NewBarrelBasePitch, 0.f, 0.f);
	BarrelBodyComponent->SetRelativeRotation(NewBarrelBaseRotation);
}

void ATurret::Multicast_StartFire_Implementation()
{
	TurretMuzzleComponent->StartFire();
}

void ATurret::Multicast_StopFire_Implementation()
{
	TurretMuzzleComponent->StopFire();
}

void ATurret::OnTargetSet()
{
	ETurretMode NewMode = CachedTarget.IsValid() ? ETurretMode::Tracking : ETurretMode::Searching;
	SetTurretMode(NewMode);
}

void ATurret::OnDeath()
{
	Multicast_OnDeath();
}

void ATurret::Multicast_OnDeath_Implementation()
{
	SetActorEnableCollision(false);
	BaseMeshComponent->SetVisibility(false, true);
	HeadBaseComponent->SetVisibility(false, true);
	SetTurretMode(ETurretMode::Dead);

	if (ExplosionVFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionVFX, GetActorLocation());
	}
}

void ATurret::OnRep_Target()
{
	OnTargetSet();
}
