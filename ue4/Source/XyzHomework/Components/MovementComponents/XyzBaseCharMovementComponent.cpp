// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#include "Components/MovementComponents/XyzBaseCharMovementComponent.h"

#include "AbilitySystemComponent.h"
#include "XyzHomeworkTypes.h"
#include "Actors/Environment/Ladder.h"
#include "Actors/Environment/Zipline.h"
#include "Actors/Equipment/Weapons/RangedWeaponItem.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "Curves/CurveVector.h"
#include "GameFramework/Character.h"
#include "GameFramework/PhysicsVolume.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ProfilingDebugging/CookStats.h"

#pragma region REPLICATION

void FSavedMove_XyzCharacter::Clear()
{
	Super::Clear();

	SavedCustomCompressedFlags = 0;
	SavedWallRunElapsedTime = 0.f;
}

void FSavedMove_XyzCharacter::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	checkf(Character->IsA<AXyzBaseCharacter>(), TEXT("FSavedMove_XyzCharacter::SetMoveFor: FSavedMove_XyzCharacter can only be used with AXyzBaseCharacter."))
	const AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(Character);
	const UXyzBaseCharMovementComponent* BaseCharMovementComponent = BaseCharacter->GetBaseCharacterMovementComponent();

	SavedCustomCompressedFlags = 0;
	if (BaseCharMovementComponent->GetMovementFlag((uint32)EGameplayAbility::Crouch))
	{
		SavedCustomCompressedFlags |= FLAG_IsCrouching;
	}
	if (BaseCharMovementComponent->GetMovementFlag((uint32)EGameplayAbility::Prone))
	{
		SavedCustomCompressedFlags |= FLAG_IsProne;
	}
	if (BaseCharMovementComponent->GetMovementFlag((uint32)EGameplayAbility::Sprint))
	{
		SavedCustomCompressedFlags |= FLAG_IsSprinting;
	}
	if (BaseCharMovementComponent->GetMovementFlag((uint32)EGameplayAbility::Slide))
	{
		SavedCustomCompressedFlags |= FLAG_IsSliding;
	}
	if (BaseCharMovementComponent->GetMovementFlag((uint32)EGameplayAbility::OutOfStamina))
	{
		SavedCustomCompressedFlags |= FLAG_IsOutOfStamina;
	}
	if (BaseCharMovementComponent->GetMovementFlag((uint32)EGameplayAbility::Dive))
	{
		SavedCustomCompressedFlags |= FLAG_IsDiving;
	}
	if (BaseCharMovementComponent->GetMovementFlag((uint32)EGameplayAbility::AimWeapon))
	{
		SavedCustomCompressedFlags |= FLAG_IsAiming;
	}
	if (BaseCharMovementComponent->GetMovementFlag((uint32)EGameplayAbility::ReloadWeapon))
	{
		SavedCustomCompressedFlags |= FLAG_IsReloading;
	}
	if (BaseCharMovementComponent->GetMovementFlag((uint32)EGameplayAbility::ThrowItem))
	{
		SavedCustomCompressedFlags |= FLAG_IsThrowing;
	}

	SavedWallRunElapsedTime = BaseCharMovementComponent->WallRunElapsedTime;
}

bool FSavedMove_XyzCharacter::CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_XyzCharacter* NewMove = StaticCast<FSavedMove_XyzCharacter*>(NewMovePtr.Get());
	if (NewMove->SavedCustomCompressedFlags != SavedCustomCompressedFlags || NewMove->SavedWallRunElapsedTime != SavedWallRunElapsedTime)
	{
		return false;
	}

	return Super::CanCombineWith(NewMovePtr, InCharacter, MaxDelta);
}

void FSavedMove_XyzCharacter::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	checkf(Character->IsA<AXyzBaseCharacter>(), TEXT("FSavedMove_XyzCharacter::PrepMoveFor: FSavedMove_XyzCharacter can only be used with AXyzBaseCharacter."))
	const AXyzBaseCharacter* BaseCharacter = StaticCast<AXyzBaseCharacter*>(Character);
	UXyzBaseCharMovementComponent* BaseCharMovementComponent = BaseCharacter->GetBaseCharacterMovementComponent();

	BaseCharMovementComponent->SetMovementFlag((uint32)EGameplayAbility::Crouch, SavedCustomCompressedFlags & FLAG_IsCrouching && BaseCharacter->IsCrouching());
	BaseCharMovementComponent->SetMovementFlag((uint32)EGameplayAbility::Prone, SavedCustomCompressedFlags & FLAG_IsProne && BaseCharacter->IsProne());
	BaseCharMovementComponent->SetMovementFlag((uint32)EGameplayAbility::Sprint, SavedCustomCompressedFlags & FLAG_IsSprinting && BaseCharacter->IsSprinting());
	BaseCharMovementComponent->SetMovementFlag((uint32)EGameplayAbility::Slide, SavedCustomCompressedFlags & FLAG_IsSliding && BaseCharacter->IsSliding());
	BaseCharMovementComponent->SetMovementFlag((uint32)EGameplayAbility::OutOfStamina, SavedCustomCompressedFlags & FLAG_IsOutOfStamina && BaseCharacter->IsOutOfStamina());
	BaseCharMovementComponent->SetMovementFlag((uint32)EGameplayAbility::Dive, SavedCustomCompressedFlags & FLAG_IsDiving && BaseCharacter->IsDiving());
	BaseCharMovementComponent->SetMovementFlag((uint32)EGameplayAbility::AimWeapon, SavedCustomCompressedFlags & FLAG_IsAiming && BaseCharacter->IsAiming());
	BaseCharMovementComponent->SetMovementFlag((uint32)EGameplayAbility::ReloadWeapon, SavedCustomCompressedFlags & FLAG_IsReloading && BaseCharacter->IsReloadingWeapon());
	BaseCharMovementComponent->SetMovementFlag((uint32)EGameplayAbility::ThrowItem, SavedCustomCompressedFlags & FLAG_IsThrowing && BaseCharacter->IsThrowingItem());

	BaseCharMovementComponent->WallRunElapsedTime = SavedWallRunElapsedTime;
}

FNetworkPredictionData_Client_XyzCharacter::FNetworkPredictionData_Client_XyzCharacter(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement) {}

FSavedMovePtr FNetworkPredictionData_Client_XyzCharacter::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_XyzCharacter());
}

bool FXyzNetworkMoveData::Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar, UPackageMap* PackageMap, ENetworkMoveType MoveType)
{
	if (!Super::Serialize(CharacterMovement, Ar, PackageMap, MoveType))
	{
		return false;
	}

	bool bIsSaving = Ar.IsSaving();
	SerializeOptionalValue<uint16>(bIsSaving, Ar, CustomCompressedFlags, 0);
	Ar << WallRunElapsedTime;

	return !Ar.IsError();
}

void FXyzNetworkMoveData::ClientFillNetworkMoveData(const FSavedMove_Character& ClientMove, ENetworkMoveType MoveType)
{
	Super::ClientFillNetworkMoveData(ClientMove, MoveType);

	const FSavedMove_XyzCharacter& CustomClientMove = StaticCast<const FSavedMove_XyzCharacter&>(ClientMove);
	CustomCompressedFlags = CustomClientMove.SavedCustomCompressedFlags;
	WallRunElapsedTime = CustomClientMove.SavedWallRunElapsedTime;
}
#pragma endregion

UXyzBaseCharMovementComponent::UXyzBaseCharMovementComponent()
{
	MovementFlags.AddZeroed((uint32)EGameplayAbility::Max);
	SetNetworkMoveDataContainer(XyzNetworkMoveDataContainer);
}

void UXyzBaseCharMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(GetOwner()->IsA<AXyzBaseCharacter>(), TEXT("UXyzBaseCharMovementComponent::BeginPlay(): UXyzBaseCharMovementComponent can only be used with AXyzBaseCharacter."))
	BaseCharacterOwner = StaticCast<AXyzBaseCharacter*>(GetOwner());
	if (IsValid(BaseCharacterOwner))
	{
		CharacterEquipmentComponent = BaseCharacterOwner->GetCharacterEquipmentComponent();
	}

	CachedBuoyancy = Buoyancy;
}

float UXyzBaseCharMovementComponent::GetMaxSpeed() const
{
	switch (MovementMode)
	{
		case MOVE_Walking:
		case MOVE_NavWalking:
			if (GetMovementFlag((uint32)EGameplayAbility::Prone))
			{
				return ProneSpeed;
			}
			if (GetMovementFlag((uint32)EGameplayAbility::Crouch))
			{
				return MaxWalkSpeedCrouched;
			}
			if (GetMovementFlag((uint32)EGameplayAbility::OutOfStamina))
			{
				return OutOfStaminaSpeed;
			}
			if (GetMovementFlag((uint32)EGameplayAbility::ThrowItem))
			{
				return BaseCharacterOwner->GetCurrentThrowItemMovementSpeed();
			}
			if (GetMovementFlag((uint32)EGameplayAbility::ReloadWeapon))
			{
				return BaseCharacterOwner->GetCurrentReloadingWalkSpeed();
			}
			if (GetMovementFlag((uint32)EGameplayAbility::AimWeapon))
			{
				return BaseCharacterOwner->GetCurrentAimingMovementSpeed();
			}
			if (GetMovementFlag((uint32)EGameplayAbility::Slide))
			{
				return SlideSpeed;
			}
			if (GetMovementFlag((uint32)EGameplayAbility::Sprint))
			{
				return SprintSpeed;
			}
			return MaxWalkSpeed;
		case MOVE_Custom:
			if (CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Ladder)
			{
				return ClimbingOnLadderMaxSpeed;
			}
			if (CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Zipline)
			{
				return ZiplineMovementMaxSpeed;
			}
			if (CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_WallRun)
			{
				return WallRunMaxSpeed;
			}
			return MaxCustomMovementSpeed;
		case MOVE_Falling:
			return MaxWalkSpeed;
		case MOVE_Swimming:
			if (GetMovementFlag((uint32)EGameplayAbility::Dive))
			{
				return DiveSpeed;
			}
			return MaxSwimSpeed;
		case MOVE_Flying:
			return MaxFlySpeed;
		case MOVE_None:
		default:
			return 0.f;
	}
}

void UXyzBaseCharMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	switch (MovementMode)
	{
		case MOVE_Walking:
			ForceTargetRotation = FRotator::ZeroRotator;
			bForceRotation = false;
			CurrentWallRunSide = EWallRunSide::None;

			if (PreviousMovementMode == MOVE_Falling)
			{
				bCanUseSameWallRunSide = false;
			}
			break;
		case MOVE_Falling:
			bNotifyApex = true;
			break;
		default:
			break;
	}

	if (PreviousMovementMode == MOVE_Swimming)
	{
		OnDiving(false);
		SetSwimmingOnWaterPlane(false);
	}
}

void UXyzBaseCharMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	if (BaseCharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		return;
	}

	switch (CustomMovementMode)
	{
		case (uint8)ECustomMovementMode::CMOVE_Mantling:
			{
				PhysMantling(DeltaTime, Iterations);
				break;
			}
		case (uint8)ECustomMovementMode::CMOVE_Ladder:
			{
				PhysLadder(DeltaTime, Iterations);
				break;
			}
		case (uint8)ECustomMovementMode::CMOVE_Zipline:
			{
				PhysZipline(DeltaTime, Iterations);
				break;
			}
		case (uint8)ECustomMovementMode::CMOVE_WallRun:
			{
				PhysWallRun(DeltaTime, Iterations);
				break;
			}
		default:
			break;
	}

	Super::PhysCustom(DeltaTime, Iterations);
}

void UXyzBaseCharMovementComponent::PhysicsRotation(float DeltaTime)
{
	if (bForceRotation)
	{
		FRotator CurrentRotation = UpdatedComponent->GetComponentRotation();
		CurrentRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): CurrentRotation"));

		FRotator DeltaRot = GetDeltaRotation(DeltaTime);
		DeltaRot.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): GetDeltaRotation"));

		constexpr float AngleTolerance = 1e-3f;

		if (!CurrentRotation.Equals(ForceTargetRotation, AngleTolerance))
		{
			FRotator DesiredRotation = ForceTargetRotation;

			if (!FMath::IsNearlyEqual(CurrentRotation.Pitch, DesiredRotation.Pitch, AngleTolerance))
			{
				DesiredRotation.Pitch = FMath::FixedTurn(CurrentRotation.Pitch, DesiredRotation.Pitch, DeltaRot.Pitch);
			}

			if (!FMath::IsNearlyEqual(CurrentRotation.Yaw, DesiredRotation.Yaw, AngleTolerance))
			{
				DesiredRotation.Yaw = FMath::FixedTurn(CurrentRotation.Yaw, DesiredRotation.Yaw, DeltaRot.Yaw);
			}

			if (!FMath::IsNearlyEqual(CurrentRotation.Roll, DesiredRotation.Roll, AngleTolerance))
			{
				DesiredRotation.Roll = FMath::FixedTurn(CurrentRotation.Roll, DesiredRotation.Roll, DeltaRot.Roll);
			}

			DesiredRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): DesiredRotation"));
			MoveUpdatedComponent(FVector::ZeroVector, DesiredRotation, false);
		}
		else
		{
			ForceTargetRotation = FRotator::ZeroRotator;
			bForceRotation = false;
		}
	}

	if (IsOnLadder())
	{
		return;
	}

	Super::PhysicsRotation(DeltaTime);
}

#pragma region REPLICATION

FNetworkPredictionData_Client* UXyzBaseCharMovementComponent::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		UXyzBaseCharMovementComponent* MutableThis = const_cast<UXyzBaseCharMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_XyzCharacter(*this);
	}

	return Super::GetPredictionData_Client();
}

void UXyzBaseCharMovementComponent::MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector& NewAccel)
{
	const FXyzNetworkMoveData* CustomMoveData = StaticCast<FXyzNetworkMoveData*>(GetCurrentNetworkMoveData());

	uint16 CustomFlags = CustomMoveData->CustomCompressedFlags;
	SetMovementFlag((uint32)EGameplayAbility::Crouch, CustomFlags & FSavedMove_XyzCharacter::FLAG_IsCrouching && BaseCharacterOwner->IsCrouching());
	SetMovementFlag((uint32)EGameplayAbility::Prone, CustomFlags & FSavedMove_XyzCharacter::FLAG_IsProne && BaseCharacterOwner->IsProne());
	SetMovementFlag((uint32)EGameplayAbility::Sprint, CustomFlags & FSavedMove_XyzCharacter::FLAG_IsSprinting && BaseCharacterOwner->IsSprinting());
	SetMovementFlag((uint32)EGameplayAbility::Slide, CustomFlags & FSavedMove_XyzCharacter::FLAG_IsSliding && BaseCharacterOwner->IsSliding());
	SetMovementFlag((uint32)EGameplayAbility::OutOfStamina, CustomFlags & FSavedMove_XyzCharacter::FLAG_IsOutOfStamina && BaseCharacterOwner->IsOutOfStamina());
	SetMovementFlag((uint32)EGameplayAbility::Dive, CustomFlags & FSavedMove_XyzCharacter::FLAG_IsDiving && BaseCharacterOwner->IsDiving());
	SetMovementFlag((uint32)EGameplayAbility::AimWeapon, CustomFlags & FSavedMove_XyzCharacter::FLAG_IsAiming && BaseCharacterOwner->IsAiming());
	SetMovementFlag((uint32)EGameplayAbility::ReloadWeapon, CustomFlags & FSavedMove_XyzCharacter::FLAG_IsReloading && BaseCharacterOwner->IsReloadingWeapon());
	SetMovementFlag((uint32)EGameplayAbility::ThrowItem, CustomFlags & FSavedMove_XyzCharacter::FLAG_IsThrowing && BaseCharacterOwner->IsThrowingItem());

	WallRunElapsedTime = CustomMoveData->WallRunElapsedTime;

	Super::MoveAutonomous(ClientTimeStamp, DeltaTime, CompressedFlags, NewAccel);
}

bool UXyzBaseCharMovementComponent::GetMovementFlag(uint32 FlagIndex) const
{
	return FlagIndex < (uint32)MovementFlags.Num() && MovementFlags[FlagIndex];
}

void UXyzBaseCharMovementComponent::SetMovementFlag(uint32 FlagIndex, bool bIsActivated)
{
	if (FlagIndex < (uint32)MovementFlags.Num())
	{
		MovementFlags[FlagIndex] = bIsActivated;
	}
}
#pragma endregion

#pragma region SWIMMING

FRotator UXyzBaseCharMovementComponent::ComputeOrientToMovementRotation(const FRotator& CurrentRotation, float DeltaTime, FRotator& DeltaRotation) const
{
	FRotator Result;
	if (Acceleration.SizeSquared() < KINDA_SMALL_NUMBER)
	{
		// AI path following request can orient us in that direction (it's effectively an acceleration)
		if (bHasRequestedVelocity && RequestedVelocity.SizeSquared() > KINDA_SMALL_NUMBER)
		{
			Result = RequestedVelocity.GetSafeNormal().Rotation();
		}
		else
		{
			// Don't change rotation if there is no acceleration.
			Result = CurrentRotation;
		}
	}
	else
	{
		// Rotate toward direction of acceleration.
		Result = Acceleration.GetSafeNormal().Rotation();
	}

	// Disable pitch rotation produced by movement while swimming
	if (IsSwimming())
	{
		Result.Pitch = BaseCharacterOwner->GetControlRotation().Pitch;
	}
	return Result;
}

void UXyzBaseCharMovementComponent::UpdateSwimmingCapsuleSize() const
{
	if (IsSwimming())
	{
		BaseCharacterOwner->GetCapsuleComponent()->SetCapsuleSize(SwimmingCapsuleRadius, SwimmingCapsuleHalfHeight);
	}
	else
	{
		const ACharacter* DefaultCharacter = BaseCharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
		BaseCharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), true);
	}
}

void UXyzBaseCharMovementComponent::Multicast_UpdateSwimmingCapsuleSize_Implementation()
{
	UpdateSwimmingCapsuleSize();
}

void UXyzBaseCharMovementComponent::SetSwimmingOnWaterPlane(bool bInIsSwimmingOnWaterPlane)
{
	bIsSwimmingOnWaterPlane = bInIsSwimmingOnWaterPlane;
}

bool UXyzBaseCharMovementComponent::IsSwimmingUnderWater(FVector LocationOffset) const
{
	const APhysicsVolume* Volume = GetPhysicsVolume();
	float VolumeTopPlane = Volume->GetActorLocation().Z + Volume->GetBounds().BoxExtent.Z * Volume->GetActorScale3D().Z;
	float HeadPositionZ = BaseCharacterOwner->GetActorLocation().Z + WaterPlaneDetectionRangeZ;
	if (HeadPositionZ + LocationOffset.Z < VolumeTopPlane)
	{
		return true;
	}
	return false;
}

void UXyzBaseCharMovementComponent::OnDiving(bool bIsDiving)
{
	SetSwimmingOnWaterPlane(bIsDiving ? false : bIsSwimmingOnWaterPlane);
	BaseCharacterOwner->OnDiving(bIsDiving);
}

void UXyzBaseCharMovementComponent::PhysSwimming(float DeltaTime, int32 Iterations)
{
	Super::PhysSwimming(DeltaTime, Iterations);

	if (bIsSwimmingOnWaterPlane && !BaseCharacterOwner->IsDiveAbilityActive())
	{
		Velocity.Z = 0.f;
		Buoyancy = 1.f;
		FVector TestStartLocation = BaseCharacterOwner->GetActorLocation();
		FVector WaterPlaneLocation = FindWaterLine(TestStartLocation, TestStartLocation + FVector::UpVector * WaterPlaneDetectionRangeZ);
		FVector Delta = WaterPlaneLocation - TestStartLocation + FVector::UpVector * WaterSnappingOffsetZ;
		float SpeedDelta = EmergeSpeed * DeltaTime;
		if (Delta.Z > SpeedDelta)
		{
			FVector EmergeDirection = WaterPlaneLocation - TestStartLocation;
			EmergeDirection.Normalize();
			Delta = FVector::UpVector * SpeedDelta;
		}
		FHitResult Hit;
		SafeMoveUpdatedComponent(Delta, BaseCharacterOwner->GetActorRotation(), false, Hit);
	}
	else
	{
		Buoyancy = CachedBuoyancy;
		if (!BaseCharacterOwner->IsDiving())
		{
			if (IsSwimmingUnderWater())
			{
				OnDiving(true);
			}
		}
		else if (!IsSwimmingUnderWater(Velocity * DeltaTime))
		{
			OnDiving(false);
			SetSwimmingOnWaterPlane(true);
		}
	}
}
#pragma endregion

#pragma region SPRINTING / SLIDING

void UXyzBaseCharMovementComponent::StartSprint()
{
	bForceMaxAccel = 1;
	SetMovementFlag((uint32)EGameplayAbility::Sprint, true);
	BaseCharacterOwner->OnStartSprint();
}

void UXyzBaseCharMovementComponent::StopSprint()
{
	bForceMaxAccel = 0;
	SetMovementFlag((uint32)EGameplayAbility::Sprint, false);
	BaseCharacterOwner->OnStopSprint();
}

void UXyzBaseCharMovementComponent::StartSlide()
{
	if (BaseCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == SlideCapsuleHalfHeight)
	{
		SetMovementFlag((uint32)EGameplayAbility::Slide, true);
		BaseCharacterOwner->OnStartSlide(0.f, 0.f);
		return;
	}

	float ComponentScale = BaseCharacterOwner->GetCapsuleComponent()->GetShapeScale();
	float OldUnscaledHalfHeight = BaseCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	float OldUnscaledRadius = BaseCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	float ClampedHalfHeight = FMath::Max3(0.f, OldUnscaledRadius, SlideCapsuleHalfHeight);
	BaseCharacterOwner->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, ClampedHalfHeight);

	float HalfHeightAdjust = OldUnscaledHalfHeight - ClampedHalfHeight;
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	UpdatedComponent->MoveComponent(FVector(0.f, 0.f, -ScaledHalfHeightAdjust), UpdatedComponent->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);

	bForceNextFloorCheck = true;
	SetMovementFlag((uint32)EGameplayAbility::Slide, true);
	BaseCharacterOwner->OnStartSlide(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void UXyzBaseCharMovementComponent::StopSlide()
{
	if (!HasValidData())
	{
		return;
	}

	ACharacter* DefaultCharacter = BaseCharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	float OldUnscaledHalfHeight = BaseCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	float OldUnscaledRadius = BaseCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	float ClampedSlideHalfHeight = FMath::Max3(0.f, OldUnscaledRadius, SlideCapsuleHalfHeight);

	if (OldUnscaledHalfHeight != ClampedSlideHalfHeight)
	{
		SetMovementFlag((uint32)EGameplayAbility::Slide, false);
		BaseCharacterOwner->OnStopSlide(0.f, 0.f);
		return;
	}

	float CurrentHalfHeight = BaseCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float ComponentScale = BaseCharacterOwner->GetCapsuleComponent()->GetShapeScale();
	float HalfHeightAdjust = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - OldUnscaledHalfHeight;
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	FVector PawnLocation = UpdatedComponent->GetComponentLocation();

	check(BaseCharacterOwner->GetCapsuleComponent());
	const UWorld* MyWorld = GetWorld();
	float SweepInflation = KINDA_SMALL_NUMBER * 10.f;
	FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(ProneTrace), false, BaseCharacterOwner);
	FCollisionResponseParams ResponseParam;
	InitCollisionParams(CapsuleParams, ResponseParam);
	FCollisionShape StandingCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -SweepInflation - ScaledHalfHeightAdjust);
	ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
	bool bEncroached = true;

	FVector StandingLocation = PawnLocation + (StandingCapsuleShape.GetCapsuleHalfHeight() - CurrentHalfHeight) * FVector::UpVector;
	bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);

	if (bEncroached)
	{
		if (IsMovingOnGround())
		{
			float MinFloorDist = KINDA_SMALL_NUMBER * 10.f;
			if (CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
			{
				StandingLocation.Z -= CurrentFloor.FloorDist - MinFloorDist;
				bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
			}
		}
	}

	UpdatedComponent->MoveComponent(StandingLocation - PawnLocation, UpdatedComponent->GetComponentQuat(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
	bForceNextFloorCheck = true;
	BaseCharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), true);
	AdjustProxyCapsuleSize();
	SetMovementFlag((uint32)EGameplayAbility::Slide, false);
	BaseCharacterOwner->OnStopSlide(HalfHeightAdjust, ScaledHalfHeightAdjust);
}
#pragma endregion

#pragma region CROUCHING / PRONE

void UXyzBaseCharMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	// Disabling this logic
	//Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UXyzBaseCharMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	// Disabling this logic
	//Super::UpdateCharacterStateAfterMovement(DeltaSeconds);
}

bool UXyzBaseCharMovementComponent::IsCrouching() const
{
	return BaseCharacterOwner->IsCrouching();
}

bool UXyzBaseCharMovementComponent::CanUnCrouch() const
{
	if (!HasValidData())
	{
		return false;
	}

	bool Result = false;

	// Same overlap test as in UCharacterMovementComponent::UnCrouch
	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	check(BaseCharacterOwner->GetCapsuleComponent());
	float CurrentCrouchedHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	float HalfHeightAdjust = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - OldUnscaledHalfHeight;
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	FVector PawnLocation = UpdatedComponent->GetComponentLocation();
	const UWorld* MyWorld = GetWorld();
	float SweepInflation = KINDA_SMALL_NUMBER * 10.f;
	FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(CrouchTrace), false, CharacterOwner);
	FCollisionResponseParams ResponseParam;
	InitCollisionParams(CapsuleParams, ResponseParam);
	FCollisionShape StandingCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -SweepInflation - ScaledHalfHeightAdjust); // Shrink by negative amount, so actually grow it.
	ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
	FVector StandingLocation = PawnLocation + FVector(0.f, 0.f, StandingCapsuleShape.GetCapsuleHalfHeight() - CurrentCrouchedHalfHeight);

	Result = !MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
	if (!Result)
	{
		if (IsMovingOnGround())
		{
			float MinFloorDist = KINDA_SMALL_NUMBER * 10.f;
			if (CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
			{
				StandingLocation.Z -= CurrentFloor.FloorDist - MinFloorDist;
				Result = !MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
			}
		}
	}

	return Result;
}

void UXyzBaseCharMovementComponent::Crouch(bool bClientSimulation/* = false*/)
{
	// Mostly copied from UCharacterMovementComponent excluding replication

	if (!HasValidData())
	{
		return;
	}

	if (!CanCrouchInCurrentState())
	{
		return;
	}

	float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	float OldUnscaledRadius = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	// Height is not allowed to be smaller than radius.
	float ClampedCrouchedHalfHeight = FMath::Max3(0.f, OldUnscaledRadius, CrouchedHalfHeight);

	// See if collision is already at desired size.
	if (OldUnscaledHalfHeight == ClampedCrouchedHalfHeight)
	{
		SetMovementFlag((uint32)EGameplayAbility::Crouch, true);
		CharacterOwner->OnStartCrouch(0.f, 0.f);
		return;
	}

	// Change collision size to crouching dimensions
	float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, ClampedCrouchedHalfHeight);
	float HalfHeightAdjust = (OldUnscaledHalfHeight - ClampedCrouchedHalfHeight);
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	// Crouching to a larger height? (this is rare)
	if (ClampedCrouchedHalfHeight > OldUnscaledHalfHeight)
	{
		FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(CrouchTrace), false, CharacterOwner);
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(CapsuleParams, ResponseParam);
		bool bEncroached = GetWorld()->OverlapBlockingTestByChannel(UpdatedComponent->GetComponentLocation() - FVector(0.f, 0.f, ScaledHalfHeightAdjust), FQuat::Identity,
		                                                            UpdatedComponent->GetCollisionObjectType(), GetPawnCapsuleCollisionShape(SHRINK_None), CapsuleParams, ResponseParam);

		// If encroached, cancel
		if (bEncroached)
		{
			CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, OldUnscaledHalfHeight);
			return;
		}
	}

	if (bCrouchMaintainsBaseLocation)
	{
		// Intentionally not using MoveUpdatedComponent, where a horizontal plane constraint would prevent the base of the capsule from staying at the same spot.
		UpdatedComponent->MoveComponent(FVector(0.f, 0.f, -ScaledHalfHeightAdjust), UpdatedComponent->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
	}

	bForceNextFloorCheck = true;

	// OnStartCrouch takes the change from the Default size, not the current one (though they are usually the same).
	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	HalfHeightAdjust = (DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - ClampedCrouchedHalfHeight);
	ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	AdjustProxyCapsuleSize();
	SetMovementFlag((uint32)EGameplayAbility::Crouch, true);
	CharacterOwner->OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void UXyzBaseCharMovementComponent::UnCrouch(bool bClientSimulation/* = false*/)
{
	// Mostly copied from UCharacterMovementComponent excluding replication

	if (!HasValidData())
	{
		return;
	}

	float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	float OldUnscaledRadius = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	// Height is not allowed to be smaller than radius.
	float ClampedCrouchedHalfHeight = FMath::Max3(0.f, OldUnscaledRadius, CrouchedHalfHeight);

	// See if collision is not at crouching size or already at desired size.
	if (OldUnscaledHalfHeight != ClampedCrouchedHalfHeight)
	{
		SetMovementFlag((uint32)EGameplayAbility::Crouch, false);
		CharacterOwner->OnEndCrouch(0.f, 0.f);
		return;
	}

	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	float CurrentCrouchedHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	float HalfHeightAdjust = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - OldUnscaledHalfHeight;
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	FVector PawnLocation = UpdatedComponent->GetComponentLocation();

	check(CharacterOwner->GetCapsuleComponent());

	// Try to stay in place and see if the larger capsule fits. We use a slightly taller capsule to avoid penetration.
	const UWorld* MyWorld = GetWorld();
	float SweepInflation = KINDA_SMALL_NUMBER * 10.f;
	FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(CrouchTrace), false, CharacterOwner);
	FCollisionResponseParams ResponseParam;
	InitCollisionParams(CapsuleParams, ResponseParam);

	// Compensate for the difference between current capsule size and standing size
	FCollisionShape StandingCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -SweepInflation - ScaledHalfHeightAdjust); // Shrink by negative amount, so actually grow it.
	ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
	bool bEncroached = true;

	if (!bCrouchMaintainsBaseLocation)
	{
		// Expand in place
		bEncroached = MyWorld->OverlapBlockingTestByChannel(PawnLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);

		if (bEncroached)
		{
			// Try adjusting capsule position to see if we can avoid encroachment.
			if (ScaledHalfHeightAdjust > 0.f)
			{
				// Shrink to a short capsule, sweep down to base to find where that would hit something, and then try to stand up from there.
				float PawnRadius, PawnHalfHeight;
				CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);
				float ShrinkHalfHeight = PawnHalfHeight - PawnRadius;
				float TraceDist = PawnHalfHeight - ShrinkHalfHeight;
				FVector Down = FVector(0.f, 0.f, -TraceDist);

				FHitResult Hit(1.f);
				FCollisionShape ShortCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, ShrinkHalfHeight);
				bool bBlockingHit = MyWorld->SweepSingleByChannel(Hit, PawnLocation, PawnLocation + Down, FQuat::Identity, CollisionChannel, ShortCapsuleShape, CapsuleParams);
				if (Hit.bStartPenetrating)
				{
					bEncroached = true;
				}
				else
				{
					// Compute where the base of the sweep ended up, and see if we can stand there
					float DistanceToBase = (Hit.Time * TraceDist) + ShortCapsuleShape.Capsule.HalfHeight;
					FVector NewLoc = FVector(PawnLocation.X, PawnLocation.Y, PawnLocation.Z - DistanceToBase + StandingCapsuleShape.Capsule.HalfHeight + SweepInflation + MIN_FLOOR_DIST / 2.f);
					bEncroached = MyWorld->OverlapBlockingTestByChannel(NewLoc, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
					if (!bEncroached)
					{
						// Intentionally not using MoveUpdatedComponent, where a horizontal plane constraint would prevent the base of the capsule from staying at the same spot.
						UpdatedComponent->MoveComponent(NewLoc - PawnLocation, UpdatedComponent->GetComponentQuat(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
					}
				}
			}
		}
	}
	else
	{
		// Expand while keeping base location the same.
		FVector StandingLocation = PawnLocation + FVector(0.f, 0.f, StandingCapsuleShape.GetCapsuleHalfHeight() - CurrentCrouchedHalfHeight);
		bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);

		if (bEncroached)
		{
			if (IsMovingOnGround())
			{
				// Something might be just barely overhead, try moving down closer to the floor to avoid it.
				float MinFloorDist = KINDA_SMALL_NUMBER * 10.f;
				if (CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
				{
					StandingLocation.Z -= CurrentFloor.FloorDist - MinFloorDist;
					bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
				}
			}
		}

		if (!bEncroached)
		{
			// Commit the change in location.
			UpdatedComponent->MoveComponent(StandingLocation - PawnLocation, UpdatedComponent->GetComponentQuat(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
			bForceNextFloorCheck = true;
		}
	}

	// If still encroached then abort.
	if (bEncroached)
	{
		return;
	}

	// Now call SetCapsuleSize() to cause touch/untouch events and actually grow the capsule
	CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), true);

	AdjustProxyCapsuleSize();
	SetMovementFlag((uint32)EGameplayAbility::Crouch, false);
	CharacterOwner->OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

bool UXyzBaseCharMovementComponent::CanUnProne() const
{
	if (!HasValidData())
	{
		return false;
	}

	bool Result = false;

	// Same overlap test as in UCharacterMovementComponent::UnProne
	check(BaseCharacterOwner->GetCapsuleComponent());
	float OldUnscaledRadius = BaseCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	float ClampedCrouchHalfHeight = FMath::Max3(0.f, OldUnscaledRadius, CrouchedHalfHeight);
	float CurrentProneHalfHeight = BaseCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float ComponentScale = BaseCharacterOwner->GetCapsuleComponent()->GetShapeScale();
	float OldUnscaledHalfHeight = BaseCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	float HalfHeightAdjust = ClampedCrouchHalfHeight - OldUnscaledHalfHeight;
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	FVector PawnLocation = UpdatedComponent->GetComponentLocation();
	const UWorld* MyWorld = GetWorld();
	float SweepInflation = KINDA_SMALL_NUMBER * 10.f;
	FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(ProneTrace), false, BaseCharacterOwner);
	FCollisionResponseParams ResponseParam;
	InitCollisionParams(CapsuleParams, ResponseParam);
	FCollisionShape CrouchingCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -SweepInflation - ScaledHalfHeightAdjust);
	ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
	FVector CrouchingLocation = PawnLocation + (CrouchingCapsuleShape.GetCapsuleHalfHeight() - CurrentProneHalfHeight) * FVector::UpVector;

	Result = !MyWorld->OverlapBlockingTestByChannel(CrouchingLocation, FQuat::Identity, CollisionChannel, CrouchingCapsuleShape, CapsuleParams, ResponseParam);
	if (!Result)
	{
		if (IsMovingOnGround())
		{
			float MinFloorDist = KINDA_SMALL_NUMBER * 10.f;
			if (CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
			{
				CrouchingLocation.Z -= CurrentFloor.FloorDist - MinFloorDist;
				Result = !MyWorld->OverlapBlockingTestByChannel(CrouchingLocation, FQuat::Identity, CollisionChannel, CrouchingCapsuleShape, CapsuleParams, ResponseParam);
			}
		}
	}

	return Result;
}

void UXyzBaseCharMovementComponent::Prone()
{
	if (!HasValidData())
	{
		return;
	}

	float OldUnscaledHalfHeight = BaseCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	float OldUnscaledRadius = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	// Height is not allowed to be smaller than radius.
	float ClampedProneHalfHeight = FMath::Max3(0.f, OldUnscaledRadius, ProneCapsuleHalfHeight);

	if (OldUnscaledHalfHeight == ClampedProneHalfHeight)
	{
		return;
	}

	float ComponentScale = BaseCharacterOwner->GetCapsuleComponent()->GetShapeScale();
	BaseCharacterOwner->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, ClampedProneHalfHeight);
	float HalfHeightAdjust = OldUnscaledHalfHeight - ClampedProneHalfHeight;
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	UpdatedComponent->MoveComponent(FVector(0.f, 0.f, -ScaledHalfHeightAdjust), UpdatedComponent->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
	bForceNextFloorCheck = true;

	SetMovementFlag((uint32)EGameplayAbility::Prone, true);
	BaseCharacterOwner->OnStartProne(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

bool UXyzBaseCharMovementComponent::UnProne()
{
	if (!HasValidData())
	{
		return false;
	}

	ACharacter* DefaultCharacter = BaseCharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	float OldUnscaledHalfHeight = BaseCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	float OldUnscaledRadius = BaseCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	// Height is not allowed to be smaller than radius.
	float ClampedProneHalfHeight = FMath::Max3(0.f, OldUnscaledRadius, ProneCapsuleHalfHeight);

	// See if collision is not at prone size or already at desired size.
	if (OldUnscaledHalfHeight != ClampedProneHalfHeight)
	{
		BaseCharacterOwner->OnStopProne(0.f, 0.f);
		SetMovementFlag((uint32)EGameplayAbility::Prone, false);
		return true;
	}

	float CurrentProneHalfHeight = BaseCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float ComponentScale = BaseCharacterOwner->GetCapsuleComponent()->GetShapeScale();
	float ClampedCrouchHalfHeight = FMath::Max3(0.f, OldUnscaledRadius, CrouchedHalfHeight);
	float HalfHeightAdjust = ClampedCrouchHalfHeight - OldUnscaledHalfHeight;
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	FVector PawnLocation = UpdatedComponent->GetComponentLocation();

	check(BaseCharacterOwner->GetCapsuleComponent());

	const UWorld* MyWorld = GetWorld();
	float SweepInflation = KINDA_SMALL_NUMBER * 10.f;
	FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(ProneTrace), false, BaseCharacterOwner);
	FCollisionResponseParams ResponseParam;
	InitCollisionParams(CapsuleParams, ResponseParam);

	FCollisionShape CrouchingCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -SweepInflation - ScaledHalfHeightAdjust);
	ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
	bool bEncroached = true;

	FVector CrouchingLocation = PawnLocation + (CrouchingCapsuleShape.GetCapsuleHalfHeight() - CurrentProneHalfHeight) * FVector::UpVector;
	bEncroached = MyWorld->OverlapBlockingTestByChannel(CrouchingLocation, FQuat::Identity, CollisionChannel, CrouchingCapsuleShape, CapsuleParams, ResponseParam);

	if (bEncroached)
	{
		if (IsMovingOnGround())
		{
			float MinFloorDist = KINDA_SMALL_NUMBER * 10.f;
			if (CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
			{
				CrouchingLocation.Z -= CurrentFloor.FloorDist - MinFloorDist;
				bEncroached = MyWorld->OverlapBlockingTestByChannel(CrouchingLocation, FQuat::Identity, CollisionChannel, CrouchingCapsuleShape, CapsuleParams, ResponseParam);
			}
		}
	}

	if (bEncroached)
	{
		return false;
	}

	UpdatedComponent->MoveComponent(CrouchingLocation - PawnLocation, UpdatedComponent->GetComponentQuat(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
	bForceNextFloorCheck = true;

	BaseCharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), ClampedCrouchHalfHeight, true);
	AdjustProxyCapsuleSize();
	SetMovementFlag((uint32)EGameplayAbility::Prone, false);
	BaseCharacterOwner->OnStopProne(HalfHeightAdjust, ScaledHalfHeightAdjust);

	return true;
}
#pragma endregion

#pragma region MANTLING

bool UXyzBaseCharMovementComponent::IsMantling() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Mantling && UpdatedComponent;
}

void UXyzBaseCharMovementComponent::StartMantle(const FMantlingMovementParameters& MantlingParameters)
{
	BaseCharacterOwner->SetActorEnableCollision(false);
	CurrentMantlingParameters = MantlingParameters;
	if (IsValid(CurrentMantlingParameters.TargetActor))
	{
		OffsetFromMantlingTarget = CurrentMantlingParameters.TargetLocation - CurrentMantlingParameters.TargetActor->GetComponentLocation();
		SetMovementMode(EMovementMode::MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Mantling);
		GetWorld()->GetTimerManager().SetTimer(MantlingTimer, this, &UXyzBaseCharMovementComponent::StopMantle, CurrentMantlingParameters.Duration, false);
	}
}

void UXyzBaseCharMovementComponent::StopMantle()
{
	BaseCharacterOwner->SetActorEnableCollision(true);
	BaseCharacterOwner->StopMantle();
	SetMovementMode(MOVE_Walking);
}

void UXyzBaseCharMovementComponent::PhysMantling(float DeltaTime, int32 Iterations)
{
	if (!IsValid(CurrentMantlingParameters.TargetActor) || !CurrentMantlingParameters.MantlingCurve)
	{
		return;
	}

	float ElapsedTime = GetWorld()->GetTimerManager().GetTimerElapsed(MantlingTimer) + CurrentMantlingParameters.StartTime;
	FVector MantlingCurveValue = CurrentMantlingParameters.MantlingCurve->GetVectorValue(ElapsedTime);
	float PositionAlpha = MantlingCurveValue.X;
	float XYCorrectionAlpha = MantlingCurveValue.Y;
	float ZCorrectionAlpha = MantlingCurveValue.Z;
	FVector CorrectedInitialLocation = FMath::Lerp(CurrentMantlingParameters.InitialLocation, CurrentMantlingParameters.InitialAnimationLocation, XYCorrectionAlpha);
	CorrectedInitialLocation.Z = FMath::Lerp(CurrentMantlingParameters.InitialLocation.Z, CurrentMantlingParameters.InitialAnimationLocation.Z, ZCorrectionAlpha);
	FVector CurrentTargetActorLocation = CurrentMantlingParameters.TargetActor->GetComponentLocation();
	FVector NewLocation = FMath::Lerp(CorrectedInitialLocation, CurrentTargetActorLocation + OffsetFromMantlingTarget, PositionAlpha);
	FRotator NewRotation = FMath::Lerp(CurrentMantlingParameters.InitialRotation, CurrentMantlingParameters.TargetRotation, PositionAlpha);
	FVector Delta = NewLocation - GetActorLocation();
	FHitResult Hit;

	Velocity = Delta / DeltaTime;
	SafeMoveUpdatedComponent(Delta, NewRotation, false, Hit);
}
#pragma endregion

#pragma region LADDER

bool UXyzBaseCharMovementComponent::IsOnLadder() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Ladder && UpdatedComponent;
}

bool UXyzBaseCharMovementComponent::IsOnTopOfCurrentLadder() const
{
	ALadder* Ladder = CurrentLadder.Get();
	if (IsValid(Ladder))
	{
		return Ladder->IsOnTop();
	}
	return false;
}

float UXyzBaseCharMovementComponent::GetLadderSpeedRatio() const
{
	ALadder* Ladder = CurrentLadder.Get();
	if (IsValid(Ladder))
	{
		return FVector::DotProduct(Ladder->GetActorUpVector(), Velocity) / ClimbingOnLadderMaxSpeed;
	}
	return 1.f;
}

void UXyzBaseCharMovementComponent::AttachCharacterToLadder(ALadder* Ladder)
{
	if (!IsValid(Ladder))
	{
		return;
	}

	CurrentLadder = Ladder;
	FVector DepthOffset = Ladder->GetActorForwardVector() * CharacterOffsetFromLadder;
	FVector ProjectionToLadder = GetCharacterProjectionToCurrentLadder(GetActorLocation()) * Ladder->GetActorUpVector();
	FVector StartLocation = Ladder->GetActorLocation() + ProjectionToLadder + DepthOffset;
	FRotator StartRotation = Ladder->GetActorForwardVector().ToOrientationRotator();
	StartRotation.Yaw += 180.f;

	if (Ladder->IsOnTop())
	{
		StartLocation = Ladder->GetAttachFromTopAnimMontageStartLocation();
	}

	BaseCharacterOwner->SetActorLocation(StartLocation);
	BaseCharacterOwner->SetActorRotation(StartRotation);

	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Ladder);
}

void UXyzBaseCharMovementComponent::DetachCharacterFromLadder(EDetachFromLadderMethod DetachFromLadderMethod /*= EDetachFromLadderMethod::Fall*/)
{
	switch (DetachFromLadderMethod)
	{
		case EDetachFromLadderMethod::JumpOff:
			{
				ALadder* Ladder = CurrentLadder.Get();
				if (IsValid(Ladder))
				{
					FVector JumpDirection = Ladder->GetActorForwardVector();
					FVector JumpVelocity = JumpDirection * JumpOffLadderSpeed;
					ForceTargetRotation = JumpDirection.ToOrientationRotator();
					bForceRotation = true;

					Launch(JumpVelocity);
				}

				SetMovementMode(MOVE_Falling);
				break;
			}
		case EDetachFromLadderMethod::ReachingTheTop:
			{
				BaseCharacterOwner->StartMantle();
				break;
			}
		case EDetachFromLadderMethod::ReachingTheBottom:
			{
				SetMovementMode(MOVE_Walking);
				break;
			}
		case EDetachFromLadderMethod::Fall:
		default:
			{
				SetMovementMode(MOVE_Falling);
				break;
			}
	}

	CurrentLadder.Reset();
	BaseCharacterOwner->StopUseEnvironmentActor();
}

float UXyzBaseCharMovementComponent::GetCharacterProjectionToCurrentLadder(FVector Location) const
{
	ALadder* Ladder = CurrentLadder.Get();
	if (IsValid(Ladder))
	{
		FVector LadderToCharacterVector = Location - Ladder->GetActorLocation();
		return FVector::DotProduct(Ladder->GetActorUpVector(), LadderToCharacterVector);
	}

	return 0.f;
}

void UXyzBaseCharMovementComponent::PhysLadder(float DeltaTime, int32 Iterations)
{
	ALadder* Ladder = CurrentLadder.Get();
	if (!IsValid(Ladder))
	{
		return;
	}

	CalcVelocity(DeltaTime, 1.f, false, ClimbingOnLadderBrakingDeceleration);
	FVector Delta = Velocity * DeltaTime;

	if (HasAnimRootMotion())
	{
		FHitResult Hit;
		SafeMoveUpdatedComponent(Delta, BaseCharacterOwner->GetActorRotation(), false, Hit);
		return;
	}

	FVector NextPosition = GetActorLocation() + Delta;
	float NextPositionProjection = GetCharacterProjectionToCurrentLadder(NextPosition);

	if (NextPositionProjection < DetachmentDistanceFromBottom)
	{
		DetachCharacterFromLadder(EDetachFromLadderMethod::ReachingTheBottom);
		return;
	}
	if (NextPositionProjection > (Ladder->GetLadderHeight() - DetachmentDistanceFromTop))
	{
		DetachCharacterFromLadder(EDetachFromLadderMethod::ReachingTheTop);
		return;
	}

	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, BaseCharacterOwner->GetActorRotation(), true, Hit);
}
#pragma endregion

#pragma region ZIPLINE

bool UXyzBaseCharMovementComponent::IsOnZipline() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Zipline && UpdatedComponent;
}

void UXyzBaseCharMovementComponent::AttachCharacterToZipline(AZipline* Zipline)
{
	if (!IsValid(Zipline))
	{
		return;
	}
	CurrentZipline = Zipline;
	FRotator StartRotation = Zipline->GetZiplineSpanVector().ToOrientationRotator();
	FVector StartLocation = Zipline->GetZiplineStartLocation() + StartRotation.RotateVector(ZiplineAttachmentOffset);

	if (BaseCharacterOwner->GetActorLocation().Z < StartLocation.Z)
	{
		return;
	}

	BaseCharacterOwner->SetActorLocation(StartLocation);
	BaseCharacterOwner->SetActorRotation(StartRotation);

	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Zipline);
}

void UXyzBaseCharMovementComponent::DetachCharacterFromZipline(EDetachFromZiplineMethod DetachFromZiplineMethod)
{
	switch (DetachFromZiplineMethod)
	{
		case EDetachFromZiplineMethod::ReachingTheEnd:
			{
				AZipline* Zipline = CurrentZipline.Get();
				if (IsValid(Zipline))
				{
					FVector ZiplineDirection = Zipline->GetZiplineSpanVector();
					FVector JumpDirection = FVector(ZiplineDirection.X, ZiplineDirection.Y, 0.f).RotateAngleAxis(JumpOffZiplineAngle, FVector::UpVector).GetSafeNormal();
					FVector JumpVelocity = JumpDirection * JumpOffZiplineSpeed;
					ForceTargetRotation = JumpDirection.ToOrientationRotator();
					bForceRotation = true;

					Launch(JumpVelocity);
				}

				SetMovementMode(MOVE_Falling);
				break;
			}
		case EDetachFromZiplineMethod::Fall:
		default:
			{
				SetMovementMode(MOVE_Falling);
				break;
			}
	}

	CurrentZipline.Reset();
	BaseCharacterOwner->StopUseEnvironmentActor();
}

float UXyzBaseCharMovementComponent::GetCharacterProjectionToCurrentZipline(FVector Location) const
{
	AZipline* Zipline = CurrentZipline.Get();
	if (IsValid(Zipline))
	{
		FVector ZiplineToCharacterVector = Location - Zipline->GetZiplineStartLocation();
		return FVector::DotProduct(Zipline->GetZiplineSpanVector().GetSafeNormal(), ZiplineToCharacterVector);
	}

	return 0.f;
}

void UXyzBaseCharMovementComponent::PhysZipline(float DeltaTime, int32 Iterations)
{
	AZipline* Zipline = CurrentZipline.Get();
	if (!IsValid(Zipline))
	{
		return;
	}

	Velocity = GetMaxSpeed() * Zipline->GetZiplineSpanVector().GetSafeNormal();
	FVector Delta = Velocity * DeltaTime;

	FVector NextPosition = GetActorLocation() + Delta;
	float NextPositionProjection = GetCharacterProjectionToCurrentZipline(NextPosition);

	if (NextPositionProjection > Zipline->GetZiplineLength() - ZiplineDetachmentDistanceFromEnd)
	{
		DetachCharacterFromZipline(EDetachFromZiplineMethod::ReachingTheEnd);
		return;
	}

	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, BaseCharacterOwner->GetActorRotation(), false, Hit);
}
#pragma endregion

#pragma region WALL RUNNING

bool UXyzBaseCharMovementComponent::IsWallRunning() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_WallRun && UpdatedComponent;
}

void UXyzBaseCharMovementComponent::StartWallRun(const FHitResult& Hit)
{
	if (!CanWallRunInCurrentState() || !IsSurfaceWallRunnable(Hit.ImpactNormal) || !AttachCharacterToRunnableWall(Hit))
	{
		return;
	}

	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_WallRun);
	BaseCharacterOwner->OnWallRunStart();
}

void UXyzBaseCharMovementComponent::DetachCharacterFromRunnableWall(EDetachFromRunnableWallMethod DetachFromRunnableWallMethod /*= EDetachFromRunnableWallMethod::Fall*/)
{
	FRotator NewRotation = BaseCharacterOwner->GetActorRotation();
	NewRotation.Roll = 0.f;
	BaseCharacterOwner->SetActorRotation(NewRotation);

	switch (DetachFromRunnableWallMethod)
	{
		case EDetachFromRunnableWallMethod::JumpOff:
			{
				FVector WallRunDirectionRightVector = FVector::CrossProduct(FVector::UpVector, CurrentWallRunDirection);
				FVector JumpDirection = CurrentWallRunSide == EWallRunSide::Left ? WallRunDirectionRightVector : -WallRunDirectionRightVector;
				JumpDirection += Velocity.GetSafeNormal();
				SetMovementMode(MOVE_Falling);

				FVector JumpVelocity = JumpDirection * JumpOffWallRunSpeed;

				ForceTargetRotation = JumpDirection.ToOrientationRotator();
				bForceRotation = true;
				Launch(JumpVelocity);

				bCanUseSameWallRunSide = true;
				break;
			}
		case EDetachFromRunnableWallMethod::Fall:
		default:
			{
				SetMovementMode(MOVE_Falling);
				break;
			}
	}

	WallRunElapsedTime = 0.f;
	BaseCharacterOwner->OnWallRunEnd();
}

bool UXyzBaseCharMovementComponent::CanWallRunInCurrentState() const
{
	return IsFalling() && UpdatedComponent && !UpdatedComponent->IsSimulatingPhysics();
}

bool UXyzBaseCharMovementComponent::IsSurfaceWallRunnable(const FVector& SurfaceNormal) const
{
	if (SurfaceNormal.Z > GetWalkableFloorZ() || SurfaceNormal.Z < -KINDA_SMALL_NUMBER)
	{
		return false;
	}

	return true;
}

void UXyzBaseCharMovementComponent::GetWallRunSideAndDirection(FVector HitNormal, EWallRunSide& OutSide, FVector& OutDirection) const
{
	if (FVector::DotProduct(HitNormal, BaseCharacterOwner->GetActorRightVector()) > 0)
	{
		OutSide = EWallRunSide::Left;
		OutDirection = FVector::CrossProduct(HitNormal, FVector::UpVector).GetSafeNormal();
	}
	else
	{
		OutSide = EWallRunSide::Right;
		OutDirection = FVector::CrossProduct(FVector::UpVector, HitNormal).GetSafeNormal();
	}
}

void UXyzBaseCharMovementComponent::GetUpdatedWallRunDeltaAndRotation(float DeltaTime, const FHitResult& HitResult, FVector& DisplacementDelta, FRotator& UpdatedCharacterRotation) const
{
	float OldCurveValue = 0.f;
	float NewCurveValue = 0.f;
	if (WallRunVerticalDisplacementCurve)
	{
		OldCurveValue = WallRunVerticalDisplacementCurve->GetFloatValue(WallRunElapsedTime / WallRunMaxDuration);
		NewCurveValue = WallRunVerticalDisplacementCurve->GetFloatValue((WallRunElapsedTime + DeltaTime) / WallRunMaxDuration);
	}

	FVector WallUpVector;
	float UpdatedAngleOffset;
	FVector Axis;
	float WallTilt;
	if (CurrentWallRunSide == EWallRunSide::Left)
	{
		WallUpVector = FVector::CrossProduct(CurrentWallRunDirection, HitResult.ImpactNormal).GetSafeNormal();
		FQuat::FindBetweenVectors(FVector::UpVector, WallUpVector).ToAxisAndAngle(Axis, WallTilt);
		WallTilt = FMath::RadiansToDegrees(WallTilt);
		UpdatedAngleOffset = CharacterAngleOffsetFromWallPlane - WallTilt;
	}
	else
	{
		WallUpVector = FVector::CrossProduct(HitResult.ImpactNormal, CurrentWallRunDirection).GetSafeNormal();
		FQuat::FindBetweenVectors(FVector::UpVector, WallUpVector).ToAxisAndAngle(Axis, WallTilt);
		WallTilt = FMath::RadiansToDegrees(WallTilt);
		UpdatedAngleOffset = -CharacterAngleOffsetFromWallPlane + WallTilt;
	}

	DisplacementDelta = (NewCurveValue - OldCurveValue) * WallRunMaxVerticalDisplacement * WallUpVector;
	UpdatedCharacterRotation = CurrentWallRunDirection.ToOrientationRotator();
	UpdatedCharacterRotation.Roll += UpdatedAngleOffset;
}

bool UXyzBaseCharMovementComponent::AttachCharacterToRunnableWall(const FHitResult& Hit)
{
	EWallRunSide Side = EWallRunSide::None;
	FVector Direction = FVector::ZeroVector;
	GetWallRunSideAndDirection(Hit.ImpactNormal, Side, Direction);
	if (Side == CurrentWallRunSide && !bCanUseSameWallRunSide)
	{
		return false;
	}

	CurrentWallRunSide = Side;
	CurrentWallRunDirection = Direction;
	FVector UpdatedOffsetFromWallPlane = CharacterOffsetFromWallPlane;
	UpdatedOffsetFromWallPlane.Y = CurrentWallRunSide == EWallRunSide::Left ? UpdatedOffsetFromWallPlane.Y : -UpdatedOffsetFromWallPlane.Y;
	WallRunStartLocation = Hit.ImpactPoint + CurrentWallRunDirection.ToOrientationRotator().RotateVector(UpdatedOffsetFromWallPlane);
	BaseCharacterOwner->SetActorLocation(WallRunStartLocation);
	SetPlaneConstraintNormal(FVector::UpVector);

	return true;
}

void UXyzBaseCharMovementComponent::EndWallRun()
{
	if (IsWallRunning())
	{
		SetPlaneConstraintNormal(FVector::ZeroVector);
		DetachCharacterFromRunnableWall(EDetachFromRunnableWallMethod::Fall);
	}
}

bool UXyzBaseCharMovementComponent::UpdateWallRunVelocity(FHitResult& HitResult)
{
	FVector StartPosition = BaseCharacterOwner->GetActorLocation();
	FVector RightVector = BaseCharacterOwner->GetActorRightVector();
	RightVector.Z = 0.f;
	FVector LineTraceDirection = CurrentWallRunSide == EWallRunSide::Right ? RightVector.GetSafeNormal() : -RightVector.GetSafeNormal();
	LineTraceDirection += BaseCharacterOwner->GetActorForwardVector() * .2f; // line trace a bit ahead of the character ensuring that it won't go around corners due to rotation interpolation
	FVector EndPosition = StartPosition + WallRunLineTraceLength * LineTraceDirection.GetSafeNormal();

	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartPosition, EndPosition, ECC_WallRunning, FCollisionQueryParams::DefaultQueryParam))
	{
		EWallRunSide Side = EWallRunSide::None;
		FVector Direction = FVector::ZeroVector;
		GetWallRunSideAndDirection(HitResult.ImpactNormal, Side, Direction);

		if (Side != CurrentWallRunSide)
		{
			EndWallRun();
			return false;
		}

		CurrentWallRunDirection = Direction;
		Velocity = GetMaxSpeed() * CurrentWallRunDirection;
	}
	else
	{
		EndWallRun();
		return false;
	}
	return true;
}

void UXyzBaseCharMovementComponent::PhysWallRun(float DeltaTime, int32 Iterations)
{
	FHitResult HitResult;
	if (!UpdateWallRunVelocity(HitResult))
	{
		return;
	}

	FVector DisplacementDelta;
	FRotator UpdatedCharacterRotation;
	GetUpdatedWallRunDeltaAndRotation(DeltaTime, HitResult, DisplacementDelta, UpdatedCharacterRotation);
	UpdatedCharacterRotation = FMath::RInterpTo(GetLastUpdateRotation(), UpdatedCharacterRotation, DeltaTime, WallRunRotationInterpSpeed);

	FHitResult Hit;
	SafeMoveUpdatedComponent(Velocity * DeltaTime + DisplacementDelta, UpdatedCharacterRotation, false, Hit);

	WallRunElapsedTime += DeltaTime;
	if (WallRunElapsedTime >= WallRunMaxDuration)
	{
		EndWallRun();
	}
}
#pragma endregion
