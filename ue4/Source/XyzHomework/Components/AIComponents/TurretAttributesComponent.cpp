// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/AIComponents/TurretAttributesComponent.h"

#include "DrawDebugHelpers.h"
#include "XyzHomeworkTypes.h"
#include "AI/Characters/Turret.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/DebugSubsystem.h"

UTurretAttributesComponent::UTurretAttributesComponent()
{
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	PrimaryComponentTick.bCanEverTick = true;
#endif
}

void UTurretAttributesComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(GetOwner()->IsA<ATurret>(), TEXT("UTurretAttributesComponent::BeginPlay(): UTurretAttributesComponent can only be used with ATurret."))
	TurretOwner = StaticCast<ATurret*>(GetOwner());

	CurrentHealth = MaxHealth;
}

void UTurretAttributesComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                               FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	DrawDebugAttributes();
#endif
}

bool UTurretAttributesComponent::IsAlive() const
{
	return !bIsDeathTriggered && CurrentHealth > 0.f;
}

void UTurretAttributesComponent::OnDamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (IsAlive())
	{
		CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);
		TryTriggerDeath();
	}
}

void UTurretAttributesComponent::TryTriggerDeath()
{
	if (!IsAlive())
	{
		bIsDeathTriggered = true;

		if (OnDeathEvent.IsBound())
		{
			OnDeathEvent.Broadcast();
		}
	}
}

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
void UTurretAttributesComponent::DrawDebugAttributes() const
{
	const UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	if (!DebugSubsystem->IsCategoryEnabled(DebugCategoryAIAttributes))
	{
		return;
	}

	const APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	FVector CameraLocation = CameraManager->GetCameraLocation();
	FVector TurretLocation = TurretOwner->GetActorLocation();
	float DistanceFromCamera = FVector::Dist(TurretLocation, CameraLocation);
	if (DistanceFromCamera > AttributesVisibilityRange)
	{
		return;
	}
	float AttributeFontScale = FMath::Clamp(DefaultPlayerDistanceFromCamera / DistanceFromCamera, 0.5f, 1.f);
	float ScaledAttributeFontSize = AttributesFontSize * AttributeFontScale;
	FRotator CameraRotation = CameraManager->GetCameraRotation();
	CameraRotation.Pitch = 0.f;
	FVector HealthBarLocation = TurretLocation + TurretOwner->GetTurretMeshHeight() * FVector::UpVector + CameraRotation.RotateVector(HealthBarOffset);
	DrawDebugString(GetWorld(), HealthBarLocation, FString::Printf(TEXT("%.2f"), CurrentHealth), nullptr, FColor::Red, 0.f, true, ScaledAttributeFontSize);
}
#endif
