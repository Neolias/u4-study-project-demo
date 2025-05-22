// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Animations/XyzBaseCharacterAnimInstance.h"

#include "Actors/Equipment/Weapons/RangedWeaponItem.h"
#include "Characters/XyzBaseCharacter.h"
#include "Characters/Controllers/XyzPlayerController.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "Components/MovementComponents/XyzBaseCharMovementComponent.h"

void UXyzBaseCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	
	if (IsValid(TryGetPawnOwner()))
	{
		checkf(TryGetPawnOwner()->IsA<AXyzBaseCharacter>(), TEXT("UXyzBaseCharacterAnimInstance::NativeBeginPlay(): UXyzBaseCharacterAnimInstance can only be used with AXyzBaseCharacter."))
		CachedBaseCharacter = StaticCast<AXyzBaseCharacter*>(TryGetPawnOwner());
	}
}

void UXyzBaseCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	AXyzBaseCharacter* BaseCharacter = CachedBaseCharacter.Get();
	if (!IsValid(BaseCharacter))
	{
		return;
	}

	const UXyzBaseCharMovementComponent* BaseCharMovementComponent = BaseCharacter->GetBaseCharacterMovementComponent();
	bIsFalling = BaseCharMovementComponent->IsFalling();
	bIsCrouching = BaseCharacter->IsCrouching();
	bIsSprinting = BaseCharacter->IsSprinting();
	bIsProne = BaseCharacter->IsProne();
	bIsSwimming = BaseCharMovementComponent->IsSwimming();
	bIsOnLadder = BaseCharMovementComponent->IsOnLadder();
	bIsOnZipline = BaseCharMovementComponent->IsOnZipline();
	bIsWallRunning = BaseCharMovementComponent->IsWallRunning();
	bIsOutOfStamina = BaseCharacter->IsOutOfStamina();
	bIsAiming = BaseCharacter->IsAiming();
	bIsReloading = BaseCharacter->IsReloadingWeapon();

	const UCharacterEquipmentComponent* CharacterEquipmentComponent = BaseCharacter->GetCharacterEquipmentComponent();
	bIsPrimaryItemEquipped = CharacterEquipmentComponent->IsPrimaryItemEquipped();
	CurrentEquipmentItemType = CharacterEquipmentComponent->GetCurrentEquipmentItemType();
	Velocity = BaseCharMovementComponent->Velocity;
	MovementDirection = CalculateDirection(BaseCharMovementComponent->Velocity, BaseCharacter->GetActorRotation());
	MovementSpeed = BaseCharMovementComponent->Velocity.Size();
	CurrentWallRunSide = BaseCharMovementComponent->GetCurrentWallRunSide();
	CharacterPitch = BaseCharacter->GetActorRotation().Pitch;
	CameraPitch = CalculateCameraPitch();
	AimRotation = BaseCharacter->GetAimOffset();
	if (bIsOnLadder)
	{
		LadderSpeedRatio = BaseCharMovementComponent->GetLadderSpeedRatio();
	}

	float PelvisOffset = BaseCharacter->GetIKPelvisOffset();
	LeftFootEffectorLocation = FVector(BaseCharacter->GetIKLeftFootOffset() - PelvisOffset, 0.f, 0.f);
	RightFootEffectorLocation = FVector(BaseCharacter->GetIKRightFootOffset() - PelvisOffset, 0.f, 0.f);
	PelvisEffectorLocation = FVector(0.f, 0.f, PelvisOffset);
	const ARangedWeaponItem* CurrentRangedWeapon = CharacterEquipmentComponent->GetCurrentRangedWeapon();
	if (IsValid(CurrentRangedWeapon))
	{
		FTransform ForeGripTransform = CurrentRangedWeapon->GetForeGripSocketTransform();
		FVector ForeGripLocation = ForeGripTransform.GetLocation() - PelvisOffset * FVector::UpVector;
		ForeGripTransform.SetLocation(ForeGripLocation);
		CurrentRangedWeaponForeGripTransform = ForeGripTransform;
	}
}

float UXyzBaseCharacterAnimInstance::CalculateCameraPitch() const
{
	const AXyzPlayerController* Controller = CachedBaseCharacter->GetController<AXyzPlayerController>();
	if (IsValid(Controller) && !Controller->IgnoresFPCameraPitch())
	{
		return Controller->GetControlRotation().Pitch;
	}
	return 0.f;
}
