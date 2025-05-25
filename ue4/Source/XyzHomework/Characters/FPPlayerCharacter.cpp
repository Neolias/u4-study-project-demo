// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "Characters/FPPlayerCharacter.h"

#include "Actors/Environment/Ladder.h"
#include "Actors/Environment/Zipline.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/MovementComponents/XyzBaseCharMovementComponent.h"
#include "Controllers/XyzPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetSystemLibrary.h"

AFPPlayerCharacter::AFPPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FPMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPSkeletalMesh"));
	FPMeshComponent->SetupAttachment(GetCapsuleComponent());
	FPMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -86.f));
	FPMeshComponent->CastShadow = false;
	FPMeshComponent->bCastDynamicShadow = false;
	FPMeshComponent->SetOnlyOwnerSee(true);

	FPCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FPCamera"));
	FPCameraComponent->SetupAttachment(FPMeshComponent, CameraSocket);
	FPCameraComponent->bUsePawnControlRotation = true;

	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->bCastHiddenShadow = true;

	CameraComponent->bAutoActivate = false;
	SpringArmComponent->bAutoActivate = false;
	SpringArmComponent->bUsePawnControlRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = true;

	bIsFirstPerson = true;
}

void AFPPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (WallRunCameraTiltCurve)
	{
		FOnTimelineFloatStatic TimelineCallBack;
		TimelineCallBack.BindUObject(this, &AFPPlayerCharacter::UpdateWallRunCameraTilt);
		WallRunCameraTiltTimeline.AddInterpFloat(WallRunCameraTiltCurve, TimelineCallBack);
	}
}

void AFPPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateCameraAlignment(DeltaSeconds);

	WallRunCameraTiltTimeline.TickTimeline(DeltaSeconds);
}

FRotator AFPPlayerCharacter::GetViewRotation() const
{
	FRotator Result = Super::GetViewRotation();

	// Makes camera alignment smoother
	if (IsAligningFPCameraToSocketRotation())
	{
		FRotator SocketRotation = FPMeshComponent->GetSocketRotation(CameraSocket);
		if (IsOutOfStamina())
		{
			SocketRotation.Yaw = Result.Yaw;
		}
		Result = SocketRotation;
	}

	return Result;
}

bool AFPPlayerCharacter::IsFPMontagePlaying() const
{
	return FPMeshComponent->GetAnimInstance() && FPMeshComponent->GetAnimInstance()->IsAnyMontagePlaying();
}

#pragma region CAMERA

bool AFPPlayerCharacter::IsAligningFPCameraToSocketRotation() const
{
	return IsFPMontagePlaying() && (BaseCharacterMovementComponent->IsMantling() || BaseCharacterMovementComponent->IsOnTopOfCurrentLadder()) || IsOutOfStamina() || bIsHardLanding;
}

void AFPPlayerCharacter::StartFPCameraAlignment()
{
	if (IsValid(GetController()))
	{
		GetController()->SetIgnoreLookInput(true);
		GetController()->SetIgnoreMoveInput(true);
	}
	bIsForcedToAlignFPCamera = true;
}

void AFPPlayerCharacter::EndFPCameraAlignment()
{
	bIsForcedToAlignFPCamera = false;
	if (IsValid(GetController()))
	{
		GetController()->ResetIgnoreLookInput();
		GetController()->ResetIgnoreMoveInput();
	}
}

void AFPPlayerCharacter::UpdateCameraAlignment(float DeltaSeconds)
{
	if (!IsValid(GetController()))
	{
		return;
	}

	if (IsAligningFPCameraToSocketRotation())
	{
		FRotator TargetControlRotation = GetController()->GetControlRotation();
		FRotator SocketRotation = FPMeshComponent->GetSocketRotation(CameraSocket);
		if (IsOutOfStamina())
		{
			SocketRotation.Yaw = TargetControlRotation.Yaw;
		}
		TargetControlRotation = FMath::RInterpTo(TargetControlRotation, SocketRotation, DeltaSeconds, AnimMontageCameraBlendSpeed);
		GetController()->SetControlRotation(TargetControlRotation);
	}

	if (BaseCharacterMovementComponent->IsWallRunning())
	{
		if (BaseCharacterMovementComponent->IsWallRunning())
		{
			FRotator CurrentWallRunDirection = BaseCharacterMovementComponent->GetCurrentWallRunDirection().ToOrientationRotator();
			FRotator TargetControlRotation = GetController()->GetControlRotation();
			TargetControlRotation.Pitch = CurrentWallRunDirection.Pitch;
			TargetControlRotation.Yaw = CurrentWallRunDirection.Yaw;
			TargetControlRotation = FMath::RInterpTo(GetController()->GetControlRotation(), TargetControlRotation, DeltaSeconds, CameraAlignmentBlendSpeed);
			GetController()->SetControlRotation(TargetControlRotation);
		}
	}

	if (bIsForcedToAlignFPCamera)
	{
		FRotator TargetControlRotation = GetController()->GetControlRotation();
		if (!TargetControlRotation.Equals(ForcedTargetControlRotation, 0.1f))
		{
			TargetControlRotation = FMath::RInterpTo(TargetControlRotation, ForcedTargetControlRotation, DeltaSeconds, CameraAlignmentBlendSpeed);
			GetController()->SetControlRotation(TargetControlRotation);
		}
		else
		{
			EndFPCameraAlignment();

			if (bWantsToEndMantle)
			{
				OnStopMantle();
			}
			else if (bWantsToAttachToLadder)
			{
				OnAttachedToLadder();
			}
			else if (bWantsToEndOutOfStamina)
			{
				bUseControllerRotationYaw = true;
				bWantsToEndOutOfStamina = false;
			}
			else if (bWantsToEndHardLand)
			{
				bUseControllerRotationYaw = true;
				bWantsToEndHardLand = false;
			}
		}
	}
}

void AFPPlayerCharacter::UpdateWallRunCameraTilt(float Value) const
{
	if (IsValid(GetController()))
	{
		FRotator TargetControlRotation = GetController()->GetControlRotation();
		TargetControlRotation.Roll = BaseCharacterMovementComponent->GetCurrentWallRunSide() == EWallRunSide::Left ? Value : -Value;
		GetController()->SetControlRotation(TargetControlRotation);
	}
}

void AFPPlayerCharacter::StartWallRunCameraTilt()
{
	WallRunCameraTiltTimeline.Play();
}

void AFPPlayerCharacter::EndWallRunCameraTilt()
{
	WallRunCameraTiltTimeline.Reverse();
}
#pragma endregion

#pragma region MOVEMENT

void AFPPlayerCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (BaseCharacterMovementComponent->IsOnLadder())
	{
		bWantsToAttachToLadder = true;
		const ALadder* CurrentLadder = BaseCharacterMovementComponent->GetCurrentLadder();
		if (IsValid(CurrentLadder))
		{
			FRotator ForcedRotation = BaseCharacterMovementComponent->GetCurrentLadder()->GetActorForwardVector().ToOrientationRotator();
			ForcedRotation.Yaw += 180.f;
			ForcedTargetControlRotation = ForcedRotation;
			StartFPCameraAlignment();
		}
	}
	else if (BaseCharacterMovementComponent->IsOnZipline())
	{
		OnAttachedToZipline();
	}
	else if (BaseCharacterMovementComponent->IsSwimming())
	{
		AXyzPlayerController* PlayerController = CachedPlayerController.Get();
		if (IsValid(PlayerController))
		{
			PlayerController->SetIgnoreFPCameraPitch(true);
		}
	}
	else if (BaseCharacterMovementComponent->IsWallRunning())
	{
		AXyzPlayerController* PlayerController = CachedPlayerController.Get();
		if (IsValid(PlayerController))
		{
			PlayerController->SetIgnoreFPCameraPitch(true);
			PlayerController->SetIgnoreLookInput(true);
			PlayerController->SetIgnoreMoveInput(true);
		}
		StartWallRunCameraTilt();
	}
	else if (PrevMovementMode == MOVE_Swimming)
	{
		AXyzPlayerController* PlayerController = CachedPlayerController.Get();
		if (IsValid(PlayerController))
		{
			PlayerController->SetIgnoreFPCameraPitch(false);
		}
	}
	else if (PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Ladder)
	{
		OnDetachedFromLadder();
	}
	else if (PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Zipline)
	{
		OnDetachedFromZipline();
	}
	else if (PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Mantling)
	{
		bWantsToEndMantle = true;
	}
	else if (PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_WallRun)
	{
		AXyzPlayerController* PlayerController = CachedPlayerController.Get();
		if (IsValid(PlayerController))
		{
			PlayerController->SetIgnoreFPCameraPitch(false);
			PlayerController->ResetIgnoreLookInput();
			PlayerController->ResetIgnoreMoveInput();
		}
		EndWallRunCameraTilt();
	}
}

void AFPPlayerCharacter::OnStartSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartSlide(HalfHeightAdjust, ScaledHalfHeightAdjust);

	if (!SlideFPAnimMontage)
	{
		return;
	}

	FVector& RelativeMeshLocation = FPMeshComponent->GetRelativeLocation_DirectMutable();
	RelativeMeshLocation.Z += HalfHeightAdjust;
	bUseControllerRotationYaw = false;
	if (UAnimInstance* FPAnimInstance = FPMeshComponent->GetAnimInstance())
	{
		bUseControllerRotationYaw = false;
		AXyzPlayerController* PlayerController = CachedPlayerController.Get();
		if (IsValid(PlayerController))
		{
			PlayerController->SetIgnoreFPCameraPitch(true);
			PlayerController->SetIgnoreLookInput(true);
			PlayerController->SetIgnoreMoveInput(true);
		}
		ForcedTargetControlRotation = GetActorForwardVector().ToOrientationRotator();
		float Duration = FPAnimInstance->Montage_Play(SlideFPAnimMontage, 1.f, EMontagePlayReturnType::Duration);
		// Uncomment to align camera after sliding
		//GetWorldTimerManager().SetTimer(FPSlideTimer, this, &AFPPlayerCharacter::StartFPCameraAlignment, Duration, false);
	}
}

void AFPPlayerCharacter::OnStopSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStopSlide(HalfHeightAdjust, ScaledHalfHeightAdjust);

	bUseControllerRotationYaw = true;
	AXyzPlayerController* PlayerController = CachedPlayerController.Get();
	if (IsValid(PlayerController))
	{
		PlayerController->SetIgnoreFPCameraPitch(false);
		PlayerController->ResetIgnoreLookInput();
		PlayerController->ResetIgnoreMoveInput();
	}
	FVector& RelativeMeshLocation = FPMeshComponent->GetRelativeLocation_DirectMutable();
	RelativeMeshLocation.Z -= HalfHeightAdjust;
}

void AFPPlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	AXyzPlayerController* PlayerController = CachedPlayerController.Get();
	if (IsValid(PlayerController))
	{
		PlayerController->SetIgnoreFPCameraPitch(true);
	}
	const AFPPlayerCharacter* DefaultCharacter = GetDefault<AFPPlayerCharacter>(GetClass());
	FVector& RelativeMeshLocation = FPMeshComponent->GetRelativeLocation_DirectMutable();
	RelativeMeshLocation.Z = DefaultCharacter->FPMeshComponent->GetRelativeLocation().Z + HalfHeightAdjust;
}

void AFPPlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	AXyzPlayerController* PlayerController = CachedPlayerController.Get();
	if (IsValid(PlayerController))
	{
		PlayerController->SetIgnoreFPCameraPitch(false);
	}
	const AFPPlayerCharacter* DefaultCharacter = GetDefault<AFPPlayerCharacter>(GetClass());
	FVector& RelativeMeshLocation = FPMeshComponent->GetRelativeLocation_DirectMutable();
	RelativeMeshLocation.Z = DefaultCharacter->FPMeshComponent->GetRelativeLocation().Z;
}

void AFPPlayerCharacter::OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartProne(HalfHeightAdjust, ScaledHalfHeightAdjust);

	AXyzPlayerController* PlayerController = CachedPlayerController.Get();
	if (IsValid(PlayerController))
	{
		PlayerController->SetIgnoreFPCameraPitch(true);
	}
	FVector& RelativeMeshLocation = FPMeshComponent->GetRelativeLocation_DirectMutable();
	RelativeMeshLocation.Z += HalfHeightAdjust;
}

void AFPPlayerCharacter::OnStopProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStopProne(HalfHeightAdjust, ScaledHalfHeightAdjust);

	AXyzPlayerController* PlayerController = CachedPlayerController.Get();
	if (IsValid(PlayerController))
	{
		PlayerController->SetIgnoreFPCameraPitch(false);
	}
	FVector& RelativeMeshLocation = FPMeshComponent->GetRelativeLocation_DirectMutable();
	RelativeMeshLocation.Z -= HalfHeightAdjust;
}

void AFPPlayerCharacter::OnMantle(const FMantlingSettings& MantlingSettings, const FMantlingMovementParameters& MantlingParameters)
{
	Super::OnMantle(MantlingSettings, MantlingParameters);

	if (FPMeshComponent->GetAnimInstance() && MantlingSettings.FPMantlingMontage)
	{
		bUseControllerRotationYaw = false;
		AXyzPlayerController* PlayerController = CachedPlayerController.Get();
		if (IsValid(PlayerController))
		{
			PlayerController->SetIgnoreFPCameraPitch(true);
			PlayerController->SetIgnoreLookInput(true);
			PlayerController->SetIgnoreMoveInput(true);
		}
		ForcedTargetControlRotation = MantlingParameters.TargetRotation;
		float MantlingDuration = FPMeshComponent->GetAnimInstance()->Montage_Play(MantlingSettings.FPMantlingMontage, 1.f, EMontagePlayReturnType::Duration, MantlingParameters.StartTime);
		GetWorldTimerManager().SetTimer(FPMantlingTimerHandle, this, &AFPPlayerCharacter::StartFPCameraAlignment, MantlingDuration, false);
	}
}

void AFPPlayerCharacter::StartHardLand()
{
	Super::StartHardLand();

	if (!IsValid(HardLandFPAnimMontage))
	{
		return;
	}

	bUseControllerRotationYaw = false;
	if (UAnimInstance* FPAnimInstance = FPMeshComponent->GetAnimInstance())
	{
		AXyzPlayerController* PlayerController = CachedPlayerController.Get();
		if (IsValid(PlayerController))
		{
			PlayerController->SetIgnoreFPCameraPitch(true);
		}
		FPAnimInstance->Montage_Play(HardLandFPAnimMontage);
	}
}

void AFPPlayerCharacter::StopHardLand()
{
	Super::StopHardLand();
	AXyzPlayerController* PlayerController = CachedPlayerController.Get();
	if (IsValid(PlayerController))
	{
		PlayerController->SetIgnoreFPCameraPitch(false);
	}
	ForcedTargetControlRotation = GetActorForwardVector().ToOrientationRotator();
	bWantsToEndHardLand = true;
	StartFPCameraAlignment();
}

void AFPPlayerCharacter::OnStopMantle()
{
	AXyzPlayerController* PlayerController = CachedPlayerController.Get();
	if (IsValid(PlayerController))
	{
		PlayerController->SetIgnoreFPCameraPitch(false);
		PlayerController->ResetIgnoreLookInput();
		PlayerController->ResetIgnoreMoveInput();
	}
	bUseControllerRotationYaw = true;
	bWantsToEndMantle = false;
}

#pragma endregion

#pragma region ENVIRONMENT ACTORS

void AFPPlayerCharacter::OnAttachedToLadderFromTop(ALadder* Ladder)
{
	Super::OnAttachedToLadderFromTop(Ladder);
	
	if (UAnimInstance* FPAnimInstance = FPMeshComponent->GetAnimInstance())
	{
		float Duration = FPAnimInstance->Montage_Play(Ladder->GetAttachFromTopFPAnimMontage(), 1.f, EMontagePlayReturnType::Duration);
		GetWorldTimerManager().SetTimer(FPLadderTimerHandle, this, &AFPPlayerCharacter::StartFPCameraAlignment, Duration, false);
	}
}

void AFPPlayerCharacter::OnAttachedToLadder()
{
	bUseControllerRotationYaw = false;
	AXyzPlayerController* PlayerController = CachedPlayerController.Get();
	if (IsValid(PlayerController))
	{
		PlayerController->SetIgnoreFPCameraPitch(true);
		FRotator CurrentControlRotation = PlayerController->GetControlRotation();
		APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;
		CameraManager->ViewPitchMin = LadderCameraMinPitch;
		CameraManager->ViewPitchMax = LadderCameraMaxPitch;
		CameraManager->ViewYawMin = CurrentControlRotation.Yaw + LadderCameraMinYaw;
		CameraManager->ViewYawMax = CurrentControlRotation.Yaw + LadderCameraMaxYaw;
	}
	bWantsToAttachToLadder = false;
}

void AFPPlayerCharacter::OnDetachedFromLadder()
{
	bUseControllerRotationYaw = true;
	AXyzPlayerController* PlayerController = CachedPlayerController.Get();
	if (IsValid(PlayerController))
	{
		PlayerController->SetIgnoreFPCameraPitch(true);
		APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;
		const APlayerCameraManager* DefaultCameraManager = CameraManager->GetClass()->GetDefaultObject<APlayerCameraManager>();

		CameraManager->ViewPitchMin = DefaultCameraManager->ViewPitchMin;
		CameraManager->ViewPitchMax = DefaultCameraManager->ViewPitchMax;
		CameraManager->ViewYawMin = DefaultCameraManager->ViewYawMin;
		CameraManager->ViewYawMax = DefaultCameraManager->ViewYawMax;
	}
}

void AFPPlayerCharacter::OnAttachedToZipline()
{
	bUseControllerRotationYaw = false;
	FRotator ZiplineRotation = BaseCharacterMovementComponent->GetCurrentZipline()->GetZiplineSpanVector().ToOrientationRotator();
	AXyzPlayerController* PlayerController = CachedPlayerController.Get();
	if (IsValid(PlayerController))
	{
		PlayerController->SetIgnoreFPCameraPitch(true);
		APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;
		CameraManager->ViewPitchMin = ZiplineCameraMinPitch;
		CameraManager->ViewPitchMax = ZiplineCameraMaxPitch;
		CameraManager->ViewYawMin = ZiplineRotation.Yaw + ZiplineCameraMinYaw;
		CameraManager->ViewYawMax = ZiplineRotation.Yaw + ZiplineCameraMaxYaw;
	}
	ForcedTargetControlRotation = ZiplineRotation;
	StartFPCameraAlignment();
}

void AFPPlayerCharacter::OnDetachedFromZipline()
{
	bUseControllerRotationYaw = true;
	AXyzPlayerController* PlayerController = CachedPlayerController.Get();
	if (IsValid(PlayerController))
	{
		PlayerController->SetIgnoreFPCameraPitch(false);
		APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;
		const APlayerCameraManager* DefaultCameraManager = CameraManager->GetClass()->GetDefaultObject<APlayerCameraManager>();

		CameraManager->ViewPitchMin = DefaultCameraManager->ViewPitchMin;
		CameraManager->ViewPitchMax = DefaultCameraManager->ViewPitchMax;
		CameraManager->ViewYawMin = DefaultCameraManager->ViewYawMin;
		CameraManager->ViewYawMax = DefaultCameraManager->ViewYawMax;
	}
}
#pragma endregion

#pragma region ATTRIBUTES

void AFPPlayerCharacter::OnDeath(bool bShouldPlayAnimMontage)
{
	Super::OnDeath(bShouldPlayAnimMontage);

	FPCameraComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	GetMesh()->SetOwnerNoSee(false);
	FPMeshComponent->SetOwnerNoSee(true);
}

void AFPPlayerCharacter::StartOutOfStaminaInternal()
{
	Super::StartOutOfStaminaInternal();

	AXyzPlayerController* PlayerController = CachedPlayerController.Get();
	if (IsValid(PlayerController))
	{
		PlayerController->SetIgnoreFPCameraPitch(true);
	}
}

void AFPPlayerCharacter::StopOutOfStaminaInternal()
{
	Super::StopOutOfStaminaInternal();

	bUseControllerRotationYaw = false;
	AXyzPlayerController* PlayerController = CachedPlayerController.Get();
	if (IsValid(PlayerController))
	{
		PlayerController->SetIgnoreFPCameraPitch(false);
	}
	ForcedTargetControlRotation = GetActorForwardVector().ToOrientationRotator();
	bWantsToEndOutOfStamina = true;
	StartFPCameraAlignment();
}
#pragma endregion
