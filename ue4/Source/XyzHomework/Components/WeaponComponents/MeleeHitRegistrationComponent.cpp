// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/WeaponComponents/MeleeHitRegistrationComponent.h"

#include "DrawDebugHelpers.h"
#include "XyzHomeworkTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/DebugSubsystem.h"

UMeleeHitRegistrationComponent::UMeleeHitRegistrationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UMeleeHitRegistrationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsHitRegistrationEnabled)
	{
		ProcessHitRegistration();
	}
}

void UMeleeHitRegistrationComponent::EnableHitRegistration(bool bIsHitRegistrationEnabled_In)
{
	APawn* Pawn = Cast<APawn>(GetOwner()->GetOwner());
	bool bIsEnabled = bIsHitRegistrationEnabled_In && IsValid(Pawn) && Pawn->GetLocalRole() == ROLE_Authority;
	PreviousComponentLocation = bIsEnabled ? GetComponentLocation() : FVector::ZeroVector;
	bIsHitRegistrationEnabled = bIsEnabled;
	CachedPawn = bIsEnabled ? Pawn : nullptr;
}

void UMeleeHitRegistrationComponent::ProcessHitRegistration()
{
	FVector CurrentComponentLocation = GetComponentLocation();
	FVector TraceVector = CurrentComponentLocation - PreviousComponentLocation;
	FHitResult HitResult;
	const UWorld* World = GetWorld();
	FCollisionQueryParams CollisionParams = FCollisionQueryParams::DefaultQueryParam;
	CollisionParams.AddIgnoredActor(CachedPawn.Get());
	bool bHasHit = World->SweepSingleByChannel(HitResult, PreviousComponentLocation, CurrentComponentLocation, FQuat::Identity, ECC_Melee, FCollisionShape::MakeSphere(SphereRadius), CollisionParams, FCollisionResponseParams::DefaultResponseParam);

#if (UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT) && ENABLE_DRAW_DEBUG
	const UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	if (DebugSubsystem->IsCategoryEnabled(DebugCategoryMeleeWeapon))
	{
		FVector CapsuleCenter = (PreviousComponentLocation + CurrentComponentLocation) / 2;
		FQuat CapsuleRotation = FRotationMatrix::MakeFromZ(TraceVector).ToQuat();

		DrawDebugCapsule(World, CapsuleCenter, TraceVector.Size() / 2, SphereRadius, CapsuleRotation, FColor::Magenta, false, 3.f);
	}
#endif

	if (bHasHit && OnHitRegisteredEvent.IsBound())
	{
		OnHitRegisteredEvent.Broadcast(TraceVector, HitResult);
	}

	PreviousComponentLocation = CurrentComponentLocation;
}
