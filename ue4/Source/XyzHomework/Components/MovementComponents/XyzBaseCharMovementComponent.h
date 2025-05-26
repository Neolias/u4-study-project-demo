// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "XyzGenericEnums.h"
#include "XyzGenericStructs.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "XyzBaseCharMovementComponent.generated.h"

class ALadder;
class AZipline;

#pragma region REPLICATION

class FSavedMove_XyzCharacter : public FSavedMove_Character
{
	typedef FSavedMove_Character Super;

public:
	virtual void Clear() override;
	virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
	virtual bool CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* InCharacter, float MaxDelta) const override;
	virtual void PrepMoveFor(ACharacter* Character) override;
	
	uint16 SavedCustomCompressedFlags = 0;
	float SavedWallRunElapsedTime = 0.f;

	/** uint16 bit masks used by UXyzBaseCharMovementComponent::MoveAutonomous() to encode movement information. */
	enum ECustomCompressedFlags
	{
		FLAG_IsCrouching = 0x01,
		FLAG_IsProne = 0x02,
		FLAG_IsSprinting = 0x04,
		FLAG_IsSliding = 0x08,
		FLAG_IsOutOfStamina = 0x10,
		FLAG_IsDiving = 0x20,
		FLAG_IsAiming = 0x40,
		FLAG_IsReloading = 0x80,
		FLAG_IsThrowing = 0x100,
	};
};

class FNetworkPredictionData_Client_XyzCharacter : public FNetworkPredictionData_Client_Character
{
	typedef FNetworkPredictionData_Client_Character Super;

public:
	FNetworkPredictionData_Client_XyzCharacter(const UCharacterMovementComponent& ClientMovement);
	virtual FSavedMovePtr AllocateNewMove() override;
};

struct FXyzNetworkMoveData : public FCharacterNetworkMoveData
{
	typedef FCharacterNetworkMoveData Super;

public:
	virtual bool Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar, UPackageMap* PackageMap, ENetworkMoveType MoveType) override;
	virtual void ClientFillNetworkMoveData(const FSavedMove_Character& ClientMove, ENetworkMoveType MoveType) override;

	/** Additional compressed flags mainly replicated to control the max movement speed of the character. */
	uint16 CustomCompressedFlags = 0;
	/** Timer used to update vertical displacement of the character during wall running. */
	float WallRunElapsedTime = 0.f;
};

struct FXyzNetworkMoveDataContainer : public FCharacterNetworkMoveDataContainer
{
	typedef FCharacterNetworkMoveDataContainer Super;

public:
	FXyzNetworkMoveDataContainer()
	{
		NewMoveData = &CustomMoves[0];
		PendingMoveData = &CustomMoves[1];
		OldMoveData = &CustomMoves[2];
	}

	FXyzNetworkMoveData CustomMoves[3];
};
#pragma endregion

/** Extends the default character movement component with new movement actions and modes. Includes full replication of the new features via SavedMoves. */
UCLASS()
class XYZHOMEWORK_API UXyzBaseCharMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	friend class FSavedMove_XyzCharacter;

public:
	UXyzBaseCharMovementComponent();
	virtual void BeginPlay() override;

protected:
	virtual float GetMaxSpeed() const override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
	virtual void PhysicsRotation(float DeltaTime) override;

private:
	UPROPERTY()
	class AXyzBaseCharacter* BaseCharacterOwner;
	UPROPERTY()
	class UCharacterEquipmentComponent* CharacterEquipmentComponent;
	/** Flag enabling custom rotation logic during PhysicsRotation(). */
	bool bForceRotation = false;
	/** Custom rotation applied to the character when bForceRotation flag is set to true. */
	FRotator ForceTargetRotation = FRotator::ZeroRotator;

#pragma region REPLICATION

public:
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	/** Returns a movement flag stored in MovementFlags. */
	bool GetMovementFlag(uint32 FlagIndex) const;
	/** Sets a new value for a movement flag stored in MovementFlags. */
	void SetMovementFlag(uint32 FlagIndex, bool bIsActivated);

protected:
	virtual void MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector& NewAccel) override;

private:
	FXyzNetworkMoveDataContainer XyzNetworkMoveDataContainer;
	/** Movement flags describing activation statuses of gameplay abilities. Used to update the max movement speed of the character in GetMaxSpeed(). */
	TArray<bool> MovementFlags;
#pragma endregion

#pragma region SWIMMING

public:
	virtual FRotator ComputeOrientToMovementRotation(const FRotator& CurrentRotation, float DeltaTime, FRotator& DeltaRotation) const override;
	/** Sets the swimming capsule size if swimming. Resets to the default capsule size otherwise. */
	void UpdateSwimmingCapsuleSize() const;
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UpdateSwimmingCapsuleSize();
	void SetSwimmingOnWaterPlane(bool bInIsSwimmingOnWaterPlane);
	/**
	 * Is the character located below the water surface?
	 * @param LocationOffset Offset that allows for prediction, e.g. if passing the Velocity value.
	 */
	bool IsSwimmingUnderWater(FVector LocationOffset = FVector::ZeroVector) const;
	/**
	 * Updates the Diving status of the character.
	 * @param bIsDiving New status.
	 */
	void OnDiving(bool bIsDiving);

	bool bIsSwimmingOnWaterPlane = false;

protected:
	virtual void PhysSwimming(float DeltaTime, int32 Iterations) override;

	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Swimming", meta = (ClampMin = 0.f, UIMin = 0.f))
	float SwimmingCapsuleHalfHeight = 34.f;
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Swimming", meta = (ClampMin = 0.f, UIMin = 0.f))
	float SwimmingCapsuleRadius = 34.f;
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Swimming", meta = (ClampMin = 0.f, UIMin = 0.f))
	float DiveSpeed = 100.f;
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Swimming", meta = (ClampMin = 0.f, UIMin = 0.f))
	float EmergeSpeed = 100.f;
	/** How far from the water plane the character should be located to detect the plane. */
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Swimming", meta = (ClampMin = 0.f, UIMin = 0.f))
	float WaterPlaneDetectionRangeZ = 25.f;
	/** How far from the water plane the character should be located to snap to the plane vertically. */
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Swimming")
	float WaterSnappingOffsetZ = -5.f;

private:
	float CachedBuoyancy = 0.f;
#pragma endregion

#pragma region SPRINTING / SLIDING

public:
	void StartSprint();
	void StopSprint();
	void StartSlide();
	void StopSlide();

protected:
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Sprinting", meta = (ClampMin = 0.f, UIMin = 0.f))
	float SprintSpeed = 1200.f;
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Sprinting", meta = (ClampMin = 0.f, UIMin = 0.f))
	float OutOfStaminaSpeed = 100.f;
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Sliding", meta = (ClampMin = 0.f, UIMin = 0.f))
	float SlideSpeed = 1000.f;
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Sliding", meta = (ClampMin = 0.f, UIMin = 0.f))
	float SlideCapsuleHalfHeight = 60.f;
#pragma endregion

#pragma region CROUCHING / PRONE

public:
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;
	virtual bool IsCrouching() const override;
	/** Conducts an overlap test and returns its result. */
	bool CanUnCrouch() const;
	virtual void Crouch(bool bClientSimulation = false) override;
	virtual void UnCrouch(bool bClientSimulation = false) override;
	float GetProneCapsuleHalfHeight() const { return ProneCapsuleHalfHeight; }
	void Prone();
	/** Conducts an overlap test and returns its result. */
	bool CanUnProne() const;
	bool UnProne();

protected:
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Prone", meta = (ClampMin = 0.f, UIMin = 0.f))
	float ProneSpeed = 50.f;
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Prone", meta = (ClampMin = 0.f, UIMin = 0.f))
	float ProneCapsuleHalfHeight = 34.f;
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Prone", meta = (ClampMin = 0.f, UIMin = 0.f))
	float ProneCapsuleRadius = 34.f;
#pragma endregion

#pragma region MANTLING

public:
	bool IsMantling() const;
	void StartMantle(const FMantlingMovementParameters& MantlingParameters);

private:
	void StopMantle();
	void PhysMantling(float DeltaTime, int32 Iterations);

	FMantlingMovementParameters CurrentMantlingParameters;
	FTimerHandle MantlingTimer;
	FVector OffsetFromMantlingTarget = FVector::ZeroVector;
#pragma endregion

#pragma region LADDER

public:
	ALadder* GetCurrentLadder() const { return CurrentLadder.Get(); }
	bool IsOnLadder() const;
	bool IsOnTopOfCurrentLadder() const;
	/** How fast the character should move verticatlly. */
	float GetLadderSpeedRatio() const;
	void AttachCharacterToLadder(ALadder* Ladder);
	void DetachCharacterFromLadder(EDetachFromLadderMethod DetachFromLadderMethod);

protected:
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Ladder", meta = (ClampMin = 0.f, UIMin = 0.f))
	float ClimbingOnLadderMaxSpeed = 200.f;
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Ladder", meta = (ClampMin = 0.f, UIMin = 0.f))
	float ClimbingOnLadderBrakingDeceleration = 2048.f;
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Ladder")
	float CharacterOffsetFromLadder = 54.f;
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Ladder")
	float DetachmentDistanceFromTop = 45.f;
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Ladder")
	float DetachmentDistanceFromBottom = 90.f;
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Ladder", meta = (ClampMin = 0.f, UIMin = 0.f))
	float JumpOffLadderSpeed = 500.f;

private:
	float GetCharacterProjectionToCurrentLadder(FVector Location) const;
	void PhysLadder(float DeltaTime, int32 Iterations);

	TWeakObjectPtr<ALadder> CurrentLadder;
#pragma endregion

#pragma region ZIPLINE

public:
	AZipline* GetCurrentZipline() const { return CurrentZipline.Get(); }
	bool IsOnZipline() const;
	void AttachCharacterToZipline(AZipline* Zipline);
	void DetachCharacterFromZipline(EDetachFromZiplineMethod DetachFromZiplineMethod);

protected:
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Zipline", meta = (ClampMin = 0.f, UIMin = 0.f))
	float ZiplineMovementMaxSpeed = 500.f;
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Zipline")
	FVector ZiplineAttachmentOffset = FVector(50.f, 10.f, -123.f);
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Zipline")
	float ZiplineDetachmentDistanceFromEnd = 100.f;
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Zipline", meta = (ClampMin = 0.f, UIMin = 0.f))
	float JumpOffZiplineSpeed = 500.f;
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Zipline", meta = (UIMin = -180.f, UIMax = 180.f))
	float JumpOffZiplineAngle = 30.f;

private:
	float GetCharacterProjectionToCurrentZipline(FVector Location) const;
	void PhysZipline(float DeltaTime, int32 Iterations);
	
	TWeakObjectPtr<AZipline> CurrentZipline;
#pragma endregion

#pragma region WALL RUNNING

public:
	EWallRunSide GetCurrentWallRunSide() const { return CurrentWallRunSide; }
	FVector GetCurrentWallRunDirection() const { return CurrentWallRunDirection; }
	bool IsWallRunning() const;
	void StartWallRun(const FHitResult& Hit);
	void StopWallRun();
	void DetachCharacterFromRunnableWall(EDetachFromRunnableWallMethod DetachFromRunnableWallMethod);

protected:
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Wall Running", meta = (ClampMin = 0.f, UIMin = 0.f))
	float WallRunMaxSpeed = 600.f;
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Wall Running", meta = (ClampMin = 0.f, UIMin = 0.f))
	float WallRunMaxDuration = 2.f;
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Wall Running", meta = (ClampMin = 0.f, UIMin = 0.f))
	float WallRunRotationInterpSpeed = 20.f;
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Wall Running")
	FVector CharacterOffsetFromWallPlane = FVector(0.f, 45.f, 0.f);
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Wall Running")
	float CharacterAngleOffsetFromWallPlane = 7.5f;
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Wall Running", meta = (ClampMin = 0.f, UIMin = 0.f))
	float WallRunLineTraceLength = 200.f;
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Wall Running", meta = (ClampMin = 0.f, UIMin = 0.f))
	float WallRunMaxVerticalDisplacement = 150.f;
	UPROPERTY(EditDefaultsOnly, Category = "Base Character|Movement Component|Wall Running")
	UCurveFloat* WallRunVerticalDisplacementCurve;
	UPROPERTY(EditAnywhere, Category = "Base Character|Movement Component|Wall Running", meta = (ClampMin = 0.f, UIMin = 0.f))
	float JumpOffWallRunSpeed = 700.f;

private:
	bool CanWallRunInCurrentState() const;
	/** Is the surface NOT walkable? */
	bool IsSurfaceWallRunnable(const FVector& SurfaceNormal) const;
	void GetWallRunSideAndDirection(FVector HitNormal, OUT EWallRunSide& OutSide, OUT FVector& OutDirection) const;
	void GetUpdatedWallRunDeltaAndRotation(float DeltaTime, const FHitResult& HitResult, OUT FVector& DisplacementDelta, OUT FRotator& UpdatedCharacterRotation) const;
	bool AttachCharacterToRunnableWall(const FHitResult& Hit);
	bool UpdateWallRunVelocity(FHitResult& HitResult);
	void PhysWallRun(float DeltaTime, int32 Iterations);
	
	EWallRunSide CurrentWallRunSide = EWallRunSide::None;
	FVector CurrentWallRunDirection = FVector::ZeroVector;
	FVector WallRunStartLocation = FVector::ZeroVector;
	float WallRunElapsedTime = 0.f;
	/** Can the character be attached to runnable walls with the same side while falling? */
	bool bCanUseSameWallRunSide = false;
#pragma endregion
};
