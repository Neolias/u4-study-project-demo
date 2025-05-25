// Copyright 2025 https://github.com/Neolias/ue4-study-project-demo/blob/main/LICENSE

#pragma once

#include "CoreMinimal.h"
#include "Characters/XyzBaseCharacter.h"
#include "Components/TimelineComponent.h"
#include "PlayerCharacter.generated.h"

UCLASS(Blueprintable)
class XYZHOMEWORK_API APlayerCharacter : public AXyzBaseCharacter
{
	GENERATED_BODY()

public:
	explicit APlayerCharacter(const FObjectInitializer& ObjectInitializer);
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

#pragma region CAMERA

protected:
	UFUNCTION(Server, Reliable)
	void Server_SetShouldSkipProneTimeline(bool bShouldSkipProneTimeline_In);
	void UpdateProneCameraTimeline(float Value);
	void UpdateAimingFOVTimeline(float Value);
	void UpdateWallRunCameraTimeline(float Value);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base Character|Player|Camera")
	class UCameraComponent* CameraComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base Character|Player|Camera")
	class USpringArmComponent* SpringArmComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Player|Camera")
	float ProneCameraHeightOffset = -50.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Player|Camera")
	float ProneCameraProximityOffset = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Player|Camera")
	float ProneCameraRightOffset = 50.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Player|Camera")
	UCurveFloat* ProneCameraTimelineCurve;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Player|Camera")
	UCurveFloat* AimingFOVTimelineCurve;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Player|Camera")
	UCurveFloat* WallRunCameraTimelineCurve;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Player|Camera", meta = (ClampMin = 0.f, ClampMax = 1.f, UIMin = 0.f, UIMax = 1.f))
	float AimTurnModifier = 0.75f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Character|Player|Camera", meta = (ClampMin = 0.f, ClampMax = 1.f, UIMin = 0.f, UIMax = 1.f))
	float AimLookUpModifier = 0.75f;

	TWeakObjectPtr<APlayerCameraManager> CachedCameraManager;
	FVector CachedSpringArmSocketOffset;
	FVector CachedSpringArmTargetOffset;
	FVector NewSpringArmSocketOffsetDelta;
	FVector NewSpringArmTargetOffsetDelta;
	FTimeline AimingFOVTimeline;
	FTimeline WallRunCameraTimeline;
	FTimeline ProneCameraTimeline;
	UPROPERTY(Replicated)
	bool bShouldSkipProneTimeline = false;
	float DefaultCameraFOV = 0.f;
	float TargetAimingFOV = 0.f;

#pragma endregion

#pragma region MOVEMENT / SWIMMING

public:
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	virtual void MoveForward(float Value) override;
	virtual void MoveRight(float Value) override;
	virtual void Turn(float Value) override;
	virtual void LookUp(float Value) override;
	virtual void TurnAtRate(float Value) override;
	virtual void LookUpAtRate(float Value) override;
	virtual void ClimbLadderUp(float Value) override;
	virtual void SwimForward(float Value) override;
	virtual void SwimRight(float Value) override;
	virtual void SwimUp(float Value) override;

#pragma endregion

#pragma region SLIDING / WALL RUNNING

public:
	virtual void OnStartSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnStopSlide(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnWallRunStart() override;
	virtual void OnWallRunEnd() override;
#pragma endregion

#pragma region CROUCHING / PRONE

public:
	virtual bool CanChangeCrouchState() const override;
	virtual void UnCrouch(bool bClientSimulation = false) override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual bool CanChangeProneState() const override;
	virtual void OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnStopProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
#pragma endregion

#pragma region AIMING

protected:
	virtual void OnStartAiming_Implementation() override;
	virtual void OnStopAiming_Implementation() override;
	float GetAimTurnModifier() const;
	float GetAimLookUpModifier() const;
#pragma endregion
};
