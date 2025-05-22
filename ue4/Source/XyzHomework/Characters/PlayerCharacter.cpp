// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/PlayerCharacter.h"

#include "AbilitySystem/XyzAbilitySystemComponent.h"
#include "Actors/Environment/Ladder.h"
#include "Actors/Equipment/Weapons/RangedWeaponItem.h"
#include "Camera/CameraComponent.h"
#include "Components/TimelineComponent.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "Components/MovementComponents/XyzBaseCharMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/StreamingSubsystem/StreamingSubsystemUtils.h"

APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->bUsePawnControlRotation = false;

	Team = ETeam::Player;
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerCharacter, bShouldSkipProneTimeline)
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (ProneCameraTimelineCurve)
	{
		FOnTimelineFloatStatic TimelineUpdate;
		TimelineUpdate.BindUObject(this, &APlayerCharacter::UpdateProneCameraTimeline);
		ProneCameraTimeline.AddInterpFloat(ProneCameraTimelineCurve, TimelineUpdate);
	}

	if (AimingFOVTimelineCurve)
	{
		FOnTimelineFloatStatic TimelineUpdate;
		TimelineUpdate.BindUObject(this, &APlayerCharacter::UpdateAimingFOVTimeline);
		AimingFOVTimeline.AddInterpFloat(AimingFOVTimelineCurve, TimelineUpdate);
	}

	if (WallRunCameraTimelineCurve)
	{
		FOnTimelineFloatStatic TimelineUpdate;
		TimelineUpdate.BindUObject(this, &APlayerCharacter::UpdateWallRunCameraTimeline);
		WallRunCameraTimeline.AddInterpFloat(WallRunCameraTimelineCurve, TimelineUpdate);
	}

	UStreamingSubsystemUtils::CheckCharacterOverlapStreamingSubsystemVolume(this);
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ProneCameraTimeline.TickTimeline(DeltaSeconds);
	AimingFOVTimeline.TickTimeline(DeltaSeconds);
	WallRunCameraTimeline.TickTimeline(DeltaSeconds);
}

#pragma region CAMERA

void APlayerCharacter::Server_SetShouldSkipProneTimeline_Implementation(bool bShouldSkipProneTimeline_In)
{
	bShouldSkipProneTimeline = bShouldSkipProneTimeline_In;
}

void APlayerCharacter::UpdateProneCameraTimeline(float Value)
{
	SpringArmComponent->TargetOffset.Z = CachedSpringArmTargetOffset.Z + NewSpringArmTargetOffsetDelta.Z * Value;
	SpringArmComponent->SocketOffset = CachedSpringArmSocketOffset + NewSpringArmSocketOffsetDelta * Value;
}

void APlayerCharacter::UpdateAimingFOVTimeline(float Value)
{
	if (APlayerCameraManager* CameraManager = CachedCameraManager.Get())
	{
		float TargetFOV = TargetAimingFOV + (DefaultCameraFOV - TargetAimingFOV) * Value;
		CameraManager->SetFOV(TargetFOV);
	}
}

void APlayerCharacter::UpdateWallRunCameraTimeline(float Value)
{
	// Mirroring and restoring back the spring arm offset
	SpringArmComponent->SocketOffset.Y = CachedSpringArmSocketOffset.Y * (2 * Value - 1.f);
}

#pragma endregion

#pragma region MOVEMENT / SWIMMING

void APlayerCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	if (BaseCharacterMovementComponent->IsSwimming())
	{
		if (IsProne())
		{
			bShouldSkipProneTimeline = true;
		}
	}
	else if (BaseCharacterMovementComponent->IsFalling() && PrevMovementMode == MOVE_Walking)
	{
		CachedSpringArmSocketOffset = SpringArmComponent->SocketOffset;
	}

	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
}

void APlayerCharacter::MoveForward(float Value)
{
	Super::MoveForward(Value);
	if ((GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling()) && !FMath::IsNearlyZero(Value))
	{
		FRotator Rotation(0.f, GetControlRotation().Yaw, 0.f);
		AddMovementInput(Rotation.Vector(), Value);
	}
}

void APlayerCharacter::MoveRight(float Value)
{
	Super::MoveRight(Value);
	if ((GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling()) && !FMath::IsNearlyZero(Value))
	{
		FRotator Rotation(0.f, GetControlRotation().Yaw, 0.f);
		FVector RightVector = Rotation.RotateVector(FVector::RightVector);
		AddMovementInput(RightVector, Value);
	}
}

void APlayerCharacter::Turn(float Value)
{
	Super::Turn(Value);
	AddControllerYawInput(Value * GetAimTurnModifier());
}

void APlayerCharacter::LookUp(float Value)
{
	Super::LookUp(Value);
	AddControllerPitchInput(Value * GetAimLookUpModifier());
}

void APlayerCharacter::TurnAtRate(float Value)
{
	Super::Turn(Value);
	AddControllerYawInput(Value * GetAimTurnModifier() * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::LookUpAtRate(float Value)
{
	Super::LookUp(Value);
	AddControllerPitchInput(Value * GetAimLookUpModifier() * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::ClimbLadderUp(float Value)
{
	Super::ClimbLadderUp(Value);
	if (BaseCharacterMovementComponent->IsOnLadder() && !FMath::IsNearlyZero(Value))
	{
		FVector LadderUpVector = BaseCharacterMovementComponent->GetCurrentLadder()->GetActorUpVector();
		AddMovementInput(LadderUpVector, Value);
	}
}

void APlayerCharacter::SwimForward(float Value)
{
	Super::SwimForward(Value);
	if (BaseCharacterMovementComponent->IsSwimming() && !FMath::IsNearlyZero(Value))
	{
		FRotator Rotation;
		if (!IsDiving())
		{
			Rotation = FRotator(0.f, GetControlRotation().Yaw, 0.f);
		}
		else
		{
			Rotation = FRotator(GetControlRotation().Pitch, GetControlRotation().Yaw, 0.f);
		}
		AddMovementInput(Rotation.Vector(), Value);
	}
}

void APlayerCharacter::SwimRight(float Value)
{
	Super::SwimRight(Value);
	if (BaseCharacterMovementComponent->IsSwimming() && !FMath::IsNearlyZero(Value))
	{
		FRotator Rotation(0.f, GetControlRotation().Yaw, 0.f);
		FVector RightVector = Rotation.RotateVector(FVector::RightVector);
		AddMovementInput(RightVector, Value);
	}
}

void APlayerCharacter::SwimUp(float Value)
{
	Super::SwimUp(Value);
	if (IsDiving() && !FMath::IsNearlyZero(Value))
	{
		FVector ForwardVector = GetActorForwardVector().GetSafeNormal2D() * 0.1f; // Ensuring that the character is facing forward
		AddMovementInput(FVector(ForwardVector.X, ForwardVector.Y, Value));
	}
}
#pragma endregion

#pragma region SLIDING / WALL RUNNING

void APlayerCharacter::OnStartSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartSlide(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SpringArmComponent->TargetOffset += FVector(0.f, 0.f, HalfHeightAdjust);
}

void APlayerCharacter::OnStopSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStopSlide(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SpringArmComponent->TargetOffset -= FVector(0.f, 0.f, HalfHeightAdjust);
}

void APlayerCharacter::OnWallRunStart()
{
	Super::OnWallRunStart();

	if (BaseCharacterMovementComponent->GetCurrentWallRunSide() == EWallRunSide::Right)
	{
		if (WallRunCameraTimelineCurve)
		{
			WallRunCameraTimeline.Play();
		}
		else
		{
			SpringArmComponent->SocketOffset.Y = 0.f;
		}
	}
}

void APlayerCharacter::OnWallRunEnd()
{
	Super::OnWallRunEnd();

	if (WallRunCameraTimelineCurve)
	{
		WallRunCameraTimeline.Reverse();
	}
	else
	{
		SpringArmComponent->SocketOffset.Y = CachedSpringArmSocketOffset.Y;
	}
}
#pragma endregion

#pragma region CROUCHING / PRONE

bool APlayerCharacter::CanChangeCrouchState() const
{
	return !ProneCameraTimeline.IsPlaying() && Super::CanChangeCrouchState();
}

void APlayerCharacter::UnCrouch(bool bClientSimulation)
{
	if (IsProne() && IsLocallyControlled())
	{
		bShouldSkipProneTimeline = true;
		Server_SetShouldSkipProneTimeline(true);
	}

	Super::UnCrouch(bClientSimulation);
}

void APlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SpringArmComponent->TargetOffset += FVector(0.f, 0.f, HalfHeightAdjust);
}

void APlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SpringArmComponent->TargetOffset -= FVector(0.f, 0.f, HalfHeightAdjust);
}

bool APlayerCharacter::CanChangeProneState() const
{
	return !ProneCameraTimeline.IsPlaying() && Super::CanChangeProneState();
}

void APlayerCharacter::OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartProne(HalfHeightAdjust, ScaledHalfHeightAdjust);

	if (HalfHeightAdjust == 0.f && ScaledHalfHeightAdjust == 0.f)
	{
		return;
	}

	CachedSpringArmTargetOffset = SpringArmComponent->TargetOffset;
	CachedSpringArmSocketOffset = SpringArmComponent->SocketOffset;
	NewSpringArmTargetOffsetDelta = FVector(0.f, 0.f, HalfHeightAdjust + ProneCameraHeightOffset);
	NewSpringArmSocketOffsetDelta = FVector(ProneCameraProximityOffset, ProneCameraRightOffset, 0.f);

	if (ProneCameraTimelineCurve)
	{
		ProneCameraTimeline.PlayFromStart();
	}
	else
	{
		SpringArmComponent->TargetOffset += NewSpringArmTargetOffsetDelta;
		SpringArmComponent->SocketOffset += NewSpringArmSocketOffsetDelta;
	}
}

void APlayerCharacter::OnStopProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStopProne(HalfHeightAdjust, ScaledHalfHeightAdjust);

	if (HalfHeightAdjust == 0.f && ScaledHalfHeightAdjust == 0.f)
	{
		return;
	}

	if (!bShouldSkipProneTimeline && ProneCameraTimelineCurve)
	{
		NewSpringArmTargetOffsetDelta = FVector(0.f, 0.f, HalfHeightAdjust + ProneCameraHeightOffset);
		ProneCameraTimeline.ReverseFromEnd();
	}
	else
	{
		SpringArmComponent->TargetOffset = CachedSpringArmTargetOffset;
		SpringArmComponent->SocketOffset = CachedSpringArmSocketOffset;
	}

	bShouldSkipProneTimeline = false;
}
#pragma endregion

#pragma region AIMING

float APlayerCharacter::GetAimTurnModifier() const
{
	if (IsAiming())
	{
		return AimTurnModifier;
	}

	return 1.f;
}

float APlayerCharacter::GetAimLookUpModifier() const
{
	if (IsAiming())
	{
		return AimLookUpModifier;
	}

	return 1.f;
}

void APlayerCharacter::OnStartAiming_Implementation()
{
	Super::OnStartAiming_Implementation();

	const ARangedWeaponItem* RangedWeapon = CharacterEquipmentComponent->GetCurrentRangedWeapon();
	if (IsValid(RangedWeapon))
	{
		const APlayerController* PlayerController = GetController<APlayerController>();
		if (IsValid(PlayerController) && PlayerController->PlayerCameraManager)
		{
			CachedCameraManager = PlayerController->PlayerCameraManager;
			DefaultCameraFOV = PlayerController->PlayerCameraManager->DefaultFOV;
			TargetAimingFOV = RangedWeapon->GetAimingFOV();

			if (AimingFOVTimelineCurve)
			{
				AimingFOVTimeline.Play();
			}
			else
			{
				PlayerController->PlayerCameraManager->SetFOV(TargetAimingFOV);
			}
		}
	}
}

void APlayerCharacter::OnStopAiming_Implementation()
{
	Super::OnStopAiming_Implementation();

	if (AimingFOVTimelineCurve)
	{
		AimingFOVTimeline.Reverse();
	}
	else
	{
		const APlayerController* PlayerController = GetController<APlayerController>();
		if (IsValid(PlayerController) && PlayerController->PlayerCameraManager)
		{
			PlayerController->PlayerCameraManager->UnlockFOV();
		}
	}
}
#pragma endregion
